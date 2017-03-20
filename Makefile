obj-m += soft_pwm.o
soft_pwm-objs := pwm.o
ccflags-y := -O2 -Wno-unused-function
