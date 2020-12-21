#!/bin/
sudo insmod fpga_interface_driver.ko
sudo insmod fpga_fnd_driver.ko
sudo insmod fpga_buzzer_driver.ko
sudo insmod fpga_led_driver.ko
sudo insmod fpga_push_switch_driver.ko
sudo insmod fpga_step_motor_driver.ko
sudo mknod /dev/fpga_fnd c 261 0 
sudo mknod /dev/fpga_led c 260 0
sudo mknod /dev/fpga_push_switch c 265 0
sudo mknod /dev/fpga_buzzer c 264 0
sudo mknod /dev/fpga_step_motor c 267 0
