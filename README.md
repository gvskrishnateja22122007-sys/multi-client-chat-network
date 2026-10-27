# Multi-Client Chat Network

A multi-client chat network implemented using C socket programming on Ubuntu.

## Project Overview

This project implements a client-server chat system where multiple clients can connect to a central server and communicate with each other over a network.

## Technologies

- C
- TCP/IP Socket Programming
- Linux/Ubuntu
- GCC
- Make
- Git/GitHub

## Project Structure

- `server/` - Server-side implementation
- `client/` - Client-side implementation
- `include/` - Shared header files
- `tests/` - Testing programs
- `docs/` - Project documentation

## Team

- G V S Krishna Teja
- Team Member 2
- Team Member 3

## Status

Project under development.
## SDN Integration

The project integrates Software-Defined Networking (SDN) using Mininet, Open vSwitch (OVS), OpenFlow 1.3, and the OS-Ken controller.

The virtual network contains three hosts connected to one OpenFlow switch:

| Host | Role | IP Address |
|---|---|---|
| h1 | Chat server | 10.0.0.1 |
| h2 | Chat client | 10.0.0.2 |
| h3 | Chat client | 10.0.0.3 |

The C chat server listens on TCP port 8080.

## Requirements

- Ubuntu Linux
- GCC and Make
- Mininet
- Open vSwitch
- Python 3
- OS-Ken SDN controller
- XTerm for opening Mininet host terminals

## Build the Application

From the project directory, compile the server and client:

```bash
make
```

## Run the SDN Controller

Open a terminal and run:

```bash
cd ~/multi-client-chat-network
osken-manager sdn/controller.py
```

Leave the controller running.

## Start the Mininet Network

Open a second terminal and run:

```bash
cd ~/multi-client-chat-network
sudo mn -c
sudo mn --topo single,3 --mac --switch ovsk,protocols=OpenFlow13 --controller remote,ip=127.0.0.1,port=6653
```

At the Mininet prompt, test connectivity:

```text
pingall
```

Inspect the OpenFlow rules:

```text
sh ovs-ofctl -O OpenFlow13 dump-flows s1
```

## Run the Chat Application

At the Mininet prompt, open the host terminals:

```text
xterm h1 h2 h3
```

In h1, start the server:

```bash
cd /home/khanishri_129/multi-client-chat-network
./server/server
```

In h2, start a client:

```bash
cd /home/khanishri_129/multi-client-chat-network
./client/client 10.0.0.1
```

In h3, start another client:

```bash
cd /home/khanishri_129/multi-client-chat-network
./client/client 10.0.0.1
```

Messages sent by one client should be broadcast to the other connected clients.

Type `/quit` in a client to disconnect.

## SDN Traffic-Blocking Policy

The controller contains the setting:

```python
BLOCK_H3_CHAT = False
```

When enabled:

```python
BLOCK_H3_CHAT = True
```

the controller installs a high-priority OpenFlow rule to drop traffic from h3 (10.0.0.3) to h1 (10.0.0.1) on TCP destination port 8080.

After changing this setting, restart the controller and recreate the Mininet network so the rule is installed when the switch connects.

Inspect the flow table:

```text
sh ovs-ofctl -O OpenFlow13 dump-flows s1
```

Verify the matching rule and packet counter before concluding that the policy is working.

## Performance Testing

### Network Latency

The following command measures ICMP round-trip time from h2 to h1:

```text
h2 ping -c 20 10.0.0.1
```

An initial observed test recorded:

- Packets transmitted: 20
- Packets received: 20
- Packet loss: 0%
- Average round-trip time: 0.132 ms

These results represent one test run and may vary between runs.

### Chat Message Delivery

A manual test sent ten messages from h2. Messages 1 through 10 were observed at h3.

Application-level message latency was not measured in this test.

### Connection Scalability

The scalability test script is located at:

`tests/scalability_test.py`

The results are saved in:

`docs/scalability-results.csv`

The recorded run reported:

| Target connections | Successful connections | Failed connections |
|---:|---:|---:|
| 2 | 2 | 0 |
| 5 | 5 | 0 |
| 10 | 10 | 0 |

These results measure TCP connection attempts. They do not independently prove simultaneous chat-message delivery or application throughput.

## Results and Limitations

The project demonstrates:

- Multi-client TCP chat communication
- A three-host Mininet virtual network
- OpenFlow 1.3 forwarding
- Controller-managed traffic blocking
- Initial ICMP latency testing
- Basic TCP connection scalability testing

Further evaluation could include repeated performance trials, application-level message latency, and testing with more than ten clients after reviewing the server's configured client limit.
