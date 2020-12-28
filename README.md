# benker-vpn
the expected outcome of this project is to build **vpn client** AND **vpn server** using **C**

---

## Progress
still learning concepts

---

## Purpose of this project
1. learning & practise networking concept
2. leanring & practise c programming
---
## purpose of POC folder (prove of concept)
POC is the folder which may contain part of the techniques/building-blocks for the vpn(client & server) implementation.

---

## Questions to ask for the vpn implementation
#### 1. In client side, how to intercept all user traffic to the vpn-client program?
checkout the [POC/TUNTAP](/POC/TUNTAP/README.md)
#### 2. In server side, how to send the received packet to real world
currently decided to use *SOCK_RAW* with domain AF_PACKET, protocol ETH_P_IP (0x0800)
##TODO (more details will be added)
##TODO
#### 3. In server side, how to receive packet send from real world to user
currently decided to use *SOCK_RAW* with domain AF_PACKET, protocol ETH_P_IP (0x0800)
need to ensure the firewall filter table *NOT REJECTING* the SYN packet (just drop them will be fine)
##TODO (more details will be added)
