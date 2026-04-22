#!/usr/bin/env python3
"""Direct 1:1 port of isp_download_tool.c for byte-level diagnosis.

Behavioural parity (intentional, even quirks):
  * Handshake: write 0xAA, best-effort read; then loop { write 0x69; read } until 0x6A.
  * sleep(1) after handshake, matching main().
  * No bare 0xC0 before first SLIP packet.
  * seq starts at 1, increments on every successful response.
  * packet header length = cmd_len (raw), CRC16 over (cmd_len+5) bytes.
  * Every byte sent/received is dumped to stdout with [TX]/[RX] prefix.

Run:
  python tools/isp_cli/isp_portref.py --port COM18
"""
from __future__ import annotations

import argparse
import sys
import time

import serial

# ---- constants ------------------------------------------------------------
DEFAULT_PORT = "COM18"
DEFAULT_BAUD = 2_000_000
SLIP_DELIMITER = 0xC0
SLIP_ESC = 0xDB
SLIP_ESC_DELIM = 0xDC
SLIP_ESC_ESC = 0xDD
RT_ISP_SYNC_HEADER = 0x52


# ---- CRC table from isp_download_tool.c ----------------------------------
_CRC16_TABLE: list[int] = []


def _init_crc_table() -> None:
    for i in range(256):
        c = i << 8
        for _ in range(8):
            c = ((c << 1) ^ 0x1021) if (c & 0x8000) else (c << 1)
            c &= 0xFFFF
        _CRC16_TABLE.append(c)


_init_crc_table()


def crc16_ccitt(buf: bytes) -> int:
    crc = 0xFFFF
    for b in buf:
        crc = _CRC16_TABLE[((crc >> 8) ^ b) & 0xFF] ^ ((crc << 8) & 0xFFFF)
    return crc & 0xFFFF


# ---- global state (mirrors reference globals) -----------------------------
sequence = 1  # matches `static uint8_t sequence = 1;`


# ---- IO helpers with logging ---------------------------------------------
def _dump(tag: str, data: bytes) -> None:
    if not data:
        return
    print(f"[{tag}] {data.hex(' ')}")


def write_port(ser: serial.Serial, buf: bytes) -> None:
    _dump("TX", buf)
    ser.write(buf)
    ser.flush()


def read_port(ser: serial.Serial, n: int, timeout: float = 1.0) -> bytes:
    """Read exactly n bytes or timeout. Returns whatever was received."""
    deadline = time.monotonic() + timeout
    buf = bytearray()
    while len(buf) < n and time.monotonic() < deadline:
        b = ser.read(n - len(buf))
        if b:
            buf.extend(b)
    _dump("RX", bytes(buf))
    return bytes(buf)


def read_byte(ser: serial.Serial, timeout: float = 1.0) -> int | None:
    b = read_port(ser, 1, timeout)
    return b[0] if b else None


# ---- SLIP encode (matches slip_packet) -----------------------------------
def slip_packet(cmd_buf: bytes, seq: int) -> bytes:
    cmd_len = len(cmd_buf)
    dp = bytearray(cmd_len + 7)
    dp[0] = RT_ISP_SYNC_HEADER
    dp[1] = seq & 0xFF
    dp[2] = (cmd_len >> 8) & 0xFF
    dp[3] = cmd_len & 0xFF
    dp[4] = (~(dp[1] + dp[2] + dp[3])) & 0xFF
    dp[5 : 5 + cmd_len] = cmd_buf
    crc = crc16_ccitt(bytes(dp[: cmd_len + 5]))
    dp[cmd_len + 5] = (crc >> 8) & 0xFF
    dp[cmd_len + 6] = crc & 0xFF

    # SLIP wrap
    out = bytearray([SLIP_DELIMITER])
    for b in dp:
        if b == SLIP_DELIMITER:
            out += bytes([SLIP_ESC, SLIP_ESC_DELIM])
        elif b == SLIP_ESC:
            out += bytes([SLIP_ESC, SLIP_ESC_ESC])
        else:
            out.append(b)
    out.append(SLIP_DELIMITER)
    return bytes(out)


# ---- SLIP receive (matches newisp state machine) -------------------------
def recv_slip_frame(ser: serial.Serial, timeout: float = 2.0) -> bytes:
    """Return the unwrapped response_recv_buf content (starting at 0x52...)."""
    deadline = time.monotonic() + timeout

    # WAIT_SLIP_DELIMITER_STATE
    while time.monotonic() < deadline:
        b = read_byte(ser, 0.2)
        if b == SLIP_DELIMITER:
            break
    else:
        raise TimeoutError("no SLIP delimiter (start)")

    buf = bytearray()
    esc = False
    while time.monotonic() < deadline:
        b = read_byte(ser, 0.2)
        if b is None:
            continue
        if b == SLIP_DELIMITER:
            if not buf:
                continue  # ignore extra delimiters
            return bytes(buf)
        if esc:
            buf.append(SLIP_DELIMITER if b == SLIP_ESC_DELIM else SLIP_ESC)
            esc = False
        elif b == SLIP_ESC:
            esc = True
        else:
            buf.append(b)
    raise TimeoutError("no SLIP delimiter (end)")


# ---- high-level transact --------------------------------------------------
def send_command(ser: serial.Serial, cmd_buf: bytes, tag: str) -> bytes:
    """Send cmd_buf as a SLIP packet and return the response payload
       (content of rsp_buf equivalent — starts at cmd echo byte)."""
    global sequence
    print(f"\n=== {tag} (seq={sequence}) ===")
    pkt = slip_packet(cmd_buf, sequence)
    write_port(ser, pkt)

    frame = recv_slip_frame(ser)
    print(f"[FRAME] {frame.hex(' ')}")
    if len(frame) < 7 or frame[0] != RT_ISP_SYNC_HEADER:
        raise RuntimeError(f"bad frame start: {frame.hex()}")
    chksum = (~(frame[1] + frame[2] + frame[3])) & 0xFF
    if chksum != frame[4]:
        raise RuntimeError(f"bad header checksum: {frame[4]:#x} vs {chksum:#x}")
    length = (frame[2] << 8) | frame[3]
    body_end = 5 + length
    if len(frame) < body_end + 2:
        raise RuntimeError(f"frame truncated: len={len(frame)} need={body_end + 2}")
    want = crc16_ccitt(bytes(frame[:body_end]))
    got = (frame[body_end] << 8) | frame[body_end + 1]
    if want != got:
        raise RuntimeError(f"crc16 mismatch: got {got:#06x} want {want:#06x}")
    if frame[1] != sequence:
        print(f"[WARN] seq mismatch: got {frame[1]} want {sequence}")
    sequence = (sequence + 1) & 0xFF
    if sequence == 0:
        sequence = 1
    # Return content equivalent to reference's rsp_buf = frame[5 : 5+length]
    return bytes(frame[5:body_end])


# ---- connect handshake (staged: wait 0x55, then 0x69 -> 0x6A) ------------
def connect(ser: serial.Serial, wait_s: float = 15.0) -> None:
    print("[connect] waiting for device... press RESET on EVK now")
    ser.reset_input_buffer()
    deadline = time.monotonic() + wait_s

    # stage 1: poll 0xAA until 0x55
    while time.monotonic() < deadline:
        write_port(ser, b"\xAA")
        b = read_byte(ser, 0.05)
        if b == 0x55:
            print("[connect] got 0x55")
            break
    else:
        raise SystemExit("[connect] no 0x55 within timeout")

    # stage 2: send 0x69, expect 0x6A (retry until deadline)
    while time.monotonic() < deadline:
        write_port(ser, b"\x69")
        b = read_byte(ser, 0.1)
        if b == 0x6A:
            print("[connect] got 0x6A — ISP mode OK")
            return
    raise SystemExit("[connect] no 0x6A within timeout")


# ---- reference flow -------------------------------------------------------
def read_flash_id(ser: serial.Serial) -> int:
    rsp = send_command(ser, bytes([0x04, 0x00]), "read_flash_id")
    # rsp_buf[0]=0x04 echo, [1]=option, [2..5]=flash id LE
    fid = rsp[2] | (rsp[3] << 8) | (rsp[4] << 16) | (rsp[5] << 24)
    print(f"[flash_id] {fid:#010x}")
    return fid


def read_status_0(ser: serial.Serial) -> None:
    while True:
        rsp = send_command(ser, bytes([0x05, 0x01]), "read_status_0")
        if rsp[0] != 0x05:
            raise RuntimeError("bad status0 echo")
        if (rsp[1] & 0x0F) == 0:
            print(f"[status0] reg={rsp[2]:#04x}")
            return


def read_status_1(ser: serial.Serial) -> None:
    while True:
        rsp = send_command(ser, bytes([0x05, 0x02]), "read_status_1")
        if rsp[0] != 0x05:
            raise RuntimeError("bad status1 echo")
        if (rsp[1] & 0x0F) != 0x01:
            print(f"[status1] reg={rsp[2]:#04x}")
            return


def enable_write(ser: serial.Serial) -> None:
    while True:
        rsp = send_command(ser, bytes([0x06, 0x01, 0x00]), "enable_write")
        if rsp[0] != 0x06:
            raise RuntimeError("bad enable_write echo")
        if (rsp[1] & 0x0F) != 0x01:
            return


def enable_qpsi(ser: serial.Serial) -> None:
    payload = bytes([0x02, 0x00, 0x18, 0x00, 0x30, 0xA0, 0xEF, 0x01, 0x00, 0x00])
    while True:
        rsp = send_command(ser, payload, "enable_qpsi")
        if rsp[0] != 0x02:
            raise RuntimeError("bad enable_qpsi echo")
        if (rsp[1] & 0x0F) != 0x01:
            return


# ---- main -----------------------------------------------------------------
def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default=DEFAULT_PORT)
    ap.add_argument("--baudrate", type=int, default=DEFAULT_BAUD)
    ap.add_argument("--skip-extras", action="store_true",
                    help="only do connect + read_flash_id (minimum test)")
    args = ap.parse_args()

    ser = serial.Serial(args.port, baudrate=args.baudrate, timeout=0.05)
    try:
        connect(ser)
        print("[main] sleep(1) like reference ...")
        time.sleep(1)
        read_flash_id(ser)
        if args.skip_extras:
            return 0
        read_status_0(ser)
        read_status_1(ser)
        enable_write(ser)
        enable_qpsi(ser)
        print("\n[main] all reference init steps completed.")
        return 0
    finally:
        ser.close()


if __name__ == "__main__":
    sys.exit(main())
