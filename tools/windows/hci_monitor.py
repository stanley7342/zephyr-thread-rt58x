#!/usr/bin/env python3
"""
RT58x BLE HCI Monitor — parses UART log from the HCI driver and validates
the HCI/ACL data flow.

Usage:
    python hci_monitor.py COM5              # live monitor (default 115200)
    python hci_monitor.py COM5 -b 115200    # explicit baud
    python hci_monitor.py --file log.txt    # parse saved log file

Checks performed:
    - HCI command opcode format and param length
    - HCI event code, param length, opcode match for CMD_COMPLETE/CMD_STATUS
    - ACL data length consistency
    - GATT service discovery flow (Read By Group Type / response)
    - CCCD write detection (notification enable)
    - TX/RX opcode pairing (sent command → matching response)
    - Timeout detection (command without response)
"""

import sys
import re
import argparse
from collections import deque
from datetime import datetime

# ANSI color codes
class C:
    RESET   = "\033[0m"
    BOLD    = "\033[1m"
    # HCI commands/events
    TX_CMD  = "\033[36m"    # cyan — outgoing HCI command
    RX_EVT  = "\033[32m"    # green — incoming HCI event
    # ACL data
    TX_ACL  = "\033[35m"    # magenta — outgoing ACL
    RX_ACL  = "\033[33m"    # yellow — incoming ACL
    # ATT decode
    ATT     = "\033[93m"    # bright yellow — ATT decode
    # Status
    OK      = "\033[92m"    # bright green
    WARN    = "\033[91m"    # bright red
    INFO    = "\033[90m"    # gray — raw lines
    BLE     = "\033[94m"    # bright blue — [BLE]/[HRS] lines
    STAR    = "\033[95m"    # bright magenta — notification enabled

# HCI opcode lookup
HCI_OPCODES = {
    0x0C01: "Set_Event_Mask",
    0x0C03: "HCI_Reset",
    0x1001: "Read_Local_Version_Info",
    0x1002: "Read_Local_Supported_Commands",
    0x1003: "Read_Local_Supported_Features",
    0x1009: "Read_BD_ADDR",
    0x2001: "LE_Set_Event_Mask",
    0x2002: "LE_Read_Buffer_Size",
    0x2003: "LE_Read_Local_Supported_Features",
    0x2005: "LE_Set_Random_Address",
    0x2006: "LE_Set_Advertising_Parameters",
    0x2008: "LE_Set_Advertising_Data",
    0x2009: "LE_Set_Scan_Response_Data",
    0x200A: "LE_Set_Advertising_Enable",
    0x200F: "LE_Read_Filter_Accept_List_Size",
    0x2013: "LE_Connection_Update",
    0x2016: "LE_Read_Remote_Features",
    0x2018: "LE_Rand",
    0x201C: "LE_Read_Supported_States",
    0xFC01: "Vendor_Set_Controller_Info",
}

# ATT opcodes
ATT_OPCODES = {
    0x01: "Error_Response",
    0x02: "Exchange_MTU_Request",
    0x03: "Exchange_MTU_Response",
    0x04: "Find_Information_Request",
    0x05: "Find_Information_Response",
    0x06: "Find_By_Type_Value_Request",
    0x07: "Find_By_Type_Value_Response",
    0x08: "Read_By_Type_Request",
    0x09: "Read_By_Type_Response",
    0x0A: "Read_Request",
    0x0B: "Read_Response",
    0x10: "Read_By_Group_Type_Request",
    0x11: "Read_By_Group_Type_Response",
    0x12: "Write_Request",
    0x13: "Write_Response",
    0x1B: "Handle_Value_Notification",
    0x52: "Write_Command",
}

# BLE UUID16
UUID16_NAMES = {
    0x1800: "GAP",
    0x1801: "GATT",
    0x180A: "Device_Information",
    0x180D: "Heart_Rate",
    0x180F: "Battery",
    0x2800: "Primary_Service",
    0x2801: "Secondary_Service",
    0x2802: "Include",
    0x2803: "Characteristic",
    0x2902: "CCCD",
    0x2A00: "Device_Name",
    0x2A01: "Appearance",
    0x2A19: "Battery_Level",
    0x2A29: "Manufacturer_Name",
    0x2A37: "Heart_Rate_Measurement",
    0x2A38: "Body_Sensor_Location",
}


class HCIMonitor:
    def __init__(self):
        self.pending_cmds = deque()  # (opcode, timestamp)
        self.services_found = []
        self.cccd_enabled = False
        self.connected = False
        self.errors = []
        self.warnings = []
        self.stats = {
            "tx_cmd": 0, "rx_evt": 0, "tx_acl": 0, "rx_acl": 0,
            "cmd_complete": 0, "cmd_status": 0, "notifications": 0,
        }

    def uuid_name(self, u16):
        return UUID16_NAMES.get(u16, f"0x{u16:04X}")

    def opcode_name(self, opcode):
        return HCI_OPCODES.get(opcode, f"0x{opcode:04X}")

    def parse_hex(self, hex_str):
        """Parse hex string like '01 02 0a ff' into bytes list."""
        return [int(x, 16) for x in hex_str.strip().split()]

    def check_tx_cmd(self, raw):
        """Validate outgoing HCI command."""
        if len(raw) < 4:
            self.errors.append(f"TX CMD too short: {len(raw)} bytes")
            return
        if raw[0] != 0x01:
            self.errors.append(f"TX CMD bad H:4 type: 0x{raw[0]:02X} (expected 0x01)")
            return

        opcode = raw[2] << 8 | raw[1]
        plen = raw[3]
        actual_plen = len(raw) - 4

        self.stats["tx_cmd"] += 1
        name = self.opcode_name(opcode)
        ogf = opcode >> 10
        ocf = opcode & 0x3FF

        if plen != actual_plen:
            self.errors.append(
                f"TX CMD {name}: plen={plen} but actual payload={actual_plen}")
        else:
            print(f"  {C.TX_CMD}✓ TX CMD {name} (OGF=0x{ogf:02X} OCF=0x{ocf:04X}) plen={plen}{C.RESET}")

        self.pending_cmds.append((opcode, datetime.now()))

    def check_rx_evt(self, raw):
        """Validate incoming HCI event."""
        if len(raw) < 3:
            self.errors.append(f"RX EVT too short: {len(raw)} bytes")
            return
        if raw[0] != 0x04:
            self.errors.append(f"RX EVT bad H:4 type: 0x{raw[0]:02X} (expected 0x04)")
            return

        evt_code = raw[1]
        plen = raw[2]
        actual_plen = len(raw) - 3

        self.stats["rx_evt"] += 1

        if plen != actual_plen:
            self.warnings.append(
                f"RX EVT 0x{evt_code:02X}: plen={plen} but actual={actual_plen}")

        if evt_code == 0x0E:  # Command Complete
            if plen < 4:
                self.errors.append("CMD_COMPLETE too short")
                return
            ncmd = raw[3]
            opcode = raw[5] << 8 | raw[4]
            status = raw[6]
            self.stats["cmd_complete"] += 1
            name = self.opcode_name(opcode)

            # Match with pending command
            matched = False
            for i, (pending_op, _) in enumerate(self.pending_cmds):
                if pending_op == opcode:
                    self.pending_cmds.remove((pending_op, _))
                    matched = True
                    break

            status_str = f"{C.OK}OK{C.RESET}" if status == 0 else f"{C.WARN}FAIL(0x{status:02X}){C.RESET}"
            match_str = f"{C.OK}matched{C.RESET}" if matched else f"{C.WARN}UNMATCHED!{C.RESET}"
            if not matched:
                self.warnings.append(f"CMD_COMPLETE for {name} but no pending TX")
            if status != 0:
                self.warnings.append(f"CMD_COMPLETE {name} status=0x{status:02X}")

            print(f"  {C.RX_EVT}✓ RX CMD_COMPLETE {name}{C.RESET} status={status_str} [{match_str}]")

        elif evt_code == 0x0F:  # Command Status
            if plen < 4:
                self.errors.append("CMD_STATUS too short")
                return
            status = raw[3]
            ncmd = raw[4]
            opcode = raw[6] << 8 | raw[5]
            self.stats["cmd_status"] += 1
            name = self.opcode_name(opcode)
            status_str = f"{C.OK}OK{C.RESET}" if status == 0 else f"{C.WARN}FAIL(0x{status:02X}){C.RESET}"
            print(f"  {C.RX_EVT}✓ RX CMD_STATUS {name}{C.RESET} status={status_str}")

        elif evt_code == 0x3E:  # LE Meta Event
            if plen >= 1:
                subevent = raw[3]
                if subevent == 0x01:
                    print(f"  {C.RX_EVT}✓ RX LE_CONNECTION_COMPLETE{C.RESET}")
                    self.connected = True
                elif subevent == 0x03:
                    print(f"  {C.RX_EVT}✓ RX LE_CONNECTION_UPDATE_COMPLETE{C.RESET}")
                elif subevent == 0x04:
                    print(f"  {C.RX_EVT}✓ RX LE_READ_REMOTE_FEATURES_COMPLETE{C.RESET}")
                else:
                    print(f"  {C.RX_EVT}✓ RX LE_META subevent=0x{subevent:02X}{C.RESET}")

        elif evt_code == 0x05:  # Disconnection Complete
            print(f"  {C.WARN}✓ RX DISCONNECTION_COMPLETE{C.RESET}")
            self.connected = False

        elif evt_code == 0x13:  # Number of Completed Packets
            print(f"  {C.RX_EVT}✓ RX NUM_COMPLETED_PACKETS{C.RESET}")

        else:
            print(f"  {C.RX_EVT}✓ RX EVT code=0x{evt_code:02X} plen={plen}{C.RESET}")

    def check_acl(self, direction, raw):
        """Validate ACL data and decode ATT layer."""
        if len(raw) < 5:
            self.errors.append(f"{direction} ACL too short: {len(raw)} bytes")
            return

        handle_flags = raw[2] << 8 | raw[1]
        handle = handle_flags & 0x0FFF
        pb_flag = (handle_flags >> 12) & 0x03
        acl_len = raw[4] << 8 | raw[3]

        if direction == "TX":
            self.stats["tx_acl"] += 1
        else:
            self.stats["rx_acl"] += 1

        if len(raw) - 5 != acl_len:
            self.warnings.append(
                f"{direction} ACL: header says {acl_len} bytes but got {len(raw)-5}")

        # Decode L2CAP
        if acl_len >= 4:
            l2cap_len = raw[6] << 8 | raw[5]
            l2cap_cid = raw[8] << 8 | raw[7]

            color = C.TX_ACL if direction == "TX" else C.RX_ACL
            if l2cap_cid == 0x0004:  # ATT
                self.decode_att(direction, raw[9:9+l2cap_len], handle)
            else:
                print(f"  {color}✓ {direction} ACL handle={handle} L2CAP CID=0x{l2cap_cid:04X} len={l2cap_len}{C.RESET}")
        else:
            color = C.TX_ACL if direction == "TX" else C.RX_ACL
            print(f"  {color}✓ {direction} ACL handle={handle} len={acl_len}{C.RESET}")

    def decode_att(self, direction, att_data, handle):
        """Decode ATT PDU."""
        if len(att_data) < 1:
            return

        opcode = att_data[0]
        name = ATT_OPCODES.get(opcode, f"0x{opcode:02X}")
        color = C.TX_ACL if direction == "TX" else C.RX_ACL
        prefix = f"  {color}✓ {direction} ATT {name}{C.RESET}"

        if opcode == 0x10 and len(att_data) >= 7:
            # Read By Group Type Request
            start = att_data[2] << 8 | att_data[1]
            end = att_data[4] << 8 | att_data[3]
            uuid = att_data[6] << 8 | att_data[5]
            print(f"{prefix}: handles={start}-{end} uuid={self.uuid_name(uuid)}")

        elif opcode == 0x11 and len(att_data) >= 2:
            # Read By Group Type Response
            entry_len = att_data[1]
            entries = att_data[2:]
            services = []
            i = 0
            while i + entry_len <= len(entries):
                start = entries[i+1] << 8 | entries[i]
                end = entries[i+3] << 8 | entries[i+2]
                if entry_len >= 6:
                    uuid = entries[i+5] << 8 | entries[i+4]
                    svc_name = self.uuid_name(uuid)
                    services.append(svc_name)
                    self.services_found.append(svc_name)
                i += entry_len
            print(f"{prefix}: services=[{', '.join(services)}]")

        elif opcode == 0x08 and len(att_data) >= 7:
            # Read By Type Request
            start = att_data[2] << 8 | att_data[1]
            end = att_data[4] << 8 | att_data[3]
            uuid = att_data[6] << 8 | att_data[5]
            print(f"{prefix}: handles={start}-{end} uuid={self.uuid_name(uuid)}")

        elif opcode == 0x52 and len(att_data) >= 3:
            # Write Command (CCCD write)
            attr_handle = att_data[2] << 8 | att_data[1]
            if len(att_data) >= 5:
                value = att_data[4] << 8 | att_data[3]
                if value & 0x01:
                    print(f"{prefix}: handle=0x{attr_handle:04X} {C.STAR}{C.BOLD}NOTIFICATION ENABLED ★{C.RESET}")
                    self.cccd_enabled = True
                else:
                    print(f"{prefix}: handle=0x{attr_handle:04X} value=0x{value:04X}")
            else:
                print(f"{prefix}: handle=0x{attr_handle:04X}")

        elif opcode == 0x12 and len(att_data) >= 3:
            # Write Request (CCCD write)
            attr_handle = att_data[2] << 8 | att_data[1]
            if len(att_data) >= 5:
                value = att_data[4] << 8 | att_data[3]
                if value & 0x01:
                    print(f"{prefix}: handle=0x{attr_handle:04X} {C.STAR}{C.BOLD}NOTIFICATION ENABLED ★{C.RESET}")
                    self.cccd_enabled = True
                else:
                    print(f"{prefix}: handle=0x{attr_handle:04X} value=0x{value:04X}")
            else:
                print(f"{prefix}: handle=0x{attr_handle:04X}")

        elif opcode == 0x1B and len(att_data) >= 3:
            # Handle Value Notification
            attr_handle = att_data[2] << 8 | att_data[1]
            self.stats["notifications"] += 1
            print(f"{prefix}: handle=0x{attr_handle:04X} data_len={len(att_data)-3}")

        elif opcode == 0x02 and len(att_data) >= 3:
            # Exchange MTU Request
            mtu = att_data[2] << 8 | att_data[1]
            print(f"{prefix}: client_mtu={mtu}")

        elif opcode == 0x03 and len(att_data) >= 3:
            # Exchange MTU Response
            mtu = att_data[2] << 8 | att_data[1]
            print(f"{prefix}: server_mtu={mtu}")

        elif opcode == 0x01 and len(att_data) >= 5:
            # Error Response
            req_opcode = att_data[1]
            attr_handle = att_data[3] << 8 | att_data[2]
            error_code = att_data[4]
            error_names = {0x0A: "ATTR_NOT_FOUND", 0x06: "REQ_NOT_SUPP"}
            err_name = error_names.get(error_code, f"0x{error_code:02X}")
            print(f"{prefix}: req=0x{req_opcode:02X} handle=0x{attr_handle:04X} err={err_name}")

        else:
            print(f"{prefix}: len={len(att_data)}")

    def process_line(self, line):
        """Process one UART log line."""
        line = line.strip()
        if not line:
            return

        # Pass through non-HCI lines
        if "[BLE]" in line or "[HRS]" in line:
            print(f"  {C.BLE}{line}{C.RESET}")
            return

        # Show all raw HCI driver lines verbatim with color
        if "[HCI]" in line:
            if "TX-SETUP" in line or ("TX[" in line and "01 " == line.split("]:")[-1].strip()[:3]):
                print(f"  {C.TX_CMD}{line}{C.RESET}")
            elif "RX-ACL" in line or "ACL-IN" in line:
                print(f"  {C.RX_ACL}{line}{C.RESET}")
            elif "ACL-OUT" in line:
                print(f"  {C.TX_ACL}{line}{C.RESET}")
            elif "RX[" in line:
                print(f"  {C.RX_EVT}{line}{C.RESET}")
            elif "TX[" in line:
                # Check if ACL (starts with 02) or CMD (starts with 01)
                hex_part = line.split("]: ")[-1] if "]: " in line else ""
                if hex_part.startswith("02"):
                    print(f"  {C.TX_ACL}{line}{C.RESET}")
                else:
                    print(f"  {C.TX_CMD}{line}{C.RESET}")
            else:
                print(f"  {C.INFO}{line}{C.RESET}")

        # Match TX raw hex: [HCI] TX[N]: xx xx xx ...
        m = re.search(r'\[HCI\]\s+TX\[(\d+)\]:\s+((?:[0-9a-f]{2}\s*)+)', line, re.I)
        if m:
            raw = self.parse_hex(m.group(2))
            if raw and raw[0] == 0x01:
                self.check_tx_cmd(raw)
            elif raw and raw[0] == 0x02:
                self.check_acl("TX", raw)
            return

        # Match RX raw hex: [HCI] RX[N]: xx xx xx ...
        m = re.search(r'\[HCI\]\s+RX\[(\d+)\]:\s+((?:[0-9a-f]{2}\s*)+)', line, re.I)
        if m:
            raw = self.parse_hex(m.group(2))
            if raw and raw[0] == 0x04:
                self.check_rx_evt(raw)
            return

        # Match RX-ACL
        m = re.search(r'\[HCI\]\s+RX-ACL\[(\d+)\]:\s+((?:[0-9a-f]{2}\s*)+)', line, re.I)
        if m:
            raw = self.parse_hex(m.group(2))
            self.check_acl("RX", raw)
            return

        # Match ACL-IN/OUT len (just info)
        m = re.search(r'\[HCI\]\s+ACL-(IN|OUT)\s+len=(\d+)', line)
        if m:
            direction = m.group(1)
            buf_len = int(m.group(2))
            return

        # Match TX-SETUP
        m = re.search(r'\[HCI\]\s+TX-SETUP\[(\d+)\]:\s+((?:[0-9a-f]{2}\s*)+)', line, re.I)
        if m:
            raw = self.parse_hex(m.group(2))
            if raw and raw[0] == 0x01:
                self.check_tx_cmd(raw)
            return

    def print_summary(self):
        """Print analysis summary."""
        print("\n" + "=" * 50)
        print("  HCI Monitor Summary")
        print("=" * 50)
        print(f"  TX Commands:        {self.stats['tx_cmd']}")
        print(f"  RX Events:          {self.stats['rx_evt']}")
        print(f"    CMD_COMPLETE:     {self.stats['cmd_complete']}")
        print(f"    CMD_STATUS:       {self.stats['cmd_status']}")
        print(f"  TX ACL:             {self.stats['tx_acl']}")
        print(f"  RX ACL:             {self.stats['rx_acl']}")
        print(f"  Notifications:      {self.stats['notifications']}")

        if self.services_found:
            print(f"\n  Services discovered: {', '.join(self.services_found)}")
        else:
            print(f"\n  ⚠ No GATT service discovery seen!")
            print(f"    (phone may be using cached services)")

        if self.cccd_enabled:
            print(f"  ★ CCCD notification enabled")
        else:
            print(f"  ⚠ CCCD notification NOT enabled")

        if self.pending_cmds:
            print(f"\n  ⚠ {len(self.pending_cmds)} commands without response:")
            for op, ts in self.pending_cmds:
                print(f"    - {self.opcode_name(op)}")

        if self.errors:
            print(f"\n  ✗ ERRORS ({len(self.errors)}):")
            for e in self.errors:
                print(f"    - {e}")

        if self.warnings:
            print(f"\n  ⚠ WARNINGS ({len(self.warnings)}):")
            for w in self.warnings:
                print(f"    - {w}")

        if not self.errors and not self.warnings and not self.pending_cmds:
            print(f"\n  ✓ All checks passed!")
        print()


def enable_ansi_colors():
    """Enable ANSI escape codes on Windows."""
    if sys.platform == "win32":
        import ctypes
        kernel32 = ctypes.windll.kernel32
        kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)

def main():
    enable_ansi_colors()
    parser = argparse.ArgumentParser(description="RT58x BLE HCI Monitor")
    parser.add_argument("port", nargs="?", help="COM port (e.g. COM5)")
    parser.add_argument("-b", "--baud", type=int, default=115200)
    parser.add_argument("-f", "--file", help="Parse saved log file instead of live COM")
    args = parser.parse_args()

    monitor = HCIMonitor()

    if args.file:
        print(f"Parsing log file: {args.file}\n")
        with open(args.file, "r") as f:
            for line in f:
                monitor.process_line(line)
        monitor.print_summary()
        return

    if not args.port:
        print("Usage: python hci_monitor.py COM5")
        print("       python hci_monitor.py --file log.txt")
        sys.exit(1)

    try:
        import serial
    except ImportError:
        print("Install pyserial: pip install pyserial")
        sys.exit(1)

    print(f"Monitoring {args.port} @ {args.baud} baud (Ctrl+C to stop)\n")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        while True:
            line = ser.readline().decode("utf-8", errors="replace")
            if line:
                monitor.process_line(line)
    except KeyboardInterrupt:
        print("\n\nStopped by user.")
        monitor.print_summary()
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
