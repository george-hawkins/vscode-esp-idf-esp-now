Visual Studio Code with the ESP-IDF extension
============================================

First, if you want to set your Visual Studio Code back to a pristine state:

```
$ mkdir old-vscode-dots
$ mv ~/.vscode ~/.config/Code ~/.cache/vscode-* old-vscode-dots
```

---

You can install the ESP-IDF manually (as described [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#manual-installation)) but they recommend installing it via your IDE.

I thought this might result in some isolated IDE specific setup but the VS Code ESP-IDF extension just does the same steps that you would have had to do manually.

Everything still ends up in `~/esp` and `~/.espressif` as it would have with the manual approach. The only difference is that nothing gets added to your `~/.bashrc` and you have to use the _ESP-IDF Terminal_ icon (in the bottom bar) in VS Code to start a shell that's set up with the right environment to use tools like `idf.py` etc.

---

So guided by Espressif's [VS Code ESP-IDF extension installation guide](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md), I did the following...

Installed the [prerequisites](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites):

```
$ sudo apt install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

Downloaded the latest `.deb` from the VS Code [downloads page](https://code.visualstudio.com/Download), then...

```
$ cd ~/Downloads
$ sudo apt install ./code_1.89.1-1715060508_amd64.deb
```

VS Code was then available via the [Super key](https://help.ubuntu.com/stable/ubuntu-help/keyboard-key-super.html).

The `python3` and `python3-distutils` packages were already installed and the `/usr/bin/pytho3` version was 3.10.12 so, no additional Python related steps were required.

I started VS Code and:

* Clicked the _Extensions_ icon (bottom icon in group of four icons at the top of the left-hand bar).
* Searched for "esp-idf", selected the one from Espressif Systems and installed it.
* Exited and restarted VS Code.
* On restart, I clicked the Espressif icon (now, just below _Extensions_). Various Espressif specific icons then appeared in the bar along the bottom (a plug-like icon, letting you select a port, etc.). And _ESP-IDF Setup_ tab appeared.

I also installed the Vim extension as I'm a Vim person.

I select _Express_ in the _ESP-IDF Setup_ tab and:

* Left download server as _GitHub_.
* Selected the latest ESP-IDF version (v5.2.1 at the time of writing).
* Left the tools and container directory as they were.
* Actively selected `/usr/bin/python` as the Python version (as it autodetected an alternative version I'd installed using `pyenv`).

It then took some minutes to download and install everything (installing the virtual environment was the step where it stayed silent longest while doing things).

That's it - done.

**Update:** it looks like selecting `/usr/bin/python` might have been the wrong thing to do, I think instead I should have made sure the `pyenv` shim was pointing to a suitable recent version of Python and let the setup process use the shim (which was the behavior it defaulted to).

You can't really change things later (as `/usr/bin/python` become baked into the resulting `~/.espressif/python_env`).

This may all be the reason why I have to explicitly activate the ESP-IDF environment (see later) even when working via the _ESP-IDF Terminal_ in VS Code (which should normally set up the environment correctly).

Note: it's this [post](https://www.esp32.com/viewtopic.php?t=38394#p127763) from an ESP employee that makes me think this.

---

In the _ESP-IDF Setup_ tab, you can now click _Show examples_ (for whatever reason, you're then forced to select _Use current ESP-IDF_ as the framework to use).

I followed the instructions in the extension's [basic-usage guide](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/basic_use.md).

I selected _blink_ and then _Create project using example blink_.

It rather abruptly pops up a file select dialog, you should select a parent folder where you want the project to be created. The extension will then, in this case, create a subdirectory there called `blink`.

For whatever reason, it also suggested I install the [_Dev Containers_ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers). I'm a big fan of Docker but it's not obvious to me where this would come into play here - maybe it's more relevant in some setup where all the tooling isn't available natively (e.g. maybe `riscv32-esp-elf-gdb`  is handled via containers on Windows).

I didn't install the _Dev Containers_ extension.

**Update:** I installed it eventually simply to stop it nagging me.

I did install the suggested [C/C++ extension pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) (I suspect it would have been better to have had this istalled before creating the blink project so that the CMake extension automatically configured things for themseleves at project creation time).

Once the project was created, I went to _Explorer_ (upper-left icon) and opened the `blink_example_main.c` file in the `main` folder.

---

The basic-usage guide describes setting the board and using the configuration menu via VS Code's command palette. But it can also be done using the little icons in the botom bar.

I pressed the _Espressif Device Target_ icon (which had defaulted to _esp32_) and selected _esp32c3_, you then has to choose from three options: _ESP-PROG_, _USB-JTAG_, _ESP USB Bridge_. _ESP-PROG_ and _ESP USB Bridge_ involve using additional boards and for most dev boards, you want the basic builtin USB connector - this is the _USB-JTAG_ option.

When I set the target, the _Output_ tab (tabs in lower half of window) logged the error:

```
[6/1/2024, 1:29:12 PM] "${workspaceFolder}/build/compile_commands.json" could not be parsed. 'includePath' from c_cpp_properties.json in folder 'blink' will be used instead.
```

I looked at both files:

```
$ cd .../blink
$ vim -O .vscode/c_cpp_properties.json build/compile_commands.json
```

Both looked fine and from a JSON point of view both were parseable (confirmed with `jq`).

**Update:** I think this issue is that you have to do at least one build (see below) before this file is properly updated to reflect the current state of things. This seems to be confirmed in the ESP-IDF extension's documentation for the [`c_cpp_properties.json` file](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/C_CPP_CONFIGURATION.md):

> For the default configuration, you must build your project beforehand in order to generate `${workspaceFolder}/build/compile_commands.json` (where `${workspaceFolder}` is your project directory). 

---

Click the _SDK Configuration Editor_ icon in the lower bar.

I found using the _SDK Configuration Editor_ via VS Code a litte scary as it automatically expands all sections of the configuration and you're overwhelmed with the internals, that you should leave as they are, when you just want to see the project specific settings.

So, just type "example" into the search field to narrow everything down to the tiny set of values that we actually want to set and set:

* Blink LED type: GPIO
* Blink GPIO number: 8
* Blink period in ms: 1000

My board has a plain old LED (i.e. not a neopixel) on pin 8, your board may be different.

Once the values are set, press _Save_.

If you go to the _Explorer_ and open the file `Kconfig.projbuild`, you'll see that the name of the section corresponds the the `menu` value in this file, i.e. "Example Configuration" and that the options are defined here.

While looking at `Kconfig.projbuild`, you can also look at `sdkconfig` - when you pressed _Save_ in _SDK Configuration Editor_, this is where the values ended up.

And when the next build is done, those values will become normal C `#define` lines in `build/config/sdkconfig.h`, which your example code includes and then uses as you'd expect (selecting the approriate LED pin etc.).

---

You can also start the SDK editor in the _ESP-IDF terminal_, click it's icon in the lower bar and...

```
$ idf.py menuconfig
Cannot import module "click". This usually means that "idf.py" was not spawned within an ESP-IDF shell environment or the python virtual environment used by "idf.py" is corrupted.
Please use idf.py only in an ESP-IDF shell environment. If problem persists, please try to install ESP-IDF tools again as described in the Get Started guide.
```

This is the problem I referred to above when setting the Python path as part of the ESP-IDF extension initial setup.

The problem is solved easily enough:

```
$ source ~/.espressif/python_env/idf5.2_py3.10_env/bin/activate
```

Now `(idf5.2_py3.10_env)` appeared before the prompt to indicate that the venv was properly activated. Now ...

```
$ idf.py menuconfig
```

This opens a terminal-based interface to the SDK Configuration and here all the sections are collapsed by default and you can just go straight to "Example Configuration", expand it and set the necessary values.

---

Now, press the _Build_ icon. Note: there are two _Build_ icons in the bottom bar, one that looks like a tin can (which is also somewhat like the traditional symbol for a DB) and another one that looks like a cog wheel and has _Build_ written beside it. I used the first clearly ESP-IDF related one, I'm not sure what the second one is for.

If interested, open the command paletter and search for "ESP-IDF: Size Analysis of the Binaries" and see how big your binary is relative to your device's available memory. If you click the _Detailed_ button, you can also see a breakdown of what's taking up all the space.

---

Plug in your device, it's port isn't automatically selected but if you click the _Select Port_ icon in the bottom bar, it suggests the USB device that's just been plugged in.

Once the port is selected, click the _Flash_ icon - I thought it was doing nothing until I notiched the dropdown that had opened at the top of the screen and had to select the flashing method.

I tried _JTAG_ first and let it start OpenOCD but this failed. So, I clicked the _Flash Method_ icon (star in bottom bar) and changed it to _UART_, this time flashing worked.

Finally, I pressed the _ESP-IDF Monitor_ icon and could watch the output of my program.

That was it

OpenOCD
-------

When I tried using the _JTAG_ flashing method and let the ESP-IDF extension try and start OpenOCD, it failed with:

```
Error: libusb_open() failed with LIBUSB_ERROR_ACCESS
Error: esp_usb_jtag: could not find or open device!
```

You can run the ESP-IDF OpenOCD from the command line like so:

```
$ cd ~/.espressif/tools/openocd-esp32/v0.12.0-esp32-20230921/openocd-esp32
$ ./bin/openocd -f share/openocd/scripts/board/esp32c3-builtin.cfg 
```

If you look at the environment variables set up for the _ESP-IDF Terminal_ in VS Code, you'll see you can also specify `OPENOCD_SCRIPTS` and do:

```
$ export OPENOCD_SCRIPTS=$HOME/.espressif/tools/openocd-esp32/v0.12.0-esp32-20230921/openocd-esp32/share/openocd/scripts
$ ./bin/openocd -f board/esp32c3-builtin.cfg 
```

It's just a permissioning problem, OpenOCD wants to open the USB device and not just the assocate tty device.

So, after plugging in and out my device and looking at `/var/log/syslog` to see:

```
Jun  1 15:25:53 joebloggs-machine kernel: [96952.231212] usb 1-7.2: New USB device found, idVendor=303a, idProduct=1001, bcdDevice= 1.01
Jun  1 15:25:53 joebloggs-machine kernel: [96952.231227] usb 1-7.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
Jun  1 15:25:53 joebloggs-machine kernel: [96952.231233] usb 1-7.2: Product: USB JTAG/serial debug unit
Jun  1 15:25:53 joebloggs-machine kernel: [96952.231237] usb 1-7.2: Manufacturer: Espressif
```

I added the following line to `/etc/udev/rules.d/50-serial-ports.rules`:

```
ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", MODE="0660", TAG+="uaccess"
```

I.e. with `idVendor` and `idProduct` matching the values seen in `syslog`.

Now, on starting OpenOCD, everything worked fine:

```
$ ./bin/openocd -f board/esp32c3-builtin.cfg 
Open On-Chip Debugger v0.12.0-esp32-20230921 (2023-09-21-13:41)
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
Info : only one transport option; autoselecting 'jtag'
Info : esp_usb_jtag: VID set to 0x303a and PID to 0x1001
Info : esp_usb_jtag: capabilities descriptor set to 0x2000
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : esp_usb_jtag: serial (64:E8:33:88:A6:20)
Info : esp_usb_jtag: Device found. Base speed 40000KHz, div range 1 to 255
Info : clock speed 40000 kHz
Info : JTAG tap: esp32c3.cpu tap/device found: 0x00005c25 (mfg: 0x612 (Espressif Systems), part: 0x0005, ver: 0x0)
Info : [esp32c3] datacount=2 progbufsize=16
Info : [esp32c3] Examined RISC-V core; found 1 harts
Info : [esp32c3]  XLEN=32, misa=0x40101104
Info : starting gdb server for esp32c3 on 3333
Info : Listening on port 3333 for gdb connections
```

I killed it and switched the flash method from _UART_ to _JTAG_ and flashing worked.

This meant, I could now set breakpoints in `blink_example_main.c`, select _Run and Debug_ and just click the _Start Debugging_ icon shown to the left of the dropdown showing "Launch". Everything worked as expected - super!

TODO
----

* Wire up two SuperMini boards so that I can connect a UART-to-USB converter to both and use those for serial input/output while also monitoring the program via USB.
* Then get the ESP-NOW component's [get started](https://github.com/espressif/esp-now/tree/master/examples/get-started) example going on both boards (create the project via the IDE method used above to create a new project from an example or from the command line as described [here](https://github.com/espressif/esp-now/blob/master/README.md#example)).
* I don't think the ESP-NOW component's [user guide](https://github.com/espressif/esp-now/blob/master/User_Guide.md) contains anything very useful - it just details the usual build steps etc. and gives some very high level overview of the main API and the sibling APIs like _Provision_.
* Look into `WIFI_PROTOCOL_LR` as seen in the non-component based ESP-NOW [`espnow_example_main.c`](https://github.com/espressif/esp-idf/blob/master/examples/wifi/espnow/main/espnow_example_main.c). Will ESP-NOW automatically use `WIFI_PROTOCOL_LR` even if all the other protocols are also enabled (as is done in the example)?

Lot's of tutorials and posts (e.g. this [one](https://www.esp32.com/viewtopic.php?t=12772)) say it's necessary to call `esp_wifi_set_ps(WIFI_PS_NONE)` but I believe this is only relevant if you're using Wi-Fi and ESP-NOW at the same time.

Finding the clearest part of the spectrum
-----------------------------------------

For the ideal channel, I think you want to be able to look at the unmodulated RF noise floor and choose the area of the spectrum showing the least "energy".

Annoyingly, it looks like the ESP32 _may_ be able to do this to some degree but doesn't expose this ability.

The original ESP32 specifications (from back in 2015) say, in the Wi-Fi section:

> Adaptive rate fallback algorithm sets the optimal transmission rate and transmit power based on actual Signal Noise Ratio (SNR) and packet loss information.

But this _may_ just mean the usual ability to scan for APs and find the one offering the highest signal strength.

And the [current ESP32 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf) says in the class bluetooth section that it supports:

> Adaptive Frequency Hopping and Channel assessment.

The Bluetooth standards website says:

> One of the techniques Bluetooth technology uses to overcome interference and find a clear transmission path that avoids packet collision is the application of a form of frequency-hopping spread spectrum (FHSS) called adaptive frequency hopping (AFH). Bluetooth divides the frequency band into smaller channels (e.g. 40 channels in the case of Bluetooth Low Energy) and rapidly hops between those channels when transmitting packets. To further reduce the chance of interference, Bluetooth adapts its hopping sequence. Channels that are noisy and busy are dynamically tracked and avoided when sending packets.

But maybe for this or similar Wi-Fi abilities to work you actually have to be transmitting (and noticing the noise simply as a product of packets getting dropped).

There are intriguing mentions of noise in the ESP-IDF:

```
components/esp_rom/esp32c3/ld/esp32c3.rom.eco7.ld:rom1_set_noise_floor = 0x400019e8;
components/esp_rom/esp32c3/ld/esp32c3.rom.ld:rom_check_noise_floor = 0x40001920;
components/esp_rom/esp32c3/ld/esp32c3.rom.ld:rom_noise_floor_auto_set = 0x4000197c;
components/esp_rom/esp32c3/ld/esp32c3.rom.ld:rom_set_noise_floor = 0x400019e8;
...
components/wpa_supplicant/src/common/bss.c:	dst->noise = src->noise;
components/wpa_supplicant/src/common/bss.h:	/** Noise level */
components/wpa_supplicant/src/common/bss.h:	int noise;
components/wpa_supplicant/src/common/scan.h: * Noise floor values to use when we have signal strength
components/wpa_supplicant/src/common/scan.h: * measurements, but no noise floor measurements. These values were
components/wpa_supplicant/src/common/scan.h:#define DEFAULT_NOISE_FLOOR_2GHZ (-89)
components/wpa_supplicant/src/common/wpa_ctrl.h:#define WPA_BSS_MASK_NOISE		BIT(6)
components/wpa_supplicant/src/drivers/driver.h: * @noise: noise level
components/wpa_supplicant/src/drivers/driver.h: * @snr: Signal-to-noise ratio in dB (calculated during scan result processing)
components/wpa_supplicant/src/drivers/driver.h:	int noise;
components/wpa_supplicant/esp_supplicant/src/esp_scan.c:    res->noise = 0;
components/bt/esp_ble_mesh/models/common/include/mesh/device_property.h: * | Present Ambient Noise                                    | 0x0079 | Noise                                     |  1   |
components/bt/esp_ble_mesh/models/common/include/mesh/device_property.h:#define BLE_MESH_PRESENT_AMBIENT_NOISE                                      0x0079
components/bt/esp_ble_mesh/models/common/include/mesh/device_property.h:#define BLE_MESH_PRESENT_AMBIENT_NOISE_LEN                                      1
components/bt/esp_ble_mesh/models/common/device_property.c:    { BLE_MESH_PRESENT_AMBIENT_NOISE,     
...
components/esp_wifi/include/local/esp_wifi_types_native.h:    signed noise_floor: 8;        /**< noise floor of Radio Frequency Module(RF). unit: dBm*/
components/esp_wifi/include/esp_wifi_he_types.h:    unsigned noise_floor: 8;                      /**< the noise floor of the reception frame */
```

And a few mentions of SNR:

```
components/esp_rom/esp32c3/ld/esp32c3.rom.ld:rcUpdateAckSnr = 0x40001774;
...
components/esp_wifi/include/esp_private/esp_wifi_he_private.h:float esp_test_get_bfr_avgsnr(void);
```

To really look at the spectrum you need a spectrum analyzer and these don't come cheap.

There have been no end of modules that rely on a laptop to provide the UI rather than the classical all-in-one spectrum analyzer. E.g. the Wi-Spy.

But there doesn't seem to be anything really convincing that's still available in the sub-US$100 range.

Andreas Spiess reviews a common module on AliExpress - the LTDZ 35-4400M - [here](https://www.youtube.com/watch?v=PRsaGEk-EsQ) but in the end he recommends an all-in-one unit with its own screen, the tinySA Ultra (he recommends it as a "much better product").

And it came up again and again in my searches. The tinySA site is [here](https://tinysa.org/wiki/pmwiki.php) and they link to this AliExpress [product page](https://www.aliexpress.com/item/1005004934403303.html) as selling genuine tinySA Ultras - for around US$120 plus an addtional US$15 for shipping.

Spiess's video is good introduction to how to use such devices.
