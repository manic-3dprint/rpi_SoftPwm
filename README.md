# Soft PWM for Raspberry PI

This module provides soft PWM capability to raspberry pi. 

# Cross - Compilation

In order to compile the module, you need:

* raspberry pi's kernel sources
* toolchain
* Module.symvers file (in order to save some time)

1. Obtaining the kernel:
	
	$ git clone --depth=1 https://github.com/raspberrypi/linux

The kernel comes without the .config file. The file must be taken from the pi
directly:
	
	$ zcat /proc/config.gz > ~/config
	$ ... transfer the config file to the build machine (scp, ftp ...)

2. Obtaining the toolchain

You can install the toolchain using your system's package manager or by cloing
the raspberry pi's toolchain repository:

	$ git clone --depth=1 https://github.com/raspberrypi/tools


