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
$ source $IDF_PYTHON_ENV_PATH/bin/activate
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

This meant, I could now set breakpoints in `blink_example_main.c`, select _Run and Debug_ and just click the _ESP-IDF: Debug_ button in the bottom bar. Everything worked as expected - super!

When not using the ESP-IDF plugin, the normal way to debug is to use the _Run and Debug_ button (left-hand gutter).

Using the _ESP-IDF: Debug_ button actually just sets up the classic _Run and Debug_ for you - it flips the _Start Debugging_ dropdown to _Eclipse CDT GDB Adapter_ and creates a suitable `launch.json` file.

If you haven't used _ESP-IDF: Debug_ and click the normal _Run and Debug_ button, things won't have been set up and instead, you see a link to "create a launch.json file", click it (it just creates the same _Eclipse CDT GDB Adapter_ `launch.json` as the other approach) and then click the _Start Debugging_ button.

You can have the monitor open at the same time as debugging - though I found this often caused OpenOCD to crash, things seem to work best if one opens the monitor before trying to debug things (which probably makes sense as the monitor usually resets the board).

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

### Coin cell example

The [coin cell example](https://github.com/espressif/esp-now/tree/master/examples/coin_cell_demo) demos binding devices.

#### Coin cell powered board

The README shows a custom coin-cell powered ESP32C2 board but they also describe how to get it going with a normal dev board (and in fact the steps involved are simpler as the coin cell board requires a separate programmer).

So, just ignore the _coin cell button_ specific instructions in the [_How to Use the Example_](https://github.com/espressif/esp-now/tree/master/examples/coin_cell_demo#how-to-use-the-example) section and remember that the `menuconfig` item `EXAMPLE_USE_COIN_CELL_BUTTON` to `N` as covered below.

Note: the button shown in the example's README is described in details in a [paywalled article](https://www.elektormagazine.com/magazine/elektor-328/62434) from Elektor (I have the [print version](https://www.elektor.com/products/elektor-special-espressif-guest-edition-2023-en) and it goes into a lot of technical detail about low-power ESP design). The button doesn't appear to be for sale but a design, _apparently_ from Espressif, is available [here](https://oshwhub.com/esp-college/32bfc63ed181441a9a44da6cd2419809) on OSHWHub (Chinese only - there's a corresponding English language site called [OSHWLab](https://oshwlab.com/) but designs are not shared across the two). Note that one of the comments at the bottom of the OSHWHub page claims that the design is incomplete and the schematic shown for version 1.0 doesn't match the one shown in the example's README (and the dates etc. don't match up), the version 1.1 schematic on the OSHWHub is only viewable if you have a login for the Chinese language version of EasyEDA (creating such an account requires WeChat or registering using your phone number).

**Update:** there are far more details of the coin-cell board in the `switch/docs` subdirectory, see [`button_in_matter_bridging.md`](https://github.com/espressif/esp-now/blob/master/examples/coin_cell_demo/switch/docs/button_in_matter_bridging.md) along with the schematic and PCB (unfortunately, only in PDF form) and the BOM [here](https://github.com/espressif/esp-now/tree/master/examples/coin_cell_demo/switch/docs).

#### Running the example on a standard dev board

As above, go to the _ESP-IDF: Explorer_ in VS Code, select _Show Examples_ (and select _Use current_ from the center-top dropdown), then click the _ESP Component Registry_ link, search for "esp-now", click the _espressif/esp-now_ link, click the _Examples_ tab, then the _coin_cell_demo/switch_ link.

The current version of the ESP-IDF plugin seems to get confused by this situation where there are two parts to the example (_switch_ and _bulb_) in the _coin_cell_demo_ directory. At the end of creating the project it says "The path .../coin_cell_demo/switch does not exist on this computer." But actually all is fine, it's created the project and all you have to do is go to _File_ / _Open Folder..._ and open the `switch` folder.

The center-top dropdown suggests you should select a kit (gcc or clangd based) for the project but just press escape (as selecting the target will select a board specific kit).

**Update:** every time I open a ESP-IDF project, I'm asked to select a kit - it's the MS [CMake Tools extentension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) that's doing this. According to this GitHub [issue comment](https://github.com/espressif/vscode-esp-idf-extension/issues/664#issuecomment-1062809722), the ESP-IDF extension does not depend on CMake Tools extentension. So, you can disable it on a per-project basis (expand _Extensions_, right click on _CMake Tools_ and select _Disable (Workspace)_) or completely (right click and select _Disable_).

Note: I have two CMake extensions - _CMake Tools_, which was installed as part of the Microsoft _C/C++ Extension Pack_, and _CMake_ which seems to have been abandoned (the last release was in 2017). I'm not sure what installed the second of these two but it's probably best avoided on a clean install.

Then set the device target to esp32c3 (or whatever's appropriate for your board), right click on the `sdkconfig` file and select _SDK Configuration Editor_ and search for "EXAMPLE_USE_COIN_CELL_BUTTON" and untick "Use coin cell button".

Click the build button, the project pulls in the [_espressif/button_ component](https://components.espressif.com/components/espressif/button/) (that provides functions for detecting double clicks and the such like) and this component results in some deprecation warnings (about `ADC_ATTEN_DB_11`) but it seems fine to ignore these.

Before taking a look at the code remember to open the command palette (ctrl-shift-P) and enter "ESP-IDF: Add vscode Configuration Folder" (as covered above). Then go to `main/app_main.c` and you'll see the file is basically split in two and which half gets compiled depends on whether `CONFIG_EXAMPLE_USE_COIN_CELL_BUTTON` is defined or not.

---

Now open the _bulb_ half of the example and go through the same steps, including running the "ESP-IDF: Add vscode Configuration Folder" task but don't flash the code to your _second_ board yet.

The code assumes you've got a dev board with a neopixel - if your board had a normal LED you'll have to modify a few things first.

Assuming you've got a simple ESP32C3 dev board with a normal onboard LED connected to pin 8 then:

**1.** Repmoved the block that defines `g_strip_handle`:

```
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
// ESP32C2-DevKit Board uses RGB LED
#if !CONFIG_IDF_TARGET_ESP32C2
static led_strip_handle_t g_strip_handle = NULL;
#endif
#else
static led_strip_t *g_strip_handle = NULL;
#endif
```

**2.** Replace the contents of function `app_led_init` with:

```
gpio_reset_pin(LED_STRIP_GPIO);
gpio_set_direction(LED_STRIP_GPIO, GPIO_MODE_OUTPUT);
```

**3.** Replace the contents of function `app_led_set_color` with:

```
uint32_t level = red == 0 && green == 0 && blue == 0 ? 0 : 1;

gpio_set_level(LED_STRIP_GPIO, level);
```

That's it. The code assumes it can indicate different states with different colors:

* Normal bult operation (off or white).
* Binding (green).
* Unbinding (red).

With a normal LED, there's just two states on or off - so bulb on, binding and unbinding all just turn the LED on, you can't distinguish between the states. One could try using different LED brightnesses to indicate bind and unbind but I haven't tried that.

Now, build and flash the code to the board.

---

**Note:** working with two ESP32 boards and OpenOCD doesn't seem to work very well - even if you change the selected port (port icon in bottom bar), OpenOCD seems to remain tied to the first device it programmed. Only having one device plugged in at a time while programming (and frequent use of `pkill -9 openocd`) seemed to be the only soliution.


```
$ source ~/esp/v5.2.2/esp-idf/export.sh
$ idf.py monitor --port /dev/esp-usb-serial1
```

Note that `monitor` tries to be a bit intelligent and needs to be in the root directory of the the project that's been uploaded to the board so that it can find its `CMakeLists.txt`. If you start `monitor` in the directory of a different project, it'll notice this as the device boots and print:

```
Warning: checksum mismatch between flashed and built applications. Checksum of built application is 9099c68bc9f4ac6dd986d5efdc315874974dacb418a6b19fdcdce8d8c099c8ee
```

The boards seemed to behave differently at different times to programming - sometimes they reset after flashing and immediately started running the flashed program, other times the RESET button had to be pressed. I thought initially this was some difference between the different Super Mini boards I have but it's something other than that.

---

After plugging in with BOOT button held down:

```
Aug 12 13:31:28 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.421458] usb 1-7.1: new full-speed USB device number 7 using xhci_hcd
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.523123] usb 1-7.1: New USB device found, idVendor=303a, idProduct=1001, bcdDevice= 1.01
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.523137] usb 1-7.1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.523142] usb 1-7.1: Product: USB JTAG/serial debug unit
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.523146] usb 1-7.1: Manufacturer: Espressif
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.523149] usb 1-7.1: SerialNumber: 64:E8:33:88:A6:20
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.571412] cdc_acm 1-7.1:1.0: ttyACM0: USB ACM device
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.571436] usbcore: registered new interface driver cdc_acm
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1491.571437] cdc_acm: USB Abstract Control Model driver for USB modems and ISDN adapters
Aug 12 13:31:29 ghawkins-OMEN-25L-Desktop-GT12-0xxx snapd[819]: hotplug.go:200: hotplug device add event ignored, enable experimental.hotplug
```

Flashing:

```
esptool.py --port /dev/esp-usb-serial0 --chip esp32c3 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 2MB 0x0 bootloader/bootloader.bin 0x10000 switch.bin 0x8000 partition_table/partition-table.bin
esptool.py v4.7.0
Serial port /dev/esp-usb-serial0
Connecting...
Chip is ESP32-C3 (QFN32) (revision v0.4)
Features: WiFi, BLE, Embedded Flash 4MB (XMC)
Crystal is 40MHz
MAC: 64:e8:33:88:a6:20
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 460800
Changed.
Configuring flash size...
Flash will be erased from 0x00000000 to 0x00003fff...
Flash will be erased from 0x00010000 to 0x000affff...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 14624 bytes to 10474...
Wrote 14624 bytes (10474 compressed) at 0x00000000 in 0.2 seconds (effective 523.3 kbit/s)...
Hash of data verified.
Compressed 652912 bytes to 398856...
Wrote 652912 bytes (398856 compressed) at 0x00010000 in 4.8 seconds (effective 1094.8 kbit/s)...
Hash of data verified.
Compressed 3072 bytes to 103...
Wrote 3072 bytes (103 compressed) at 0x00008000 in 0.0 seconds (effective 573.6 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

Despite saying "Hard resetting" above, the board did not reset itself (there was no further output to `/var/log/syslog`) and I had to press the board's RESET button. The following was then logged to `/var/log/syslog`:

```
Aug 12 13:32:47 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.045147] usb 1-7.1: USB disconnect, device number 7
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.526646] usb 1-7.1: new full-speed USB device number 8 using xhci_hcd
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.610460] usb 1-7.1: device descriptor read/64, error -32
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.819072] usb 1-7.1: device descriptor read/all, error -32
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.898456] usb 1-7.1: new full-speed USB device number 9 using xhci_hcd
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1570.978547] usb 1-7.1: device descriptor read/64, error -32
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1571.166481] usb 1-7.1: device descriptor read/64, error -32
Aug 12 13:32:48 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1571.275243] usb 1-7-port1: attempt power cycle
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1571.882425] usb 1-7.1: new full-speed USB device number 10 using xhci_hcd
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1571.903093] usb 1-7.1: device descriptor read/8, error -32
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.031385] usb 1-7.1: device descriptor read/8, error -32
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.218380] usb 1-7.1: new full-speed USB device number 11 using xhci_hcd
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.240414] usb 1-7.1: New USB device found, idVendor=303a, idProduct=1001, bcdDevice= 1.01
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.240427] usb 1-7.1: New USB device strings: Mfr=1, Product=2, SerialNumber=3
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.240432] usb 1-7.1: Product: USB JTAG/serial debug unit
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.240436] usb 1-7.1: Manufacturer: Espressif
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.240440] usb 1-7.1: SerialNumber: 64:E8:33:88:A6:20
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx kernel: [ 1572.254999] usb 1-7.1: can't set config #1, error -32
Aug 12 13:32:49 ghawkins-OMEN-25L-Desktop-GT12-0xxx fwupd[2917]: 11:32:49:0768 FuEngine             failed to probe device usb:01:00:07:01: USB error on device 303a:1001 : Entity not found [-5]
```

---

#### Coin cell redo

I couldn't get the switch part of the example to work with my ESP32C3 boards, after flashing and pressing RESET, I always got the issue shown above where the device was not recognised as a serial device and various errors were logged to `/var/log/syslog`.

So, I decided to recreate the project from scratch, creating the `switch` and `bulb` parts of the example with the ESP-IDF command line tools and copying over the main `.c` code from the original project but nothing else (i.e. none of the supporting structure, e.g. `sdkconfig` etc.).

Here are the steps I went thru...

I set up the `esp-idf` command line environment, created a completely new project, set the target to `esp32c3`, looked up the latest `espressif/esp-now` and `espressif/button` in the [ESP Component Registry](https://components.espressif.com/) and added them as dependencies:

```
$ source ~/esp/v5.2.2/esp-idf/export.sh
$ mkdir coin_cell_clean
$ cd coin_cell_clean
$ idf.py create-project switch
$ cd switch
$ idf.py set-target esp32c3
$ idf.py add-dependency 'espressif/esp-now^2.5.1'
$ idf.py add-dependency 'espressif/button^3.3.1'
```

I downloaded the original `switch` code and removed all reference to the coin-cell board, i.e. the code will only work with a standard dev board:

```
$ cd main
$ curl -o switch.c https://raw.githubusercontent.com/espressif/esp-now/master/examples/coin_cell_demo/switch/main/app_main.c
$ vim switch.c
$ cd ..
```

You can see the changes I made to the original `app_main.c` [here](https://github.com/george-hawkins/vscode-esp-idf-esp-now/commit/2179409b22176f227cf5d6a8395eb3c82c114b1d). I also removed two other minor `#ifdef` usages to make things as simple as possible.

Note: when downloading `app_main.c` with curl, I renamed it to `switch.c`. This is simply because the `idf.py create-project` step above had create a `switch.c` file containing a skeleton for the `app_main` function and I just decided to stick with its choice of file names.

Then I did a `reconfigure` to download the dependencies (the `add-dependency` step above just adds entries to `main/idf_component.yml` but doesn't actually download the code for the components):

```
$ idf.py reconfigure
```

Then I held down the boards BOOT button, pressed RESET and then released the BOOT button to make sure it was ready to be flashed.

Then I built and flashed the code:

```
$ idf.py build
$ cd build
$ esptool.py --port /dev/esp-usb-serial0 --chip esp32c3 -b 460800 --before default_reset --after hard_reset write_flash "@flash_args"
```

Make sure to change the `port` value (`/dev/esp-usb-serial0` above) to whatever's appropriate for your setup.

After flashing, I pressed the board's RESET button and this time it showed up as desired as a serial device.

---

I then did a similar set of steps for the `bulb` part of the example even though it had uploaded fine using the original setup. I just wanted to have the two parts really in sync with each other in terms of what components etc. they were using:

```
$ cd .../coin_cell_clean
$ idf.py create-project bulb
$ cd bulb
$ idf.py set-target esp32c3
$ idf.py add-dependency 'espressif/esp-now^2.5.1'
```

My dev board has a normal LED connected to pin 8, if your board has neopixel then you also need to add the `espressif/led_strip` dependency:

```
$ idf.py add-dependency 'espressif/led_strip^2.5.4'
```

Then download the original `app_main.c`:

```
$ cd main
$ curl -o bulb.c https://raw.githubusercontent.com/espressif/esp-now/master/examples/coin_cell_demo/bulb/main/app_main.c
$ vim bulb.c
$ cd ..
```

At the time of writing, the `espressif/esp-now` version was `2.5.1` but the `bulb` code on `master` (that was retrieved using `curl`) had been updated to use the at-the-time unreleased `ESP_EVENT_ESPNOW_CTRL_BIND_ERROR` to diagnose binding failures. So, I had to remove the `bind_error_to_string` function and the `ESP_EVENT_ESPNOW_CTRL_BIND_ERROR` `case` lower down in the `app_espnow_event_handler` function.

Other than this change, it isn't necessary to change anything else _if_ your board has a neopixel. If your board has a normal LED, the changes covered above need to be made, you can see a `diff` of the change I made [here](https://github.com/george-hawkins/vscode-esp-idf-esp-now/commit/6112f28fe8545c88823e5664cb87634453d1a931).

Then the final build and flashing steps:

```
$ idf.py reconfigure
$ idf.py build
$ cd build
$ esptool.py --port /dev/esp-usb-serial0 --chip esp32c3 -b 460800 --before default_reset --after hard_reset write_flash "@flash_args"
```

Make sure you're keeping track of which device is connected to which port and that you're not simply e.g. overwriting the `switch` code on one board with the `bulb` code (I did this more than once).

Once both boards are ready run `monitor` for both (at this stage, I'd decided to do everything on the command line and use ESP-IDF tools for everything).

In one terminal (remember you need to be in the right folder so `monitor` can find the right `CMakeLists.txt` file), connect to the switch:

```
$ cd .../switch
$ idf.py monitor --port /dev/esp-usb-serial0
...
--- esp-idf-monitor 1.4.0 on /dev/esp-usb-serial0 115200 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
...
I (236) cpu_start: cpu freq: 160000000 Hz
I (236) cpu_start: Application information:
I (239) cpu_start: Project name:     switch
I (244) cpu_start: App version:      6112f28-dirty
I (250) cpu_start: Compile time:     Aug 12 2024 16:04:35
I (256) cpu_start: ELF file SHA256:  b1cd45a21...
I (261) cpu_start: ESP-IDF:          v5.2.2
...
I (510) espnow: esp-now Version: 2.5.1
W (520) wifi:Haven't to connect to a suitable AP now!
I (520) ESPNOW: espnow [version: 1.0] init
I (520) espnow: mac: 64:e8:33:88:a6:20, version: 2
I (530) espnow: Enable main task
I (530) espnow: main task entry
I (540) app_switch: started
I (540) main_task: Returned from app_main()
```

**TODO:** what on earth does the warning `wifi:Haven't to connect to a suitable AP now!` mean?

**Answer:** who knows what this "unusual" piece of English is trying to communicate but it's output as a result of calling `espnow_init` - `espnow_init` wants to check it the device was connected to an AP prior to `espnow_init` being called, so it calls `esp_wifi_sta_get_ap_info` to see if it returns `ESP_OK` (which would indicate that the device is connected to an AP). Unfortunately, in the case where the device is not connected to an AP, the `esp_wifi_sta_get_ap_info` not only returns `ESP_ERR_WIFI_NOT_CONNECT` but also always prints this warning. The ESP-IDF team have been asked if they'd remove the warning and leave it up to the caller to decide whether to print something or not but they've said they're keeping the warning (see this [comment](https://github.com/espressif/esp-idf/issues/12219#issuecomment-1976342932)).

**Note:** `cpu freq` defaults to 160 MHz - in some places, I've seen it claimed that the ESP32C3 can run up to 240 MHz but if I open the _SDK Configuration Editor_ and search for `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ`, the maximum shown frequency is 160 MHz.

Then in another terminal terminal connect to the bulb:

```
$ idf.py monitor --port /dev/esp-usb-serial1
...
I (521) espnow: esp-now Version: 2.5.1
W (521) wifi:Haven't to connect to a suitable AP now!
I (531) ESPNOW: espnow [version: 1.0] init
I (531) espnow: mac: 40:4c:ca:fa:f6:a8, version: 2
I (541) espnow: Enable main task
I (541) espnow: main task entry
I (541) main_task: Returned from app_main()
I (881) restart_func: num: 32, reason: 3, crash: 0
```

The follow the instructions from the example's [README](https://github.com/espressif/esp-now/blob/master/examples/coin_cell_demo/README.md#how-to-use-the-example):

* Double click the `BOOT` button on the device, it will send the binding command.
* Single click the `BOOT` button on the device, it will send the control command.
* Long press (more than 1.5 seconds) the `BOOT` button on the device, it will send the unbinding command.

I found the switch would consistently send the bind command on double pressing the boot button and output:

```
I (941770) app_switch: switch bind press
```

But the bulb would often show no sign of having seen the bind command, retrying or resetting the board would resolve things (I suspect resetting was never really necessary and just retrying should always eventually work). If everything went well then the buld board would output:

```
I (23534) espnow_ctrl: bind, esp_log_timestamp: 23534, timestamp: 1800554, rssi: -51, rssi: -55
I (23534) app_bulb: bind, uuid: 64:e8:33:88:a6:20, initiator_type: 513
```

Despite ESP-NOW being a connection-oriented protocol (e.g. it knows to retransmit if the other side fails to receive a packet), neither side seemed very aware of the state of the other. E.g. the switch sends bind on double press and a switch update on single press but doesn't seem to maintain an awareness of what state its in or if the bind succeeded (I presume this is what the upcoming `ESP_EVENT_ESPNOW_CTRL_BIND_ERROR` changes, mentioned above, are about), e.g. it doesn't say something like "can't send switch status when in unbound state".

Similarly, the bulb often seemed to miss unbind events. If everything worked properly, the switch would output:

```
I (1965190) app_switch: switch unbind press
```

And the bulb would output:

```
I (990717) app_bulb: unbind, uuid: 64:e8:33:88:a6:20, initiator_type: 513
```

Once bound, single presses on the switch's _BOOT_ button would cause it to output:

```
I (2231260) app_switch: switch send press
I (2231270) app_switch: key status: 1
```

And the bulb would turn on (or off) its onboard LED and output:

```
I (1256717) app_bulb: app_bulb_ctrl_data_cb, initiator_attribute: 513, responder_attribute: 1, value: 1
```

Again the bulb would sometimes miss these messages.

A neopixel or a richer set of LED behaviors (e.g. breathing when unbound or the other things that are possible with the `LEDC` mode of the [`espressif/led_indicator`](https://components.espressif.com/components/espressif/led_indicator) component, see example below) would make it clearer what's happening.

Note: as with `screen` and `minicom`, I found that exiting `monitor` (with `ctrl-[`) could sometimes take quite a few seconds. And at other times would be near instantaneous - I've no idea why disconnecting took longer at some times than others.

##### Update

The original example set various `sdkconfig` items like `CONFIG_ESPNOW_CONTROL_RETRANSMISSION_TIMES` that aren't set in my `sdkconfig` files which are just in a completely default state.

See the differences [here](examples/coin_cell_clean/sdkconfig-diffs).

It turns out most of these are all set as a result of including the following line in the `sdkconfig.defaults` file of each part (`switch` and `bulb`):

```
CONFIG_ESPNOW_CONTROL_AUTO_CHANNEL_SENDING=y
```

This causes various things, like `CONFIG_ESPNOW_CONTROL_RETRANSMISSION_TIMES`, to be turned on (seach for `AUTO_CHANNEL_SENDING` in [`esp-now:Kconfig`](https://github.com/espressif/esp-now/blob/master/Kconfig)) and enables quite different behavior in [`esp-now:src/control/src/espnow_ctrl.c`](https://github.com/espressif/esp-now/blob/master/src/control/src/espnow_ctrl.c).

The remaining differences are a result of the original code also setting `CONFIG_ESPNOW_LIGHT_SLEEP=y` which is only relevant if using the coin-cell button rather than a normal dev board (as it needs to sleep for a certain amount of time before sending in order to fully charge the capicitor that it needs to have enough power to handle the transmit step).

So, for `switch` and `bulb`, I did:

```
$ echo CONFIG_ESPNOW_CONTROL_AUTO_CHANNEL_SENDING=y > sdkconfig.defaults
$ idf.py set-target esp32c3
```

The `set-target` is required to force it to rebuild the full `sdkconfig` file.

And then:

```
$ idf.py build
$ cd build
$ esptool.py --port /dev/esp-usb-serial0 --chip esp32c3 -b 460800 --before default_reset --after hard_reset write_flash "@flash_args"
```

As before make sure to update `port` values so, you really do update the particular board you really want to update.

This didn't seem to greatly improve how often packets got lost but at least the switch side seemed to be aware of the transmit failures and output:

```
W (184040) espnow_ctrl: [espnow_ctrl_initiator_handle, 429] <ESP_FAIL> espnow_broadcast, ret: -1
```

If I pressed the double-clicked to bind repeatedly the bulb would sometimes output:

```
I (70672) espnow_ctrl: bind, esp_log_timestamp: 70672, timestamp: 1800542, rssi: -55, rssi: -55
```

And sometimes output two lines:

```
I (106842) espnow_ctrl: bind, esp_log_timestamp: 106842, timestamp: 1800542, rssi: -43, rssi: -55
I (106842) app_bulb: bind, uuid: 64:e8:33:88:a6:20, initiator_type: 513
```

This would make sense if e.g. `espnow_ctrl` was only forwarding the event to the application layer if the device wasn't already bound but this didn't seem to be it, double clicking the switch while already bound seemingly randomly resulted in the one or two line output from the bulb.

**Theory:**

With a little bit more experimentation, I found there was far less packet loss if:

* The board was lifted clear of its surroundings, i.e. not just lying amoung bits and pieces on my desk.
* I kept my fingers well clear of the antenna - this sounds obvious but when using a board with a tiny onboard ceramic antenna it's easy to block the antenna while pressing the BOOT button.
* Using a board where board designer has got [impedance matching](https://en.wikipedia.org/wiki/Impedance_matching) right - if the board is a no-name board from AliExpress and gets very hot during use then this is probably not the case.

All these items are fairly obvious but easy to forget about.

##### Notes on opening newly created project in VS Code

Open in VS Code and remember to run the "ESP-IDF: Add vscode Configuration Folder" task.

When I tried to start OpenOCD, it failed with the following warning and errors:

```
WARNING: boards/esp32-wrover.cfg is deprecated, and may be removed in a future release.
         If your board is ESP32-WROVER-KIT, use board/esp32-wrover-kit-1.8v.cfg instead.

Info : auto-selecting first available session transport "jtag". To override use 'transport select <transport>'.

Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections

x Error: unable to open ftdi device with description '*', serial '*' at bus location '*'

x /home/ghawkins/.espressif/tools/openocd-esp32/v0.12.0-esp32-20240318/openocd-esp32/share/openocd/scripts/target/esp_common.cfg:9: Error: 
at file "/home/ghawkins/.espressif/tools/openocd-esp32/v0.12.0-esp32-20240318/openocd-esp32/share/openocd/scripts/target/esp_common.cfg", line 9
```

Even though the plugin had correctly picked up the chip type as `esp32c3` (and displayed it in the bottom bar), it seems OpenOCD still thought the device was a generic classic ESP32 WROVER kit.

I had to click the _EDP-IDF: Set Espressif Device Target_ icon and select `esp32c3` (it wasn't necessary to switch to another device and switch back, selecting the already selected device was enough). But this meant if cleared out the current build and I had to rebuild before flashing.

### LED example

You can find the _espressif/led_indicator_ component in the ESP-IDF component registry.

I downloaded one of the examples like so:

```
$ idf.py create-project-from-example 'espressif/led_indicator^0.9.3:indicator/ledc'
```

This example uses [LEDC](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html), if plain on and off blinking (without any fading in and out) is enough, see the GPIO example.

Oddly, the LEDC example depends on both `led_indicator` (expected) and `cmd_led_indicator` (unexpected). You can find `cmd_led_indicator` in the Espressif `esp-iot-solution` repo [here](https://github.com/espressif/esp-iot-solution/tree/master/examples/indicator/components/cmd_led_indicator) but its not available via the ESP Component Registry.

So, an initial attempt to set the chip type fails due to this.

I resolved this issue (you can see the changes [here](https://github.com/george-hawkins/vscode-esp-idf-esp-now/commit/158b951158b151850fec38f691d1a684e15a811d)) and I also:

* Set `GPIO_LED_PIN` to pin 8 to match my board.
* Set `GPIO_ACTIVE_LEVEL` to false to indicate that the LED on my board is on when its GPIO pin is LOW (you'd use true for HIGH).
* Hardcoded the LEDC channel to `LEDC_CHANNEL_0` (as the timer is already harcoded, I don't know why the channel, in contrast, was configurable).

I made these changes in `main.c` rather than bothering with `sdkconfig`.

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
