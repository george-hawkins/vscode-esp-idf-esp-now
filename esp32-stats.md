ESP32 statistics
================

` idf.py size` from same image as startup sequence below.

```
Used stat D/IRAM:  112612 bytes ( 208684 remain, 35.0% used)
      .data size:   11616 bytes
      .bss  size:   14672 bytes
      .text size:   86324 bytes
Used Flash size :  653698 bytes
           .text:  525546 bytes
         .rodata:  127896 bytes
Total image size:  751638 bytes (.bin may be padded larger)
```

```
    I  (15) boot: ESP-IDF v5.2.2 2nd stage bootloader
    I  (15) boot: compile time Sep  7 2024 22:52:14
    I  (16) boot: chip revision: v0.4
    I  (18) boot.esp32c3: SPI Speed      : 80MHz
    I  (23) boot.esp32c3: SPI Mode       : DIO
*   I  (28) boot.esp32c3: SPI Flash Size : 2MB
    I  (33) boot: Enabling RNG early entropy source...
    I  (38) boot: Partition Table:
    I  (42) boot: ## Label            Usage          Type ST Offset   Length
    I  (49) boot:  0 nvs              WiFi data        01 02 00009000 00006000
    I  (56) boot:  1 phy_init         RF data          01 01 0000f000 00001000
    I  (64) boot:  2 factory          factory app      00 00 00010000 00100000
    I  (71) boot: End of partition table
*   I  (75) esp_image: segment 0: paddr=00010020 vaddr=3c090020 size=1f498h (128152) map
    I (104) esp_image: segment 1: paddr=0002f4c0 vaddr=3fc95200 size=00b58h (  2904) load
*   I (105) esp_image: segment 2: paddr=00030020 vaddr=42000020 size=804ech (525548) map
    I (198) esp_image: segment 3: paddr=000b0514 vaddr=3fc95d58 size=02208h (  8712) load
    I (200) esp_image: segment 4: paddr=000b2724 vaddr=40380000 size=15134h ( 86324) load
    I (227) boot: Loaded app from partition at offset 0x10000
    I (227) boot: Disabling RNG early entropy source...
    I (438) cpu_start: Unicore app
    I (447) cpu_start: Pro cpu start user code
*   I (447) cpu_start: cpu freq: 160000000 Hz
    I (447) cpu_start: Application information:
*   I (450) cpu_start: Project name:     experiments
*   I (455) cpu_start: App version:      f2d7ec1
*   I (460) cpu_start: Compile time:     Sep  7 2024 22:52:08
    I (466) cpu_start: ELF file SHA256:  9faead865...
*   I (471) cpu_start: ESP-IDF:          v5.2.2
    I (476) cpu_start: Min chip rev:     v0.3
    I (481) cpu_start: Max chip rev:     v1.99 
*   I (486) cpu_start: Chip rev:         v0.4
    I (491) heap_init: Initializing. RAM available for dynamic allocation:
*   I (498) heap_init: At 3FC9B8B0 len 00024750 (145 KiB): RAM
*   I (504) heap_init: At 3FCC0000 len 0001C710 (113 KiB): Retention RAM
*   I (511) heap_init: At 3FCDC710 len 00002950 (10 KiB): Retention RAM
*   I (518) heap_init: At 50000010 len 00001FD8 (7 KiB): RTCRAM
    I (525) spi_flash: detected chip: generic
    I (529) spi_flash: flash io: dio
*   W (533) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
    I (546) sleep: Configure to isolate all GPIO pins in sleep state
    I (553) sleep: Enable automatic switching of GPIO sleep configuration
    I (560) main_task: Started on CPU0
*   I (560) main_task: Calling app_main()
    I (560) experiment: esp_task_wdt_status: 0x0000
    I (570) pp: pp rom version: 9387209
    I (570) net80211: net80211 rom version: 9387209
*   I (590) wifi:wifi driver task: 3fca3858, prio:23, stack:6656, core=0
    I (590) wifi:wifi firmware version: 3e0076f
    I (590) wifi:wifi certification version: v7.0
    I (590) wifi:config NVS flash: disabled
    I (600) wifi:config nano formating: disabled
    I (600) wifi:Init data frame dynamic rx buffer num: 32
    I (600) wifi:Init static rx mgmt buffer num: 5
    I (610) wifi:Init management short buffer num: 32
    I (610) wifi:Init dynamic tx buffer num: 32
    I (620) wifi:Init static tx FG buffer num: 2
    I (620) wifi:Init static rx buffer size: 1600
    I (620) wifi:Init static rx buffer num: 10
    I (630) wifi:Init dynamic rx buffer num: 32
    I (630) wifi_init: rx ba win: 6
    I (640) wifi_init: tcpip mbox: 32
    I (640) wifi_init: udp mbox: 6
    I (640) wifi_init: tcp mbox: 6
    I (650) wifi_init: tcp tx win: 5760
    I (650) wifi_init: tcp rx win: 5760
    I (660) wifi_init: tcp mss: 1440
    I (660) wifi_init: WiFi IRAM OP enabled
    I (660) wifi_init: WiFi RX IRAM OP enabled
    I (670) wifi:Set ps type: 0, coexist: 0
    I (670) wifi:ifx:0, phymode(new:0x4, nvs:0x3)
    I (680) phy_init: phy_version 1170,f4aea9b,Apr 30 2024,10:49:24
*   I (720) wifi:mode : sta (64:e8:33:88:a6:20)
    I (720) wifi:enable tsf
```
