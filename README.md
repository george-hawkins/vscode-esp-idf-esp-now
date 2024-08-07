Visual Studio Code with the ESP-IDF extension
============================================

First, if you want to set your Visual Studio Code back to a pristine state:

```
$ mkdir old-vscode-dots
$ mv ~/.vscode ~/.config/Code ~/.cache/vscode-* old-vscode-dots
```

---

You can install the ESP-IDF manually (as described [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#manual-installation)) but they recommend installing it via your IDE.

I thought this might result in some isolated IDE specific setup but the VS Code ESP-IDF extension just does the same steps that you would have had to do manually.

Everything still ends up in `~/esp` and `~/.espressif` as it would have with the manual approach. The only difference is that nothing gets added to your `~/.bashrc` and you have to use the _ESP-IDF Terminal_ icon (in the bottom bar) in VS Code to start a shell that's set up with the right environment to use tools like `idf.py` etc.

See the _Upgrade_ section below for how to upgrade to a newer ESP-IDF version (VS Code and the extension keep themselves automatically up-to-date) - this section also covers how simple it would have been to do things manually rather than leaving it to VS Code.

---

So guided by Espressif's [VS Code ESP-IDF extension installation guide](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md), I did the following...

Installed the [prerequisites](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites):

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

Note: I use `pyenv` and it automatically detected `~/.pyenv/shims/python3` as the Python executable. This is fine if you're happy with whatever Python version is being pointed to - it'll be resolved to a concrete path later (i.e. the virtual env will bake in the thing that the shim redirects to).

It then took some minutes to download and install everything (installing the virtual environment was the step where it stayed silent longest while doing things).

That's it - done.

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

The issue is that you have to do at least one build (see below) before this file is properly updated to reflect the current state of things. This is covered in the ESP-IDF extension's documentation for the [`c_cpp_properties.json` file](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/C_CPP_CONFIGURATION.md):

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

The problem is solved easily enough:

```
$ source $IDF_PYTHON_ENV_PATH/activate
```

Now `(idf5.2_py3.10_env)` appeared before the prompt to indicate that the venv was properly activated. I'd kind of expect that pressing _ESP-IDF terminal_ would open a terminal where the venv was already activated. But this is how it behaves on my Linux system.

Now ...

```
$ idf.py menuconfig
```

This opens a terminal-based interface to the SDK Configuration and here all the sections are collapsed by default and you can just go straight to "Example Configuration", expand it and set the necessary values.

---

Now, press the _Build_ icon. Note: there are two _Build_ icons in the bottom bar, one that looks like a tin can (which is also somewhat like the traditional symbol for a DB) and another one that looks like a cog wheel and has _Build_ written beside it. I used the first clearly ESP-IDF related one, I'm not sure what the second one is for.

**Update:** now rather than a tin-can-like icon, it looks like a spanner.

If interested, open the command paletter and search for "ESP-IDF: Size Analysis of the Binaries" and see how big your binary is relative to your device's available memory. If you click the _Detailed_ button, you can also see a breakdown of what's taking up all the space.

---

Plug in your device, it's port isn't automatically selected but if you click the _Select Port_ icon in the bottom bar, it suggests the USB device that's just been plugged in.

Once the port is selected, click the _Flash_ icon - I thought it was doing nothing until I notiched the dropdown that had opened at the top of the screen and had to select the flashing method.

I tried _JTAG_ first and let it start OpenOCD but this failed. So, I clicked the _Flash Method_ icon (star in bottom bar) and changed it to _UART_, this time flashing worked.

**Update:** see below for how to get OpenOCD working - it's worth it.

Finally, I pressed the _ESP-IDF Monitor_ icon and could watch the output of my program.

That was it

### Jumping to #include files

Somewhat oddly, you can't jump to include files like `freertos/FreeRTOS.h` and lines like the following appear highlighted with a squiggly red line beneath them:

```
#include "freertos/FreeRTOS.h"
```

This is odd as the build process can clearly find these files.

The solution is simple. Once, you've done at least one build such that the `${workspaceFolder}/build/compile_commands.json` file is created (see above), open the command palette (ctrl-shift-P) and enter "ESP-IDF: Add vscode Configuration Folder".

In your project directory, there's a `.vscode` directory and this command adds `c_cpp_properties.json` there and this tells VS Code (or more precisely, the C/C++ extension) where to find things.

Note: this simple step is described in a [very unclear page](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/C_CPP_CONFIGURATION.md) in the ESP-IDF extension documentation - all the detail hides the fact that there's just one thing to do (run the "Add vscode Configuration Folder" command).

This step also adds a `tasks.json` file that allows you to access things like flashing via tasks, e.g. select the menu item _Go_ / _Go to File_ and enter "tasks " and then you'll see e.g. "Flash - flash this device". This just replicates the buttons in the bottom bar but may be useful if you prefer to access things via tasks.

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

Upgrade - automatic and manual
------------------------------

First time through, it's convenient to let the VS Code extension take care of installing the ESP-IDF and associated tool.

But it's not really doing that much and when you later e.g. want to upgrade to a newer version of the ESP-IDF, you may find it better to be in full control of the process.

If VS Code announces that a new version of the ESP-IDF extension is available, you may decide this is also a good time to upgrade your ESP-IDF version and its associated tools. To do this:

* VS Code automatically updates itself and its extensions (so, when it tells you about an update, it's really just asking if you want to update right now or sometime very soon at VS Codes discretion). So, just restart VS Code and check the extensions (cube, with top-left quadrant floating away, if left-hand gutter) and it'll probably already be up-to-date.
* Close all your tabs (in particular ones that are associated with ESP-IDF features) and exist VS Code.
* Remove or rename the current ESP-IDF directory and the associated tools folder:

```
$ cd
$ mv esp esp.bak
$ mv .espressif .espressif.bak
```

* Click the _ESP-IDF: Explorer_ icon in the left-hand gutter and, in the _Commands_ section, single click _Configure ESP-IDF Extension_ (it may seem to do nothing initially but it's downloading details from the internet and the configuration tab will pop up after a few seconds).
* Then select the latest stable version from the _Select ESP-IDF version_ dropdown.
* It'll take a substantial amount of time to install the ESP-IDF version, the associated tools and the Python virtual environment.
* Once done, I restart VS Code, though this doesn't seem to be strictly required.
* Once restarted, exit any terminal sessions in the _Terminal_ tab - slightly unbelieveably, VS Code seems to restore the environment of any session that was open before this whole process started and so its environment variables will point to the old version of things.

**Important:**

After updating to a new version of the ESP-IDF, the CMake files in your projects will still be pointing to the old version and if you try to build, you'll get an error like this:

```
[0/1] Re-running CMake...
CMake Error at build/CMakeFiles/3.22.1/CMakeSystem.cmake:6 (include):
  include could not find requested file:

    /home/joebloggs/esp/v5.2.1/esp-idf/tools/cmake/toolchain-esp32c3.cmake
```

To update the CMake files, just switch to another target (i.e. click the MCU icon in the bottom bar) and then switch back to your original target. On each switch the CMake files are updated. There's probably a way to do this with `idf.py` in one step.

### Manual

So the above process is just doing three steps. You could chose to install things manually from the start, i.e.:

* Install ESP-IDF in `~/esp` as covered in [step 2](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#step-2-get-esp-idf) of the manual installation process.
* Use the installer scripts to install the associated tools and Python virtual envionment into `~/.espressif` as covered in [step 3](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#step-3-set-up-the-tools). Note the option to install `all` rather than e.g. `esp32` (which just installs the toolss for a specific ESP32 chip type) - installing for all chip types is what the VS Code extensions does.

Then later when you want you can upgrade to a newer stable release as described [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/versions.html#updating-esp-idf).

Examples
--------

We got the Blink example working above, now let's try some more things...

### ESP-NOW component

Go to the _ESP-IDF Welcome_ tab, click the _Show examples_ button - at the top of the examples page it says "For external components examples, check _IDF Component Registry_", click the _IDF Component Registry_ and enter "esp-now" in the search field there. _espressif/esp-now_ should be the first result, click that and then go to its _Examples_ tab.

Click on _get-started_ and then click the _Create project from this example_ button. On my Linux machine, the modal dialog that asks you to select a _parent_ directory for the new project opened behind the main VS Code so, giving the impression VS Code had just hung until I found and switched to the dialog.

Or just run:

```
$ idf.py create-project-from-example "espressif/esp-now:get-started"
```

Then I activated the `venv` and configured things:

```
$ source ~/.espressif/python_env/idf5.2_py3.10_env/bin/activate
$ idf.py menuconfig
```

There's actually nothing interesting to configure here (the project's `README` says you can configured the UART that's used but actually that's hardcoded).

Then I pressed the chip icon in the bottom bar to set the _Device Target_ to _esp32c3_ and selected USB-JTAG as the method to interact with it.

I plugged in my board and oddly it wasn't auto-detected so, I had to click the _Port_ icon and select it.

Then I click the _Build_ button (spanner icon in bottom bar) and then clicked the _Flash_ button (initially, the OpenOCD step failed but it was because somehow VS Code had already started an OpenOCD instance, that had taken the 6666 port, and it failed on trying to start a second instance - I quit VS Code, killed the surviving OpenOCD process and, on restarting VS Code, all worked fine).

I flashed the code to a second ESP32C3 board.

Note: to debug, I found that rather than the debug icon in the bottom bar, I had to use the normal _Run and Debug_ button (left-hand gutter), click the link to "create a launch.json file", select ESP-IDF and then I could run the resulting "ESP-IDF Debug: Launch" launch configuration.

I assumed, I'd be able to open the monitor and type stuff and have it broadcast and picked up by the other board. But typing into a serial connection created via USB didn't work (see below for explanation). Instead, I had to connect a UART-to-USB converter (I used this [one](https://www.aliexpress.com/item/1005004399796277.html) from WeAct) to pins 21 (TX) and 20 (RX) of the board.

Then I could open the device for the UART-to-USB converter:

```
$ screen /dev/ch343p-usb-serial 115200
```

And then the device for the other board:

```
$ screen /dev/esp-usb-serial1 115200
```

And type into the `screen` session for the UART-to-USB converter and see this:

```
I (4347754) app_main: espnow_send, count: 60, size: 1, data: f
I (4348024) app_main: espnow_send, count: 61, size: 1, data: o
I (4348214) app_main: espnow_send, count: 62, size: 1, data: o
I (4348544) app_main: espnow_send, count: 63, size: 1, data: b
I (4348694) app_main: espnow_send, count: 64, size: 1, data: a
I (4348924) app_main: espnow_send, count: 65, size: 1, data: r
```

And see it received in the other `screen` session like so:

```
I (4361687) app_main: espnow_recv, <60> [40:4c:ca:fa:f6:a8][1][-46][1]: f
I (4361957) app_main: espnow_recv, <61> [40:4c:ca:fa:f6:a8][1][-38][1]: o
I (4362147) app_main: espnow_recv, <62> [40:4c:ca:fa:f6:a8][1][-46][1]: o
I (4362477) app_main: espnow_recv, <63> [40:4c:ca:fa:f6:a8][1][-46][1]: b
I (4362627) app_main: espnow_recv, <64> [40:4c:ca:fa:f6:a8][1][-46][1]: a
I (4362857) app_main: espnow_recv, <65> [40:4c:ca:fa:f6:a8][1][-46][1]: r
```

If I pasted text into the first session, I could send more than a single character per message.

If I opened a `screen` session corresponding to the USB port of the board that was also connected to the UART-to-USB converter, I could see the same output there as the output echoed in the UART-to-USB converter `screen` session but pressing keys in that session (or in the session corresponding to the second board) didn't result in anything.

**Update:** it turns out that the relationship between the hardware UART0 and the USB serial interface are weaker than I thought. I think it's more a case of things being actively copied from one to the other rather than them being separate views on the same thing. On looking at how the underlying C code in MicroPython works, I found its using the low-level function `usb_serial_jtag_ll_read_rxfifo` (see [`usb_serial_jtag.c:54`](https://github.com/micropython/micropython/blob/f74131134c7af52638348c65ecf1be5e769a5f4b/ports/esp32/usb_serial_jtag.c#L54)). After a little more digging I found that in general, you shouldn't use this function directly but should instead use `usb_serial_jtag_read_bytes`.

I then found that fortunately a example for this functionality had just fairly recently been added to the esp-idf repo in [`examples/peripherals/usb_serial_jtag/usb_serial_jtag_echo`](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb_serial_jtag/usb_serial_jtag_echo).

### usb_serial_jtag_echo

Initially, I didn't get any console output from the `ESP_LOGx` calls. On comparing my `sdkconfig` file with one from a project where such calls worked as expected, I worked out the solution to be...

Right-click on `sdkconfig` in the _Explorer_ panel and select _SDK Configuration Editor_, search for "secondary" and change the _Channel for console secondary output_ from _No secondary console_ to _USB_SERIAL_JTAG_PORT_.

Even after having done this, things didn't work first time, I had to press the RESET button before I got both the `usb_serial_jtag_write_bytes` output and the `ESP_LOGx` output as expected.

For whatever reason, this example comes with a `sdkconfig.defaults` file that contains the single line `CONFIG_ESP_CONSOLE_SECONDARY_NONE=y` and this is the cause of the issue. I guess the idea was maybe that you should see keys echoed via the USB serial port and see the log output via the physical UART pins.

But I saw neither until I made this change - if I used the debugger, I could see that it was reading my key presses successfully with `usb_serial_jtag_read_bytes` but oddly `usb_serial_jtag_write_bytes` didn't seem to write them back, i.e. echo them.

Note: changing `ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG` is mentioned somewhat cryptically in the example's [`README`](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb_serial_jtag/usb_serial_jtag_echo).

Anyway, once done, in addition to the inital firmware output (which showed without issue before this change), you also see log output from the program like:

```
I (477) usb_serial_jtag echo: USB_SERIAL_JTAG init done
```

And then if you press any key, you see:

```
sI (135727) Recv str: : 0x3fc933ac   73                                                |s|
```

Where the first character is the just the character you entered being echoed by `usb_serial_jtag_read_bytes` and the rest of the line is output from the `ESP_LOG_BUFFER_HEXDUMP` call.

**IMPORTANT:**

* `usb_serial_jtag_read_bytes` is only supported by the S3, C3 and later chips (see supported targets listed for the [echo example](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb_serial_jtag/usb_serial_jtag_echo)).
* An alternative is TinyUSB but this supported by an even more limited subset of chips with just the S3 from the commonly used ones (see the supported targets for the [TinyUSB serial device example](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb/device/tusb_serial_device)).
* For the classic ESP32, I think the hardware UART0 is really the same thing as one sees via USB, i.e. boards just route the hardware UART0 out via the UART-to-USB chip and so in this case one can just use `uart_read_bytes` as in the [UART echo example](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/uart/uart_echo) (for which all targets are supported).

This lines up with the `#define`s one sees in the MicroPython and what one sees around the lines of code that put stuff into the `stdin` ring buffer:

```
$ cd .../micropython/ports/esp32
$ fgrep -r stdin | fgrep put
uart.c:            ringbuf_put(&stdin_ringbuf, rbuf[i]);
usb_serial_jtag.c:            ringbuf_put(&stdin_ringbuf, rx_buf[i]);
usb.c:                ringbuf_put(&stdin_ringbuf, usb_rx_buf[i]);
mphalport.c:    if ((poll_flags & MP_STREAM_POLL_RD) && stdin_ringbuf.iget != stdin_ringbuf.iput) {
```

So, that means you either support both reading from the hardware UART0 and using `usb_serial_jtag_read_bytes` or you just support `usb_serial_jtag_read_bytes` and don't support the classic ESP32 or those boards that use the C3 and later but which use a UART-to-USB chip rather than the builtin USB support (for whatever reason, there are various boards that do this).

### UART echo

I opened the UART echo example.

I switch to a [ESP32-S3-DevKitC](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) as this has two USB connectors:

* One labelled USB that goes through to the native S3 USB support.
* The other labelled UART that goes through to a CP2104 UART-to-USB chip that makes UART0 available via USB (and connects to the same pins as those marked as TX and RX on the edge of the dev board which are also labelled as GPIO43 and 44 in the Espressif pinout diagram).

This makes it easier to see what gets printed to which or what gets read from each.

I'm going to call one USB the _UART USB_ and the other the _JTAG USB_.

This application has the same issues that confused me above (the whole `CONFIG_ESP_CONSOLE_SECONDARY_NONE` and `ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG` thing above) but at this stage the behavior seems fairly reasonable. The device startup logging goes to UART0, the user logging goes to the JTAG USB and you can only do normal UART input and output via the UART USB.

I had to open `sdkconfig` and change the TXD and RXD values to 43 and 44 respectively (to match the GPIO values mentioned just above) and it also seemed to be necessary to change the UART to 0.

**IMPORTANT:** a little surprisingly, these values seem to reset if you switch between chip targets, e.g. from S3 to C3.

Then I could watch what output went by connecting to both USB ports on the dev board and running `screen` against both.

`screen` doesn't support the line-return mode used by the ESP32 console so, `minicom` works better:

```
$ sudo apt install minicom
$ minicom --color=on -b 115200 -D /dev/ttyACM0
```

When you exit `minicom` (with `ctrl-A` `X`), it leaves the terminal whatever color it was while using `minicom` - just enter `clear` to get things looking normal.

Another big plus for `minicom` is that, unlike `screen`, it doesn't exit when you reset the dev board and the JTAG USB device temporarily goes away. Instead it reconnects immediately when the device becomes available.

`minicom` assumes hardware flow control - this is fine for the JTAG USB but doesn't work for the UART USD _by default_ (see the example's README, you can set `ECHO_TEST_RTS` and `ECHO_TEST_CTS` if you want). Oddly, you can't turn hardware flow control on and off on via command line arguments. Instead, with `minicom` running, you have to:

* Press `ctrl-A` `O` to `cOnfigure` `minicom`.
* Select the `Serial port setup` menu item.
* Press `F` to toggle the `Hardware Flow Control`.
* Press return to exit out to the parent menu and this time select `Save setup as dfl`.

Now, the change is saved:

```
$ cat ~/.minirc.dfl 
# Machine-generated file - use setup menu in minicom to change parameters.
pu rtscts           No
```

If you wanted you could save this setting to its own file, e.g. `minirc.no-rtscts` (the filename must start with `minirc.`) so, its not a global setting, and then specify it (just the suffix bit) when starting minicom:

```
$ minicom --color=on -b 115200 -D /dev/ttyACM0 no-rtscts
```

It'll also search for files named `.minirc.xyz` in your home directory, so you could store `.minirc.no-rtscts` there and the contained options would be readily available irrespective of your current location in the filesystem.

### Determine if SoC support JTAG USB

If you want code that runs on SoCs that do and don't support `usb_serial_jtag_read_bytes`, you have to use:

```
#if CONFIG_SOC_USB_SERIAL_JTAG_SUPPORTED
    ...
#else
    ...
#endif
```

### PSRAM and UART interupts

In some code, you see the caller of `uart_driver_install` explicitly setting `ESP_INTR_FLAG_IRAM` depending on whether `CONFIG_UART_ISR_IN_IRAM` is set but this is pointless.

The `uart_driver_install` function itself always sets the `ESP_INTR_FLAG_IRAM` appropriately and ignores the passed in state of the flag.

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
