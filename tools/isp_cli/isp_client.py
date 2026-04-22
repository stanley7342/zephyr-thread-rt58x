#!/usr/bin/env python3
"""ISP client CLI for Rafael bootloader-app.

Protocol reference:
  examples/bootloader-app/bootloader-app/{bootloader.c,isp.c,slip.c,include/*.h}
"""
from __future__ import annotations

import argparse
import struct
import sys
import time
from pathlib import Path
from typing import Optional

try:
    import serial
except ImportError:
    sys.stderr.write("missing dependency: pip install pyserial\n")
    sys.exit(2)


# ---------------------------------------------------------------- constants --
SLIP_DELIMITER = 0xC0
SLIP_ESC = 0xDB
SLIP_ESC_DELIM = 0xDC
SLIP_ESC_ESC = 0xDD

START_BYTE = 0x52
DEFAULT_BAUDRATE = 2_000_000
DEFAULT_TIMEOUT = 2.0

CMD_WRITE_REG = 0x02
CMD_READ_ID = 0x04
CMD_READ_STATUS = 0x05
CMD_WRITE_STATUS = 0x06
CMD_WRITE_BYTE = 0x07
CMD_ERASE_FLASH = 0x08
CMD_WRITE_PAGE = 0x09
CMD_JUMP = 0x0A
CMD_WRITE_SRAM = 0x0B
CMD_CHECK_CHIP_ID = 0x0C
CMD_VERIFY_FLASH = 0x0E

ERASE_SECTOR_4K = 0x01
ERASE_BLOCK_32K = 0x02
ERASE_BLOCK_64K = 0x03

PAGE_SIZE = 256

# Per-chip flash layout for `boot` / `app` convenience commands.
CHIP_LAYOUT = {
    "rt58x": {"boot": 0x00000000, "app": 0x00010000},
    "rt584": {"boot": 0x10000000, "app": 0x10010000},
}

# Flash device id → chip family. Populated from observed hardware; add more as
# needed. 0x001565C8 is the GigaDevice GD25LE128 seen on RT583 EVB.
FLASH_ID_TO_CHIP = {
    0x001565C8: "rt58x",
}


def chip_from_flash_id(flash_id: int) -> "str | None":
    return FLASH_ID_TO_CHIP.get(flash_id)


# -------------------------------------------------------------------- CRC16 --
def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    crc = init
    for b in data:
        crc = ((crc << 8) & 0xFFFF) ^ _CRC_TABLE[((crc >> 8) ^ b) & 0xFF]
    return crc & 0xFFFF


def crc8_d5(data: bytes) -> int:
    """CRC8 poly 0xD5, init 0, no xor-out (matches isp_download_tool)."""
    crc = 0
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = ((crc << 1) ^ 0xD5) & 0xFF if (crc & 0x80) else (crc << 1) & 0xFF
    return crc


_CRC_TABLE: list[int] = []


def _build_crc_table() -> None:
    for i in range(256):
        c = i << 8
        for _ in range(8):
            c = ((c << 1) ^ 0x1021) if (c & 0x8000) else (c << 1)
            c &= 0xFFFF
        _CRC_TABLE.append(c)


_build_crc_table()


# --------------------------------------------------------------------- SLIP --
def slip_encode(payload: bytes) -> bytes:
    out = bytearray([SLIP_DELIMITER])
    for b in payload:
        if b == SLIP_DELIMITER:
            out += bytes([SLIP_ESC, SLIP_ESC_DELIM])
        elif b == SLIP_ESC:
            out += bytes([SLIP_ESC, SLIP_ESC_ESC])
        else:
            out.append(b)
    out.append(SLIP_DELIMITER)
    return bytes(out)


def slip_decode_stream(ser: serial.Serial, timeout: float) -> bytes:
    """Read one SLIP frame (content between two 0xC0s)."""
    deadline = time.monotonic() + timeout
    # wait for opening delimiter
    while time.monotonic() < deadline:
        b = ser.read(1)
        if not b:
            continue
        if b[0] == SLIP_DELIMITER:
            break
    else:
        raise TimeoutError("no SLIP start delimiter")

    buf = bytearray()
    esc = False
    while time.monotonic() < deadline:
        b = ser.read(1)
        if not b:
            continue
        v = b[0]
        if v == SLIP_DELIMITER:
            if not buf:
                # back-to-back delimiter -- keep waiting for real payload
                continue
            return bytes(buf)
        if esc:
            if v == SLIP_ESC_DELIM:
                buf.append(SLIP_DELIMITER)
            elif v == SLIP_ESC_ESC:
                buf.append(SLIP_ESC)
            else:
                raise ValueError(f"bad SLIP escape byte: {v:#x}")
            esc = False
        elif v == SLIP_ESC:
            esc = True
        else:
            buf.append(v)
    raise TimeoutError("no SLIP end delimiter")


# -------------------------------------------------------- ISP packet helpers --
def build_isp_packet(cmd: int, option: int, data: bytes = b"", seq: int = 0) -> bytes:
    data_len = len(data)
    length = data_len + 2  # +2 covers cmd + option
    hdr = bytearray(5)
    hdr[0] = START_BYTE
    hdr[1] = seq & 0xFF
    hdr[2] = (length >> 8) & 0xFF
    hdr[3] = length & 0xFF
    hdr[4] = (~(hdr[1] + hdr[2] + hdr[3])) & 0xFF
    body = bytes([cmd & 0xFF, option & 0xFF]) + bytes(data)
    pkt = bytes(hdr) + body
    crc = crc16_ccitt(pkt)
    return pkt + bytes([(crc >> 8) & 0xFF, crc & 0xFF])


def parse_isp_response(frame: bytes) -> tuple[int, int, bytes]:
    """Return (command, option, payload)."""
    if len(frame) < 9:
        raise ValueError(f"short frame ({len(frame)} bytes): {frame.hex()}")
    if frame[0] != START_BYTE:
        raise ValueError(f"bad start byte {frame[0]:#x}")
    length = (frame[2] << 8) | frame[3]
    want_checksum = (~(frame[1] + frame[2] + frame[3])) & 0xFF
    if frame[4] != want_checksum:
        raise ValueError(f"header checksum bad: got {frame[4]:#x} want {want_checksum:#x}")
    expected_total = 5 + length + 2
    if len(frame) != expected_total:
        raise ValueError(
            f"length mismatch: frame={len(frame)} header says total={expected_total}"
        )
    crc_body = frame[: 5 + length]
    want_crc = crc16_ccitt(bytes(crc_body))
    got_crc = (frame[5 + length] << 8) | frame[6 + length]
    if want_crc != got_crc:
        raise ValueError(f"crc16 bad: got {got_crc:#06x} want {want_crc:#06x}")
    cmd = frame[5]
    option = frame[6]
    payload = frame[7 : 5 + length]
    return cmd, option, payload


# ---------------------------------------------------------------- transport --
class IspClient:
    def __init__(self, port: str, baudrate: int = DEFAULT_BAUDRATE,
                 timeout: float = DEFAULT_TIMEOUT, verbose: bool = False):
        # timeout=0 => non-blocking reads so the handshake loop can poll fast
        self.ser = serial.Serial(port, baudrate=baudrate,
                                 timeout=0.0, write_timeout=timeout)
        self.timeout = timeout
        self.verbose = verbose
        # isp_download_tool initializes sequence = 1; seq=0 is often ignored.
        self.seq = 1
        self.in_slip = False

    def close(self) -> None:
        try:
            self.ser.close()
        except Exception:
            pass

    # ---- low-level handshake ---------------------------------------------
    def pulse_reset(self, hold_ms: int = 1) -> None:
        """Pulse DTR low to reset EVK (if USB-UART DTR is wired to MCU reset)."""
        self.ser.dtr = True   # drive low
        time.sleep(hold_ms / 1000.0)
        self.ser.dtr = False
        if self.verbose:
            print(f"{_tag('isp')} DTR reset pulsed")

    def connect(self, wait_s: float = 10.0, hint: bool = True,
                auto_reset: bool = False) -> bool:
        """Two-stage ISP entry:
             1) send 0xAA every ~5 ms until 0x55 echoed back
             2) send 0x69 repeatedly until 0x6A echoed back
        Short interval catches ROM ISP's brief post-reset window without
        overflowing the MCU UART FIFO.
        """
        self.ser.reset_input_buffer()
        if auto_reset:
            self.pulse_reset()
        deadline = time.monotonic() + wait_s
        hinted = False
        got_55 = False

        # stage 1: 0xAA -> 0x55
        while time.monotonic() < deadline:
            if (hint and not hinted
                    and (deadline - time.monotonic()) < (wait_s - 0.3)):
                sys.stdout.write(
                    f"{_tag('isp')} waiting for handshake... "
                    "press RESET on EVK (or use --auto-reset)\n"
                )
                sys.stdout.flush()
                hinted = True
            self.ser.write(b"\xAA")
            self.ser.flush()
            step_end = time.monotonic() + 0.001  # 1 ms
            while time.monotonic() < step_end:
                b = self.ser.read(1)
                if b == b"\x55":
                    got_55 = True
                    break
            if got_55:
                break

        if not got_55:
            return False

        # stage 2: 0x69 -> 0x6A
        self.ser.reset_input_buffer()
        while time.monotonic() < deadline:
            self.ser.write(b"\x69")
            self.ser.flush()
            step_end = time.monotonic() + 0.02
            while time.monotonic() < step_end:
                b = self.ser.read(1)
                if b == b"\x6A":
                    return True
        return False

    def enter_slip(self) -> None:
        """Mark SLIP entry. ROM ISP accepts SLIP packets directly after handshake;
        bootloader-app's extra 0xC0 sync is not used here to match isp_download_tool.
        """
        if self.in_slip:
            return
        time.sleep(0.001)
        self.in_slip = True
        if self.verbose:
            print(f"{_tag('isp')} entered SLIP mode")

    # ---- command transaction ---------------------------------------------
    def _next_seq(self) -> int:
        s = self.seq & 0xFF
        if s == 0:
            s = 1
        nxt = (s + 1) & 0xFF
        self.seq = 1 if nxt == 0 else nxt
        return s

    def transact(self, cmd: int, option: int = 0, data: bytes = b"",
                 expect_response: bool = True,
                 reuse_seq: bool = False) -> tuple[int, int, bytes]:
        if not self.in_slip:
            self.enter_slip()
        if reuse_seq and hasattr(self, "_last_seq") and self._last_seq is not None:
            seq = self._last_seq
        else:
            seq = self._next_seq()
            self._last_seq = seq
        pkt = build_isp_packet(cmd, option, data, seq=seq)
        frame = slip_encode(pkt)
        self.ser.write(frame)
        self.ser.flush()
        if not expect_response:
            return (cmd, 0, b"")
        rframe = slip_decode_stream(self.ser, self.timeout)
        rcmd, ropt, rpayload = parse_isp_response(rframe)
        if rcmd != cmd:
            raise RuntimeError(
                f"response cmd mismatch: sent {cmd:#x}, got {rcmd:#x}"
            )
        return rcmd, ropt, rpayload

    # ---- high level ops --------------------------------------------------
    def cmd_check_chip_id(self) -> bytes:
        _, _, p = self.transact(CMD_CHECK_CHIP_ID)
        return p

    def cmd_read_flash_id(self) -> int:
        _, _, p = self.transact(CMD_READ_ID)
        if len(p) < 4:
            raise RuntimeError(f"flash id payload too short: {p.hex()}")
        return struct.unpack("<I", p[:4])[0]

    def cmd_read_status(self, mode: int = 1) -> int:
        _, _, p = self.transact(CMD_READ_STATUS, option=mode)
        if not p:
            raise RuntimeError("empty status payload")
        return p[0]

    def wait_wip_clear(self, timeout_s: float = 2.0) -> None:
        """Poll SPI flash status register 0 until WIP (bit 0) clears.

        The ROM ISP returns from erase/write commands before the flash itself
        finishes; firing the next op too early produces status 0x51. Polling
        the flash's own status register is the canonical fix.

        isp_portref.py's read_status_0 treats `(opt & 0x0F) != 0` as "ROM ISP
        still busy, response payload incomplete" and loops. We do the same
        here instead of relying on cmd_read_status's single-shot behaviour.
        """
        deadline = time.monotonic() + timeout_s
        reg = 0xFF
        while time.monotonic() < deadline:
            _, opt, p = self.transact(CMD_READ_STATUS, option=1)
            if (opt & 0x0F) != 0 or not p:
                # ROM ISP busy — no valid flash status yet, retry
                continue
            reg = p[0]
            if (reg & 0x01) == 0:
                return
        raise RuntimeError(f"flash still WIP after {timeout_s}s (reg={reg:#04x})")

    def cmd_erase(self, addr: int, mode: int = ERASE_SECTOR_4K,
                  retries: int = 5) -> None:
        data = struct.pack("<I", addr & 0xFFFFFFFF)
        last_opt = None
        for attempt in range(retries + 1):
            _, opt, _ = self.transact(CMD_ERASE_FLASH, option=mode, data=data,
                                      reuse_seq=(attempt > 0))
            last_opt = opt
            if (opt & 0x01) == 0:
                self.wait_wip_clear()
                return
            # 0x51 = flash busy/stale; needs real settle time. Other errors
            # can retry fast.
            time.sleep(0.01 if opt == 0x51 else 0.001)
        raise RuntimeError(
            f"erase failed at {addr:#x} (last status {last_opt:#x}) "
            f"after {retries} retries"
        )

    def _build_write_frame(self, addr: int, page: bytes, mode: int,
                           seq: int) -> bytes:
        addr_bytes = struct.pack("<I", addr & 0xFFFFFFFF)
        addr_chk = (~sum(addr_bytes)) & 0xFF
        body = bytes([addr_chk]) + addr_bytes + page + bytes([crc8_d5(page)])
        pkt = build_isp_packet(CMD_WRITE_PAGE, mode & 0xFF, body, seq=seq)
        return slip_encode(pkt)

    def write_pages_pipelined(self, pages: "list[tuple[int, bytes]]",
                              depth: int = 8, on_progress=None,
                              mode: int = 0x00) -> None:
        """Pipeline N in-flight CMD_WRITE_PAGE requests.

        ROM ISP accepts up to ~5 queued commands and responds to each as the
        underlying SPI flash completes, which hides the host↔ISP round-trip
        behind flash program time. Any page returning a non-zero status is
        retried serially via cmd_write_page (which owns the 0x51 recovery).

        pages: list of (addr, 256-byte page). Bytes shorter than PAGE_SIZE are
        rejected by _build_write_frame's caller responsibility.
        on_progress: optional callable invoked with count of pages completed
        per batch.
        """
        i = 0
        while i < len(pages):
            window = pages[i : i + depth]
            seqs = [self._next_seq() for _ in window]
            # Send all frames back-to-back.
            for (addr, page), seq in zip(window, seqs):
                frame = self._build_write_frame(addr, page, mode, seq)
                self.ser.write(frame)
            self.ser.flush()
            # Read back one response per command.
            failures: list[int] = []  # indices within window
            for j, ((addr, _), seq) in enumerate(zip(window, seqs)):
                rframe = slip_decode_stream(self.ser, self.timeout)
                rcmd, ropt, _ = parse_isp_response(rframe)
                if rcmd != CMD_WRITE_PAGE:
                    raise RuntimeError(
                        f"pipeline: resp cmd {rcmd:#x} != write (0x09) at idx {j}"
                    )
                if (ropt & 0x01) != 0:
                    failures.append(j)
            # Fall back to serial recovery for the failures.
            for j in failures:
                addr, page = window[j]
                self.cmd_write_page(addr, page, mode=mode)
            if on_progress:
                on_progress(len(window))
            i += len(window)

    def cmd_write_page(self, addr: int, page: bytes, mode: int = 0x00,
                       retries: int = 5) -> None:
        if len(page) != PAGE_SIZE:
            raise ValueError(f"page must be {PAGE_SIZE} bytes, got {len(page)}")
        # Payload (matches isp_download_tool WriteFlashCommand):
        #   [0]        ~(sum of addr bytes) & 0xFF    (addr checksum)
        #   [1..4]     addr LE
        #   [5..260]   256-byte data
        #   [261]      CRC8 (poly 0xD5) of data
        addr_bytes = struct.pack("<I", addr & 0xFFFFFFFF)
        addr_chk = (~sum(addr_bytes)) & 0xFF
        body = bytes([addr_chk]) + addr_bytes + page + bytes([crc8_d5(page)])
        last_opt = None
        recovered = False
        for attempt in range(retries + 1):
            _, opt, _ = self.transact(CMD_WRITE_PAGE, option=mode & 0xFF,
                                      data=body)
            last_opt = opt
            if (opt & 0x01) == 0:
                return
            # Status 0x51 = target page still has stale bits. Rafael ROM ISP's
            # 64K block erase occasionally half-succeeds; recover by re-erasing
            # the containing 4K sector, then retry. One recovery per page.
            if opt == 0x51 and not recovered:
                sector = addr & ~0xFFF
                try:
                    self.cmd_erase(sector, ERASE_SECTOR_4K, retries=2)
                except RuntimeError:
                    pass
                recovered = True
                continue
            time.sleep(0.01 if opt == 0x51 else 0.001)
        raise RuntimeError(
            f"write page failed at {addr:#x} (last status {last_opt:#x}) "
            f"after {retries} retries"
        )

    # --- prep steps borrowed from isp_download_tool ----------------------
    def cmd_enable_write(self) -> None:
        """Matches _enable_write: cmd=0x06, option=0x01, payload=[0x00]."""
        while True:
            _, opt, _ = self.transact(CMD_WRITE_STATUS, option=0x01, data=b"\x00")
            if (opt & 0x0F) != 0x01:
                break

    def cmd_enable_qpsi(self) -> None:
        """Matches _enable_qpsi: cmd=0x02 with extended write_reg payload."""
        payload = bytes([0x18, 0x00, 0x30, 0xA0, 0xEF, 0x01, 0x00, 0x00])
        while True:
            _, opt, _ = self.transact(CMD_WRITE_REG, option=0x00, data=payload)
            if (opt & 0x0F) != 0x01:
                break

    def cmd_jump(self, to_sram: bool = False) -> None:
        # firmware acks but then doesn't return -- best effort
        try:
            self.transact(CMD_JUMP, option=0x01 if to_sram else 0x00,
                          expect_response=True)
        except TimeoutError:
            pass


# --------------------------------------------------------- erase planning ----
def plan_erase(start: int, length: int, sector_only: bool = False):
    """Yield (addr, erase_mode) covering [start, start+length).

    Uses 64K block erase when both addr and remaining length allow; otherwise
    4K sector erase. 32K mode is intentionally avoided -- Rafael ROM ISP has
    been observed to silently half-erase with mode 0x02, leaving stale bits
    that prevent subsequent page programming (manifests as status 0x51).

    sector_only=True forces 4K sectors everywhere (slower but survives ROM
    ISP 64K half-erase bugs seen on certain flash blocks).
    """
    end = start + length
    addr = start
    while addr < end:
        if sector_only:
            yield addr, ERASE_SECTOR_4K
            addr += 0x1000
            continue
        remaining = end - addr
        if remaining >= 0x10000 and (addr & 0xFFFF) == 0:
            yield addr, ERASE_BLOCK_64K
            addr += 0x10000
        else:
            yield addr, ERASE_SECTOR_4K
            addr += 0x1000


# ----------------------------------------------------------------- CLI glue --
def _open_client(args) -> IspClient:
    client = IspClient(args.port, baudrate=args.baudrate,
                       timeout=args.timeout, verbose=args.verbose)
    if not client.connect(wait_s=args.wait, hint=True,
                          auto_reset=args.auto_reset):
        client.close()
        raise SystemExit(
            "[error] handshake failed "
            "(press RESET on EVK while the tool is running, or raise --wait)"
        )
    if args.verbose:
        print(f"{_tag('isp')} handshake OK (0x55 + 0x6A)")
    client.enter_slip()
    return client


def _enable_vt_mode_windows() -> None:
    """Turn on ANSI escape processing in the Windows console (no-op elsewhere)."""
    if sys.platform != "win32":
        return
    try:
        import ctypes
        from ctypes import wintypes

        kernel32 = ctypes.windll.kernel32
        STD_OUTPUT_HANDLE = -11
        ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
        hout = kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
        mode = wintypes.DWORD()
        if not kernel32.GetConsoleMode(hout, ctypes.byref(mode)):
            return
        kernel32.SetConsoleMode(hout, mode.value | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
    except Exception:
        pass


# Bytes we forward verbatim: printable ASCII, ANSI-safe control codes
# (ESC, TAB, LF, CR, BEL, BS).
_MON_PASSTHRU = set(range(0x20, 0x7F)) | {0x07, 0x08, 0x09, 0x0A, 0x0D, 0x1B}


def _run_monitor(port: str, baudrate: int) -> int:
    """Serial monitor with ANSI color passthrough."""
    _enable_vt_mode_windows()
    ser = serial.Serial(port, baudrate=baudrate, timeout=0.1)
    print(f"{_tag('monitor')} {port} @ {baudrate} (Ctrl-C to stop)")
    out = sys.stdout.buffer  # write raw bytes so ANSI reaches the terminal intact
    try:
        while True:
            data = ser.read(256)
            if not data:
                continue
            clean = bytes(b if b in _MON_PASSTHRU else 0x2E for b in data)
            out.write(clean)
            out.flush()
    except KeyboardInterrupt:
        return 0
    finally:
        ser.close()


def cmd_monitor(args) -> int:
    return _run_monitor(args.port, args.baudrate)


def cmd_ping(args) -> int:
    client = IspClient(args.port, baudrate=args.baudrate,
                       timeout=args.timeout, verbose=args.verbose)
    try:
        ok = client.connect(wait_s=args.wait, hint=True,
                            auto_reset=args.auto_reset)
    finally:
        client.close()
    print("connect:", "OK" if ok else "FAIL")
    return 0 if ok else 1


def cmd_chipid(args) -> int:
    client = _open_client(args)
    try:
        p = client.cmd_check_chip_id()
    finally:
        client.close()
    print("chip id payload:", p.hex())
    return 0


def cmd_flashid(args) -> int:
    client = _open_client(args)
    try:
        fid = client.cmd_read_flash_id()
    finally:
        client.close()
    print(f"flash id: {fid:#010x}")
    return 0


def cmd_status(args) -> int:
    client = _open_client(args)
    try:
        s = client.cmd_read_status(args.mode)
    finally:
        client.close()
    print(f"status reg{args.mode}: {s:#04x}")
    return 0


def cmd_erase(args) -> int:
    client = _open_client(args)
    plan = list(plan_erase(args.addr, args.length))
    pb = ProgressBar(len(plan), label="erase",
                     enabled=not args.verbose)
    try:
        for addr, mode in plan:
            if args.verbose:
                tag = {1: "4K", 2: "32K", 3: "64K"}[mode]
                print(f"[erase] {addr:#010x} ({tag})")
            client.cmd_erase(addr, mode)
            pb.update()
        pb.close("done")
        print(f"{_tag('OK')} {args.addr:#010x} ~ {args.addr + args.length:#010x} "
              f"({args.length} bytes erased)")
    finally:
        client.close()
    return 0


def _pad_page(chunk: bytes) -> bytes:
    if len(chunk) == PAGE_SIZE:
        return chunk
    return chunk + b"\xFF" * (PAGE_SIZE - len(chunk))


LABEL_WIDTH = 7  # "[erase]" / "[write]" / "[ OK  ]" — align on left gutter


def _stamp() -> str:
    return time.strftime("%H:%M:%S")


def _tag(label: str) -> str:
    return f"{_stamp()} [{label}]".ljust(len("HH:MM:SS ") + LABEL_WIDTH)


class ProgressBar:
    """Aligned ASCII progress bar:

        [erase] [##############################]   24/  24 100.0%  ...KB/s  ETA 00:00
        [write] [##############################] 2217/2217 100.0% 83.3KB/s  ETA 00:00
    """

    def __init__(self, total: int, label: str, unit_bytes: int = 0,
                 width: int = 30, stream=sys.stdout, enabled: bool = True):
        self.total = max(total, 1)
        self.label = label
        self.unit_bytes = unit_bytes
        self.width = width
        self.stream = stream
        self.enabled = enabled
        self.current = 0
        self.start_t = time.monotonic()
        self._count_w = len(str(self.total))
        self._last_pct10 = -1  # redraw throttle: tenths of a percent
        # Enable ANSI escape processing on Windows so the green `#` sequence
        # renders instead of printing literal `\x1b[32m`.
        _enable_vt_mode_windows()
        if self.enabled:
            self._render()

    def update(self, n: int = 1) -> None:
        if not self.enabled:
            return
        self.current = min(self.current + n, self.total)
        pct10 = int(self.current * 1000 / self.total)
        if pct10 == self._last_pct10 and self.current != self.total:
            return
        self._last_pct10 = pct10
        self._render()

    def _fmt_rate(self, rate: float) -> str:
        if rate < 1024:
            return f"{rate:6.1f} B/s"
        if rate < 1024 * 1024:
            return f"{rate / 1024:6.1f}KB/s"
        return f"{rate / (1024 * 1024):6.2f}MB/s"

    def _render(self) -> None:
        frac = self.current / self.total
        filled = int(self.width * frac)
        pct = frac * 100
        elapsed = max(time.monotonic() - self.start_t, 1e-6)
        cur = str(self.current).rjust(self._count_w)
        tot = str(self.total).rjust(self._count_w)
        if self.unit_bytes > 0 and self.current > 0:
            rate = self.current * self.unit_bytes / elapsed
            remain_s = (self.total - self.current) * self.unit_bytes / rate if rate else 0
        else:
            rate = self.current / elapsed if self.current else 0
            per_unit = elapsed / self.current if self.current else 0
            remain_s = (self.total - self.current) * per_unit
        mm, ss = divmod(int(remain_s), 60)
        rate_s = self._fmt_rate(rate) if self.unit_bytes > 0 else " " * 10
        # Green fill + default-colour track, with explicit reset so the
        # surrounding text stays uncoloured.
        bar_filled = "\x1b[32m" + "#" * filled + "\x1b[0m"
        bar_empty = "-" * (self.width - filled)
        line = (f"\r{_tag(self.label)} [{bar_filled}{bar_empty}] {cur}/{tot} {pct:5.1f}%  "
                f"{rate_s}  ETA {mm:02d}:{ss:02d}")
        self.stream.write(line)
        self.stream.flush()

    def close(self, status: str = "") -> None:
        if not self.enabled:
            return
        self._render()
        if status:
            self.stream.write(f"  {status}")
        self.stream.write("\n")
        self.stream.flush()


def _run_prep(client: IspClient, verbose: bool) -> None:
    """Follows isp_download_tool main() init sequence."""
    fid = client.cmd_read_flash_id()
    if verbose:
        print(f"{_tag('prep')} flash id: {fid:#010x}")
    client.cmd_read_status(1)
    client.cmd_read_status(2)
    client.cmd_enable_write()
    client.cmd_enable_qpsi()
    if verbose:
        print(f"{_tag('prep')} enable_write + enable_qpsi done")


def cmd_prep(args) -> int:
    client = _open_client(args)
    try:
        _run_prep(client, verbose=True)
    finally:
        client.close()
    print("prep done")
    return 0


def write_image(client: "IspClient", addr: int, payload: bytes,
                no_erase: bool = False, verbose: bool = False,
                erase_4k: bool = False) -> None:
    """Erase + program a pre-connected client. Caller owns connect/prep/close."""
    if not payload:
        raise ValueError("payload is empty")
    total_pages = (len(payload) + PAGE_SIZE - 1) // PAGE_SIZE

    if not no_erase:
        aligned_len = total_pages * PAGE_SIZE
        # ROM ISP only reliably erases with 32K/64K outside MP sector,
        # so round erase length up to 64K alignment.
        erase_len = (aligned_len + 0xFFFF) & ~0xFFFF
        eplan = list(plan_erase(addr, erase_len, sector_only=erase_4k))
        epb = ProgressBar(len(eplan), label="erase")
        for eaddr, mode in eplan:
            time.sleep(0.001)
            client.cmd_erase(eaddr, mode)
            epb.update()
        epb.close("done")

    wpb = ProgressBar(total_pages, label="write", unit_bytes=PAGE_SIZE)
    start_t = time.monotonic()
    page_list = [
        (addr + i * PAGE_SIZE,
         _pad_page(payload[i * PAGE_SIZE : (i + 1) * PAGE_SIZE]))
        for i in range(total_pages)
    ]
    client.write_pages_pipelined(
        page_list, depth=8, on_progress=lambda n: wpb.update(n),
    )
    wpb.close("done")
    elapsed = time.monotonic() - start_t
    total_bytes = total_pages * PAGE_SIZE
    print(f"{_tag('OK')} {len(payload):>7} bytes -> {addr:#010x}  "
          f"({elapsed:5.1f}s, {total_bytes / 1024 / max(elapsed, 0.001):6.1f} KB/s)")


def cmd_write(args) -> int:
    payload = Path(args.file).read_bytes()
    if not payload:
        print("[error] file is empty")
        return 2

    client = _open_client(args)
    try:
        _run_prep(client, verbose=args.verbose)
        write_image(client, args.addr, payload,
                    no_erase=args.no_erase, verbose=args.verbose)
    finally:
        client.close()
    return 0


def _cmd_write_with_addr(args, addr: int) -> int:
    args.addr = addr
    if not hasattr(args, "no_erase"):
        args.no_erase = False
    rc = cmd_write(args)
    if rc == 0 and getattr(args, "monitor", False):
        mon_baud = getattr(args, "monitor_baud", None) or args.baudrate
        print(f"{_tag('monitor')} switching to monitor mode, press reset "
              "on EVK to see app output")
        return _run_monitor(args.port, mon_baud)
    return rc


def cmd_boot(args) -> int:
    addr = CHIP_LAYOUT[args.chip]["boot"]
    print(f"{_tag('chip')} {args.chip}  bootloader @ {addr:#010x}")
    return _cmd_write_with_addr(args, addr)


def cmd_app(args) -> int:
    addr = CHIP_LAYOUT[args.chip]["app"]
    print(f"{_tag('chip')} {args.chip}  application @ {addr:#010x}")
    return _cmd_write_with_addr(args, addr)


def cmd_jump(args) -> int:
    client = _open_client(args)
    try:
        client.cmd_jump(to_sram=args.sram)
    finally:
        client.close()
    print("jump issued")
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="isp-cli",
        description="Rafael bootloader-app ISP client",
    )
    p.add_argument("--port", default="COM18",
                   help="serial port (default: COM18)")
    p.add_argument("--baudrate", type=int, default=DEFAULT_BAUDRATE)
    p.add_argument("--timeout", type=float, default=DEFAULT_TIMEOUT,
                   help="per-command timeout in seconds")
    p.add_argument("--wait", type=float, default=10.0,
                   help="seconds to wait for RESET/handshake (default: 10)")
    p.add_argument("--auto-reset", action="store_true",
                   help="pulse DTR to reset EVK via USB-UART bridge")
    p.add_argument("-v", "--verbose", action="store_true")

    sub = p.add_subparsers(dest="command", required=True)

    sub.add_parser("monitor", help="raw serial monitor (no handshake)").set_defaults(
        func=cmd_monitor)

    sub.add_parser("ping", help="handshake test (0xAA -> 0x55)").set_defaults(
        func=cmd_ping)

    sub.add_parser("prep", help="connect + flash_id + enable_write + qpsi").set_defaults(
        func=cmd_prep)

    sub.add_parser("chip-id", help="query chip id (0x0C)").set_defaults(
        func=cmd_chipid)

    sub.add_parser("flash-id", help="query flash device id (0x04)").set_defaults(
        func=cmd_flashid)

    sp = sub.add_parser("status", help="read flash status register (0x05)")
    sp.add_argument("--mode", type=int, default=1, choices=[1, 2])
    sp.set_defaults(func=cmd_status)

    sp = sub.add_parser("erase", help="erase flash region")
    sp.add_argument("--addr", type=lambda s: int(s, 0), required=True)
    sp.add_argument("--length", type=lambda s: int(s, 0), required=True)
    sp.set_defaults(func=cmd_erase)

    sp = sub.add_parser("write", help="erase+write a binary file")
    sp.add_argument("--addr", type=lambda s: int(s, 0), required=True)
    sp.add_argument("--file", required=True)
    sp.add_argument("--no-erase", action="store_true",
                    help="skip erase (caller pre-erased)")
    sp.set_defaults(func=cmd_write)

    for name, fn, label in (("boot", cmd_boot, "bootloader"),
                            ("app", cmd_app, "application")):
        sp = sub.add_parser(name, help=f"flash {label} to chip-default address")
        sp.add_argument("--chip", required=True, choices=list(CHIP_LAYOUT))
        sp.add_argument("--file", required=True)
        sp.add_argument("--no-erase", action="store_true",
                        help="skip erase (caller pre-erased)")
        sp.add_argument("--monitor", action="store_true",
                        help="open serial monitor after flash")
        sp.add_argument("--monitor-baud", type=int, default=None,
                        help="baudrate for monitor (default: same as --baudrate)")
        sp.set_defaults(func=fn)

    sp = sub.add_parser("jump", help="jump to flash or SRAM (0x0A)")
    sp.add_argument("--sram", action="store_true")
    sp.set_defaults(func=cmd_jump)

    return p


def main(argv: Optional[list[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        return args.func(args)
    except SystemExit:
        raise
    except Exception as e:
        print(f"[error] {type(e).__name__}: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
