import socket
import csv
import time
from pathlib import Path

SERVER_IP = "10.0.0.1"
SERVER_PORT = 8080
CLIENT_COUNTS = [2, 5, 10]
TIMEOUT = 3

results = []

for target in CLIENT_COUNTS:
    connections = []
    successes = 0
    failures = 0
    start = time.perf_counter()

    for _ in range(target):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(TIMEOUT)

        try:
            sock.connect((SERVER_IP, SERVER_PORT))
            connections.append(sock)
            successes += 1
        except OSError as exc:
            failures += 1
            print(f"Connection failed: {exc}")
            sock.close()

    elapsed = time.perf_counter() - start

    print(
        f"Target clients: {target}, "
        f"successful: {successes}, failed: {failures}, "
        f"connection-test duration: {elapsed:.4f} seconds"
    )

    results.append({
        "target_clients": target,
        "successful_connections": successes,
        "failed_connections": failures,
        "connection_test_duration_seconds": round(elapsed, 4),
    })

    for sock in connections:
        sock.close()

    time.sleep(1)

output = Path("docs/scalability-results.csv")
output.parent.mkdir(parents=True, exist_ok=True)

with output.open("w", newline="") as file:
    writer = csv.DictWriter(file, fieldnames=results[0].keys())
    writer.writeheader()
    writer.writerows(results)

print(f"\nResults saved to {output}")
