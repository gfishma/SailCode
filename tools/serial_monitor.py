#!/usr/bin/env python3
"""
Serial monitor for STM32 UART debug output.
Usage:
  python serial_monitor.py                    # interactive mode, print to console
  python serial_monitor.py --capture 10       # capture 10 seconds, save to log
  python serial_monitor.py --capture 10 --cmd "get_temp\r\n"  # send command, then capture
"""

import serial
import sys
import time
import argparse
import os
import threading
from datetime import datetime

DEFAULT_PORT = "COM4"
DEFAULT_BAUD = 115200
LOG_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "logs")


def timestamp():
    return datetime.now().strftime("%H:%M:%S.%f")[:-3]


def ensure_log_dir():
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)


def capture_mode(port, baud, duration, command=None):
    """Capture serial output for N seconds, save to dated log file."""
    ensure_log_dir()
    date_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_path = os.path.join(LOG_DIR, f"serial_{date_str}.log")

    ser = serial.Serial(port, baud, timeout=1.0)
    ser.reset_input_buffer()

    print(f"[{timestamp()}] Serial capture started: {port} @ {baud} baud")
    print(f"[{timestamp()}] Log: {log_path}")

    start = time.time()
    all_lines = []

    if command:
        if isinstance(command, str):
            # Interpret escape sequences like \r\n, \t, etc.
            command = command.replace('\\r', '\r').replace('\\n', '\n').replace('\\t', '\t').replace('\\\\', '\\')
        cmd_bytes = command.encode() if isinstance(command, str) else command
        ser.write(cmd_bytes)
        print(f"[{timestamp()}] Sent: {command!r}")
        # Wait briefly for echo + SCMD response to arrive
        time.sleep(0.15)

    with open(log_path, "w", encoding="utf-8") as f:
        f.write(f"# Serial capture: {port} @ {baud} baud\n")
        f.write(f"# Started: {datetime.now().isoformat()}\n")
        f.write(f"# Duration: {duration}s\n")
        f.write("#" + "=" * 60 + "\n\n")

        # Use bulk read + line split for reliability
        buffer = ""
        while time.time() - start < duration:
            try:
                chunk = ser.read(4096).decode("utf-8", errors="replace")
                if chunk:
                    buffer += chunk
                    # Process complete lines
                    while "\n" in buffer:
                        line, buffer = buffer.split("\n", 1)
                        line = line + "\n"
                        ts = timestamp()
                        text = f"[{ts}] {line}"
                        print(text, end="", flush=True)
                        f.write(f"[{ts}] {line}")
                        all_lines.append(line)
            except serial.SerialException as e:
                print(f"[{timestamp()}] Serial error: {e}")
                break
            except KeyboardInterrupt:
                break

        # Flush any remaining partial line
        if buffer:
            ts = timestamp()
            text = f"[{ts}] {buffer}"
            print(text, end="", flush=True)
            f.write(f"[{ts}] {buffer}")
            all_lines.append(buffer)

    ser.close()
    elapsed = time.time() - start
    print(f"\n[{timestamp()}] Capture finished: {len(all_lines)} lines in {elapsed:.1f}s")
    print(f"[{timestamp()}] Log saved to: {log_path}")
    return log_path, all_lines


def interactive_mode(port, baud):
    """Run in interactive mode — prints to console until Ctrl+C."""
    ser = serial.Serial(port, baud, timeout=0.1)
    ser.reset_input_buffer()
    print(f"[{timestamp()}] Serial monitor: {port} @ {baud} baud")
    print(f"[{timestamp()}] Press Ctrl+C to exit.\n")

    running = [True]

    def reader():
        while running[0]:
            try:
                line = ser.readline().decode("utf-8", errors="replace")
                if line:
                    print(f"[{timestamp()}] {line}", end="", flush=True)
            except serial.SerialException:
                break

    t = threading.Thread(target=reader, daemon=True)
    t.start()

    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass

    running[0] = False
    t.join(timeout=1)
    ser.close()
    print(f"\n[{timestamp()}] Monitor stopped.")


def list_ports():
    """List available serial ports."""
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return
    for p in ports:
        print(f"  {p.device} - {p.description} ({p.hwid})")


def main():
    parser = argparse.ArgumentParser(description="STM32 Serial Monitor")
    parser.add_argument("--port", "-p", default=DEFAULT_PORT, help=f"Serial port (default: {DEFAULT_PORT})")
    parser.add_argument("--baud", "-b", type=int, default=DEFAULT_BAUD, help=f"Baud rate (default: {DEFAULT_BAUD})")
    parser.add_argument("--capture", "-c", type=float, default=None, help="Capture mode: duration in seconds")
    parser.add_argument("--cmd", "-m", default=None, help="Command to send before capturing (e.g. 'get_temp\\r\\n')")
    parser.add_argument("--list", "-l", action="store_true", help="List available serial ports")
    parser.add_argument("--last-log", action="store_true", help="Print the path of the most recent log file")

    args = parser.parse_args()

    if args.list:
        list_ports()
        return

    if args.last_log:
        ensure_log_dir()
        logs = sorted([f for f in os.listdir(LOG_DIR) if f.endswith(".log")], reverse=True)
        if logs:
            print(os.path.join(LOG_DIR, logs[0]))
        else:
            print("No logs found.")
        return

    if args.capture is not None:
        capture_mode(args.port, args.baud, args.capture, command=args.cmd)
    else:
        interactive_mode(args.port, args.baud)


if __name__ == "__main__":
    main()
