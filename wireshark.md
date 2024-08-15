WiFi monitor mode with Wireshark
================================

The following page covers all the steps needed to switch your WiFi interface into [monitor mode](https://en.wikipedia.org/wiki/Monitor_mode) and start capturing packets with `wireshark`.

Monitor mode support
--------------------

Before you do anything, you need to find your WiFi interface and confirm that it does support monitor mode.

Now, find the WiFi interface:

```
$ ip link show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: enp2s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
    link/ether 04:0e:3c:3b:63:f6 brd ff:ff:ff:ff:ff:ff
8: wlo1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
    link/ether b0:7d:64:56:0b:4a brd ff:ff:ff:ff:ff:ff
    altname wlp0s20f3
```

In this case `wlo1` is the WiFi interface.

If you're not sure which one if the WiFi interface, you can check with `iw`:

```
$ sudo apt install iw
$ iw dev
phy#0
	Unnamed/non-netdev interface
		wdev 0x9
		addr b0:7d:64:56:0b:49
		type P2P-device
	Interface wlo1
		ifindex 10
		wdev 0x8
		addr b0:7d:64:56:0b:4a
        ...
```

It's output shows `Interface wlo1`.


Now, check that it supports monitor mode:

```
$ iw list | less
```

And search for the bit that shows `monitor` in the _support-modes_ list:

```
Supported interface modes:
         * IBSS
         * managed
         * AP
         * AP/VLAN
         * monitor
```

If it does show `monitor`, you can carry on...

Setup
-----

Install `wireshark`:

```
$ sudo apt install wireshark-qt
```

It asks you if you want to enable `wireshark` capture for non-root users - but points out there are security implications to this and defaults to _No_ (which means you'd always have to run `wireshark` with `sudo` which is almost certainly worse security-wise).

And it's not even as if it's _any_ non-root user - it's only users that are added to the `wireshark` group.

So select _Yes_ for enabling `wireshark` capture for non-root users. And add yourself to the `wireshark` group (created by the `wireshark` install process):

```
$ sudo usermod -a -G wireshark $USER
```

You can use `newgrp` to start a subshell in which you're now in this group but it's less confusing to just log out completely (really) and log back in to pick up this group membership properly.

Then install `aircrack-ng` so you can enable monitor mode.

```
$ sudo apt install aircrack-ng
```

Then you run `aircrack-ng` to see if there are any process that'll interfere with using monitor mode (by initiating scans or switching channel):

```
$ sudo airmon-ng check

Found 1 processes that could cause trouble.
Kill them using 'airmon-ng check kill' before putting
the card in monitor mode, they will interfere by changing channels
and sometimes putting the interface back in managed mode

    PID Name
    785 avahi-daemon
    790 NetworkManager
    828 wpa_supplicant
    858 avahi-daemon
```

If you try killing some of those they'll just restart, so...

```
$ sudo systemctl stop NetworkManager
$ sudo systemctl disable avahi-daemon
$ sudo systemctl stop avahi-daemon
$ sudo systemctl stop wpa_supplicant
```

If you don't disable `avahi-daemon` then another service - `avahi-daemon.socket` - will just immediately triggers its restart.

If so, you can start monitor mode (be aware that doing so will stop all normal WiFi activity so, ideally, you also have a wired network connection):

```
$ sudo airmon-ng start wlo1
```

If you check the interfaces again, you'll find the original WiFi interface has been replaced with one with the same name with the `mon` suffix added:

```
$ ip link show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: enp2s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
    link/ether 04:0e:3c:3b:63:f6 brd ff:ff:ff:ff:ff:ff
9: wlo1mon: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/ieee802.11/radiotap b0:7d:64:56:0b:49 brd ff:ff:ff:ff:ff:ff
```

Double-check that you're in the `wireshark` group (either from having used `newgrp` or from having logged in and out):

```
$ groups
wireshark ... joebloggs
```

And start `wireshark`:

```
$ wireshark
```

And, on the start screen, double click the WiFi monitor interace, in this case `wlo1mon`.

`wireshark` will only monitor traffic on whatever channel the device is currently tuned. You can see this with:

```
$ iw wlo1mon info
Interface wlo1mon
	ifindex 9
	wdev 0x7
	addr b0:7d:64:56:0b:49
	type monitor
	wiphy 0
	channel 2 (2417 MHz), width: 20 MHz (no HT), center1: 2417 MHz
```

Above it's tuned to channel 2.

You can switch the channel that's being monitored, while `wireshark` is running, like so:

```
$ sudo iw dev wlo1mon set channel 3
```

`wireshark` will immediately start capturing packets from this new channel. To confirm this, select the most recently captured packet in `wireshark` and in the middle pane, expand the `802.11 radio information` section and you'll see `Channel: 3` there (some channels will have little or no traffic so make sure at least one packet has been captured since you changed channel).

That's it.

Stop (and start)
----------------

Now everything is set up, it's quick to stop monitor mode and restore the services that you stopped:

```
$ sudo airmon-ng stop wlo1mon
$ sudo systemctl start wpa_supplicant
$ sudo systemctl enable avahi-daemon
$ sudo systemctl start NetworkManager
```

Note: you don't have to start `avahi-daemon` as `avahi-daemon.socket` will do this for you.

And it's quick to restart monitor mode and `wireshark`:

```
$ sudo systemctl stop NetworkManager
$ sudo systemctl disable avahi-daemon
$ sudo systemctl stop avahi-daemon
$ sudo systemctl stop wpa_supplicant
$ sudo airmon-ng start wlo1mon
$ wireshark
```

And switch channel:

```
$ sudo iw dev wlo1mon set channel 3
```

Capture by mac address
----------------------

If you know the mac address of the device you're interested in, e.g. an ESP32 board, then you can capture all traffic to and from it by specifying a filter like this in `wireshark`:

```
wlan.sa == 40:F5:20:25:2c:86 || wlan.ra == 40:F5:20:25:2c:86
```
