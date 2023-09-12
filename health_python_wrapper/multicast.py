"""
This program uses a multicast technique over OSC to transmit
the LAN ip address its host is using.

Author: Vincent Lacasse
Date: 2023-09-12

Required installation:
  $ pip3 install python-osc
"""

from subprocess import check_output
from pythonosc import udp_client

PORT = 7401
IP_MULTICAST = "224.0.0.1"  # this is the general multicast address for a local LAN

# get the LAN ip address of this host

ip_address = check_output(["hostname", "-I"]).split()[0].decode()
print(ip_address)  # to debug

# encode the ip address to a OSC message and transmit it via multicast

client = udp_client.SimpleUDPClient(IP_MULTICAST, PORT)
client.send_message("/address", [ip_address, 10, 2.4])
