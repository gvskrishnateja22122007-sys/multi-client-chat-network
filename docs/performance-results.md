# Multi-Client Chat Network — Performance Test Results

**Date:** 27 October 2026  
**Branch:** feature/sdn-integration  
**Environment:** Ubuntu VM, Mininet, Open vSwitch, OS-Ken, C TCP chat application

## 1. Network Latency Test

| Test ID | Source | Destination | Packets Sent | Packets Received | Packet Loss | Min RTT (ms) | Average RTT (ms) | Max RTT (ms) | Result |
|---|---|---|---:|---:|---:|---:|---:|---:|---|
| P-01 | h2 | h1 (10.0.0.1) | 20 | 20 | 0% | 0.084 | 0.132 | 0.420 | PASS |

### Observations

- All 20 ICMP ping packets were received.
- No packet loss was observed in this test.
- Average ICMP round-trip time was 0.132 ms.
- These measurements represent network latency, not application-level chat latency.

## 2. Chat Message Delivery Test

| Test ID | Sender | Receiver | Messages Sent | Messages Observed at Receiver | Observed Delivery Rate | Application Latency |
|---|---|---|---:|---:|---:|---|
| C-01 | h2 | h3 | 10 | 10 | 100% | Not measured |

### Observations

- Ten test messages were sent from h2 through the C chat server.
- The server log showed messages 1 through 10.
- The h3 client displayed messages 1 through 10.
- All ten messages were observed at the receiver during this manual test.
- Application-level response time was not measured.

## 3. Test Limitations

- Results represent a single observed test run.
- The results do not establish performance under heavy load.
- Additional tests are required for 2, 5, and 10 simultaneous clients.
- A 20-client test requires reviewing the server's configured maximum of 10 clients first.
- Repeated runs are needed to assess consistency.
- Application-level latency requires a separate timed measurement.

## 4. Planned Tests

| Test ID | Test | Status |
|---|---|---|
| P-01 | ICMP latency: h2 to h1 | Completed |
| C-01 | Ten-message delivery test: h2 to h3 | Completed |
| S-02 | Two-client connection test | Previously demonstrated |
| S-05 | Five simultaneous clients | Pending |
| S-10 | Ten simultaneous clients | Pending |
| S-20 | Twenty simultaneous clients | Pending; requires capacity review |
| L-01 | Timed application-level message latency | Pending |
| SDN-01 | Controller-enabled blocking policy | Tested; document final repeatable results separately |

## 5. Reproducibility

The ICMP latency test was run from the Mininet CLI using:

    h2 ping -c 20 10.0.0.1

The chat delivery test used the C chat client on h2 and the receiving client on h3, with the C server running on h1 at TCP port 8080.
