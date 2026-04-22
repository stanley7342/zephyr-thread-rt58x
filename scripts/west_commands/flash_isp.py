"""west flash-isp: UART-ISP flash driver for RT58x/RT584.

Reads sysbuild's domains.yaml and flashes each domain's image in a single
isp_client session: one handshake + one prep up front, then erase+write
per domain. Per-domain flash address comes from CONFIG_FLASH_LOAD_OFFSET.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

import yaml
from west import log
from west.commands import WestCommand


_CONFIG_RE = re.compile(r"^CONFIG_([A-Z0-9_]+)=(.+)$")


def _parse_dotconfig(path: Path) -> dict[str, str]:
    out: dict[str, str] = {}
    if not path.is_file():
        return out
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = _CONFIG_RE.match(line.strip())
        if m:
            out[m.group(1)] = m.group(2).strip('"')
    return out


def _pick_binary(domain_build: Path) -> Path:
    """Prefer signed.bin (MCUboot-wrapped app) over raw zephyr.bin."""
    signed = domain_build / "zephyr" / "zephyr.signed.bin"
    raw = domain_build / "zephyr" / "zephyr.bin"
    if signed.is_file():
        return signed
    if raw.is_file():
        return raw
    raise FileNotFoundError(f"no zephyr.bin / zephyr.signed.bin under {domain_build}")


class FlashIsp(WestCommand):

    def __init__(self):
        super().__init__(
            "flash-isp",
            "flash RT58x/RT584 over UART using isp_cli",
            "Flash each sysbuild domain in flash_order in a single isp_cli session.",
        )

    def do_add_parser(self, parser_adder):
        p = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description,
            formatter_class=argparse.RawDescriptionHelpFormatter,
        )
        p.add_argument("-d", "--build-dir", default="build",
                       help="sysbuild top-level build dir (default: build)")
        p.add_argument("--port", required=True, help="serial port, e.g. COM18")
        p.add_argument("--chip", choices=["rt58x", "rt584"], default=None,
                       help="target chip; auto-detected from flash id if omitted")
        p.add_argument("--baudrate", type=int, default=2_000_000)
        p.add_argument("--timeout", type=float, default=2.0,
                       help="per-command timeout passed to isp_cli")
        p.add_argument("--wait", type=float, default=10.0,
                       help="seconds to wait for handshake")
        p.add_argument("--auto-reset", action="store_true",
                       help="pulse DTR to reset EVK before handshake")
        p.add_argument("--skip", action="append", default=[],
                       metavar="DOMAIN",
                       help="skip a domain by name (repeatable)")
        p.add_argument("--only", action="append", default=[],
                       metavar="DOMAIN",
                       help="only flash the listed domain(s) (repeatable)")
        p.add_argument("--no-erase", action="store_true",
                       help="skip flash erase (caller pre-erased)")
        p.add_argument("--erase-4k", action="store_true",
                       help="force 4K sector erase only; slower but survives "
                            "ROM ISP 64K half-erase bugs")
        p.add_argument("--dry-run", action="store_true",
                       help="print planned writes without connecting")
        p.add_argument("--monitor", action="store_true",
                       help="open serial monitor after the last write")
        return p

    def do_run(self, args, unknown_args):
        build_dir = Path(args.build_dir).resolve()
        domains_yaml = build_dir / "domains.yaml"
        if not domains_yaml.is_file():
            log.die(f"{domains_yaml} not found — pass -d pointing at a sysbuild build dir")

        with domains_yaml.open("r", encoding="utf-8") as f:
            dom = yaml.safe_load(f)

        order: list[str] = list(dom.get("flash_order") or
                                [d["name"] for d in dom.get("domains", [])])
        by_name = {d["name"]: Path(d["build_dir"]) for d in dom.get("domains", [])}

        only = set(args.only)
        skip = set(args.skip)

        repo_root = Path(__file__).resolve().parents[2]
        isp_dir = repo_root / "tools" / "isp_cli"
        if not (isp_dir / "isp_client.py").is_file():
            log.die(f"isp_client.py not found at {isp_dir}")
        if str(isp_dir) not in sys.path:
            sys.path.insert(0, str(isp_dir))

        plan: list[tuple[str, int, Path]] = []  # (domain, addr, bin_path)
        for name in order:
            if only and name not in only:
                continue
            if name in skip:
                continue
            dbuild = by_name.get(name)
            if dbuild is None:
                log.wrn(f"domain '{name}' in flash_order but missing from domains — skipping")
                continue
            cfg = _parse_dotconfig(dbuild / "zephyr" / ".config")
            addr = int(cfg.get("FLASH_LOAD_OFFSET", "0x0"), 0)
            bin_path = _pick_binary(dbuild)
            plan.append((name, addr, bin_path))

        if not plan:
            log.die("nothing to flash (check --only / --skip)")

        if args.dry_run:
            for name, addr, bin_path in plan:
                log.inf(f"[dry-run] {name:<24} @ {addr:#010x} <- {bin_path}")
            return

        # In-process isp_client: one handshake, one prep, multiple writes.
        import isp_client  # noqa: E402

        verbose = self.verbosity >= log.VERBOSE_VERY

        client = isp_client.IspClient(
            args.port, baudrate=args.baudrate,
            timeout=args.timeout, verbose=verbose,
        )
        try:
            if not client.connect(wait_s=args.wait, hint=True,
                                  auto_reset=args.auto_reset):
                log.die("handshake failed (press RESET on EVK or use --auto-reset)")
            client.enter_slip()
            isp_client._run_prep(client, verbose=verbose)

            # Chip auto-detection from flash id (re-read; prep already fetched
            # it but didn't hand back the value through a stable API).
            fid = client.cmd_read_flash_id()
            detected = isp_client.chip_from_flash_id(fid)
            if args.chip is None:
                if detected is None:
                    log.die(f"cannot auto-detect chip from flash id {fid:#010x}; "
                            "pass --chip rt58x|rt584")
                log.inf(f"chip auto-detected: {detected} (flash id {fid:#010x})")
                args.chip = detected
            elif detected is not None and detected != args.chip:
                log.wrn(f"--chip {args.chip} but flash id {fid:#010x} looks like {detected}")

            for name, addr, bin_path in plan:
                payload = bin_path.read_bytes()
                if not payload:
                    log.die(f"{bin_path} is empty")
                log.inf(f"\n--- {name} @ {addr:#010x}  ({bin_path.name}, "
                        f"{len(payload)} bytes) ---")
                isp_client.write_image(
                    client, addr, payload,
                    no_erase=args.no_erase, verbose=verbose,
                    erase_4k=args.erase_4k,
                )
        finally:
            client.close()

        if args.monitor:
            import subprocess
            subprocess.call([
                sys.executable, str(isp_dir / "isp_client.py"),
                "--port", args.port, "--baudrate", str(args.baudrate),
                "monitor",
            ])
