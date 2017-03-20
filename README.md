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
	$ copy the config file as linux/.config

2. Obtaining the toolchain

You can install the toolchain using your system's package manager or by cloing
the raspberry pi's toolchain repository:

	$ git clone --depth=1 https://github.com/raspberrypi/tools


3. Preparing the kernel

In order to compile any modules we need Module.symvers file. If the kernel versions
matches then you can obtain Module.symvers from raspberry pi's repositories. Otherwise
you have to devote some time in order to generate it. In order for the file being
generate, all kernel modules must be rebuild:

	$ cd linux
	$ ARCH=arm CROSS_COMPILE=../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- make oldconfig
	$ ARCH=arm CROSS_COMPILE=../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- make prepare
	$ ARCH=arm CROSS_COMPILE=../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- make uImage
	$ ARCH=arm CROSS_COMPILE=../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- make modules

After 30 minutes... (or so) we should have the Module.symvers file. We are good to go
to build and install the module.

	$ cd ../rpi_SoftPwm
	$ ARCH=arm CROSS_COMPILE=../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf- make M=$PWD -C ../linux modules

On the pi

	$ sudo insmod pwm.ko


# Usage

The driver creates a sysfs interface under 
	
	/sys/class/soft_pwm


containing two file export & unexport. In order to create a PWM channel for a given
gpio line, just echo the line number to the export file:

	echo 4 > /sys/class/soft_pwm/export

This will create a PWM channel for gpio line 4. pwm-4 sysfs class should now be created 
in /sys/class/soft_pwm to control the channel. There are relevant files:

* /sys/class/soft_pwm/pwm-4/frequency - to control the PWM frequency, by default 50Hz, i.e cycle period 20ms.
* /sys/class/soft_pwm/pwm-4/period_ns - to control the PWM period in nano second, by default 20000000ns(20ms)
* /sys/class/soft_pwm/pwm-4/duty_cycle_ns - to control the duty_cycle of the pulse in nano second, by default 1500000ns(1.5ms)
* /sys/class/soft_pwm/pwm-4/duty_cycle - to control the duty_cycle of the pulse in percentage (0-100)

You can create as many channels as you want (as far as the system can handle it).
All the channels are independent.

In order to delete a channel, simply pass the gpio number to the unexport file.
