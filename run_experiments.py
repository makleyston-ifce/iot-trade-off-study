#!/usr/bin/env python3
"""Run a randomized 3^3 experiment design and save results to CSV.

This script executes the existing NS-3 scratch application 27 times for all
combinations of:
  - Communication protocol: MQTT, COAP, AMQP
  - Security mode: NONE, TLS, MTLS
  - Number of sensor nodes: 200, 400, 800

Each experiment is executed in random order and the results are appended to a
single CSV file.

The script does not change the simulator implementation; it only automates
invocation and result collection.
"""

import csv
import random
import re
import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
WORKSPACE_ROOT = SCRIPT_DIR.parent.parent if SCRIPT_DIR.name == "iot-study" else SCRIPT_DIR.parent
EXECUTABLE = WORKSPACE_ROOT / "build" / "scratch" / "iot-study" / "ns3-dev-main-default"
OUTPUT_CSV = SCRIPT_DIR / "experiment_results.csv"

PROTOCOLS = ["MQTT", "COAP", "AMQP"]
SECURITIES = ["NONE", "TLS", "MTLS"]
NODE_COUNTS = [100, 200, 400]  # Reduced node counts for quicker testing; adjust as needed

METRIC_PATTERNS = {
    "Protocol": re.compile(r"^Protocol:\s*(.*)$"),
    "Security": re.compile(r"^Security:\s*(.*)$"),
    "Nodes": re.compile(r"^Sensors:\s*(\d+)$"),
    "Duration": re.compile(r"^Duration:\s*(\d+) s$"),
    "Payload": re.compile(r"^Payload:\s*(\d+) bytes$"),
    "TX Packets": re.compile(r"^TX Packets:\s*(\d+)$"),
    "RX Packets": re.compile(r"^RX Packets:\s*(\d+)$"),
    "Lost Packets": re.compile(r"^Lost Packets:\s*(\d+)$"),
    "RX Bytes": re.compile(r"^RX Bytes:\s*(\d+)$"),
    "Throughput": re.compile(r"^Throughput:\s*([0-9.eE+-]+) kbps$"),
    "Average Delay": re.compile(r"^Average Delay:\s*([0-9.eE+-]+) ms$"),
    "Average Jitter": re.compile(r"^Average Jitter:\s*([0-9.eE+-]+) ms$"),
    "Simulation CPU Time": re.compile(r"^Simulation CPU Time:\s*([0-9.eE+-]+) s$"),
    "Messages Sent": re.compile(r"^Messages Sent:\s*(\d+)$"),
    "Messages Received": re.compile(r"^Messages Received:\s*(\d+)$"),
    "Messages Lost": re.compile(r"^Messages Lost:\s*(\d+)$"),
    "Average Application Delay": re.compile(r"^Average Application Delay:\s*([0-9.eE+-]+) ms$"),
    "Security Overhead Bytes": re.compile(r"^Security Overhead Bytes:\s*(\d+) bytes$"),
    "Security Handshake Messages": re.compile(r"^Security Handshake Messages:\s*(\d+)$"),
}

CSV_FIELDS = [
    "Protocol",
    "Security",
    "Nodes",
    "DurationSeconds",
    "PayloadBytes",
    "TXPackets",
    "RXPackets",
    "LostPackets",
    "RXBytes",
    "ThroughputKbps",
    "AverageDelayMs",
    "AverageJitterMs",
    "SimulationCpuTimeS",
    "MessagesSent",
    "MessagesReceived",
    "MessagesLost",
    "AverageAppDelayMs",
    "SecurityOverheadBytes",
    "SecurityHandshakeMessages",
    "Seed",
]


def locate_executable() -> Path:
    if EXECUTABLE.exists() and EXECUTABLE.is_file():
        return EXECUTABLE
    raise FileNotFoundError(
        f"Executable not found at expected path: {EXECUTABLE}\n"
        "Build the project first and rerun the script."
    )


def parse_output(output: str) -> dict:
    result = {}
    for line in output.splitlines():
        stripped = line.strip()
        for key, pattern in METRIC_PATTERNS.items():
            match = pattern.match(stripped)
            if not match:
                continue
            if key == "Protocol":
                result["Protocol"] = match.group(1)
            elif key == "Security":
                result["Security"] = match.group(1)
            elif key == "Nodes":
                result["Nodes"] = int(match.group(1))
            elif key == "Duration":
                result["DurationSeconds"] = int(match.group(1))
            elif key == "Payload":
                result["PayloadBytes"] = int(match.group(1))
            elif key == "TX Packets":
                result["TXPackets"] = int(match.group(1))
            elif key == "RX Packets":
                result["RXPackets"] = int(match.group(1))
            elif key == "Lost Packets":
                result["LostPackets"] = int(match.group(1))
            elif key == "RX Bytes":
                result["RXBytes"] = int(match.group(1))
            elif key == "Throughput":
                result["ThroughputKbps"] = float(match.group(1))
            elif key == "Average Delay":
                result["AverageDelayMs"] = float(match.group(1))
            elif key == "Average Jitter":
                result["AverageJitterMs"] = float(match.group(1))
            elif key == "Simulation CPU Time":
                result["SimulationCpuTimeS"] = float(match.group(1))
            elif key == "Messages Sent":
                result["MessagesSent"] = int(match.group(1))
            elif key == "Messages Received":
                result["MessagesReceived"] = int(match.group(1))
            elif key == "Messages Lost":
                result["MessagesLost"] = int(match.group(1))
            elif key == "Average Application Delay":
                result["AverageAppDelayMs"] = float(match.group(1))
            elif key == "Security Overhead Bytes":
                result["SecurityOverheadBytes"] = int(match.group(1))
            elif key == "Security Handshake Messages":
                result["SecurityHandshakeMessages"] = int(match.group(1))
            elif key == "Transmission Cost":
                result["TransmissionCost"] = float(match.group(1))
            elif key == "Protocol Cost":
                result["ProtocolCost"] = float(match.group(1))
            elif key == "Security Cost":
                result["SecurityCost"] = float(match.group(1))
            elif key == "Total CPU Cost":
                result["TotalCpuCost"] = float(match.group(1))
    return result


def run_experiment(executable: Path, protocol: str, security: str, nodes: int, seed: int) -> dict:
    command = [
        str(executable),
        f"--protocol={protocol}",
        f"--security={security}",
        f"--nodes={nodes}",
        f"--seed={seed}",
    ]
    completed = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=False,
    )

    if completed.returncode != 0:
        raise RuntimeError(
            f"Experiment failed: protocol={protocol}, security={security}, nodes={nodes}\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}\n"
        )

    result = parse_output(completed.stdout)
    result["Protocol"] = protocol
    result["Security"] = security
    result["Nodes"] = nodes
    result["Seed"] = seed
    return result


def main() -> None:
    executable = locate_executable()
    experiments = [
        (protocol, security, nodes)
        for protocol in PROTOCOLS
        for security in SECURITIES
        for nodes in NODE_COUNTS
    ]
    random.shuffle(experiments)

    output_file = OUTPUT_CSV
    with output_file.open("w", newline="", encoding="utf-8") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=CSV_FIELDS)
        writer.writeheader()

        for index, (protocol, security, nodes) in enumerate(experiments, start=1):
            seed = index
            print(f"Running {index}/{len(experiments)}: protocol={protocol}, security={security}, nodes={nodes}, seed={seed}")
            result = run_experiment(executable, protocol, security, nodes, seed)
            row = {field: result.get(field, "") for field in CSV_FIELDS}
            writer.writerow(row)
            print(f"  -> done, wrote row for {protocol}/{security}/{nodes}\n")

    print(f"All experiments complete. Results written to: {output_file}")


if __name__ == "__main__":
    main()
