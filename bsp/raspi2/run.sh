#!/bin/bash
qemu-system-arm -M raspi2  -smp 4 -m 512M -global bcm2835-fb.pixo=0 -name "Raspberry Pi 2" -show-cursor -rtc base=localtime -serial null -serial stdio -kernel rtthread.elf

