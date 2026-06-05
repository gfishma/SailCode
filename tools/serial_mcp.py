#!/usr/bin/env python3
"""
MCP server for STM32 serial (SCMD) commands.
Implements JSON-RPC 2.0 over stdio.
Provides: mcp_tool_send_command(port, baud, cmd, timeout_sec)
"""

import json
import sys
import serial
import time


def send_command(port: str, baud: int, cmd: str, timeout_sec: float = 2.0):
    """Send an SCMD command and return the response."""
    ser = serial.Serial(port, baud, timeout=0.5)
    ser.reset_input_buffer()

    ser.write(cmd.encode() + b"\r\n")
    ser.flush()

    # Read line by line until idle
    deadline = time.time() + timeout_sec
    lines = []
    while time.time() < deadline:
        line = ser.readline().decode("utf-8", errors="replace")
        if line:
            lines.append(line)
        else:
            break  # timeout — no more data

    ser.close()
    resp = "".join(lines)

    # extract SCMD response (skip echo line starting with >, include everything else)
    raw_lines = resp.replace("\r\n", "\n").split("\n")
    resp_lines = [l.strip("\r") for l in raw_lines if l.strip() and not l.strip().startswith(">")]
    result = "\n".join(resp_lines).strip()

    return {"response": result, "raw": resp.strip()}


def main():
    # Minimal MCP JSON-RPC loop over stdio
    # Expects: {"jsonrpc":"2.0","method":"tools/call","params":{"name":"send_command","arguments":{...}},"id":1}
    for raw in sys.stdin:
        raw = raw.strip()
        if not raw:
            continue
        try:
            req = json.loads(raw)
            method = req.get("method", "")
            req_id = req.get("id", 0)

            if method == "tools/list":
                # Respond with tool list
                resp = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "tools": [{
                            "name": "send_command",
                            "description": "Send an SCMD command to the STM32 over serial and return response lines",
                            "inputSchema": {
                                "type": "object",
                                "properties": {
                                    "port": {"type": "string", "default": "COM4", "description": "Serial port"},
                                    "baud": {"type": "integer", "default": 115200, "description": "Baud rate"},
                                    "cmd": {"type": "string", "description": "SCMD command to send (e.g. '>em_cvs info')"},
                                    "timeout_sec": {"type": "number", "default": 1.0, "description": "Read timeout in seconds"}
                                },
                                "required": ["cmd"]
                            }
                        }]
                    }
                }
                sys.stdout.write(json.dumps(resp) + "\n")
                sys.stdout.flush()

            elif method == "tools/call":
                args = req.get("params", {}).get("arguments", {})
                port = args.get("port", "COM4")
                baud = args.get("baud", 115200)
                cmd = args.get("cmd", "")
                timeout_sec = args.get("timeout_sec", 1.0)

                if not cmd:
                    raise ValueError("cmd is required")

                result = send_command(port, baud, cmd, timeout_sec)

                resp = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "content": [{"type": "text", "text": result["response"]}]
                    }
                }
                sys.stdout.write(json.dumps(resp) + "\n")
                sys.stdout.flush()

            elif method == "initialize":
                resp = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "protocolVersion": "2024-11-05",
                        "serverInfo": {"name": "serial-mcp", "version": "1.0.0"},
                        "capabilities": {"tools": {}}
                    }
                }
                sys.stdout.write(json.dumps(resp) + "\n")
                sys.stdout.flush()

        except Exception as e:
            err_resp = {
                "jsonrpc": "2.0",
                "id": req.get("id", 0) if 'req' in dir() else 0,
                "error": {"code": -1, "message": str(e)}
            }
            sys.stdout.write(json.dumps(err_resp) + "\n")
            sys.stdout.flush()


if __name__ == "__main__":
    main()
