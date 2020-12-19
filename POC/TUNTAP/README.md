# Prove of concept (capture all user traffic)
This **prove of concept** demonstrates 1 of the ways to collect all user packet by a program. There maybe some better way to do this as well >v<

## steps to run, such that all traffic will go via tap0
```sh
# run this for the first time
sudo ip tuntap add tap0 mode tap
# start the program
sudo ./tuntapTrial
# turn on tap0
sudo ip link set tap0 up
# set ip for the tap0
sudo ip addr add 10.10.1.2/24 dev tap0
# set arp for tap0
sudo arp -s 10.10.1.1 0a:0b:0c:0d:0e:0f -i tap0
# set default route via tap0
sudo ip route add default via 10.10.1.1 dev tap0
```
