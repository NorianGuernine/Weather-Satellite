# What is the project weather satellite ?

It is a tool allowing to automate in a simple way the recovery of satellite images.

**Example of images retrieved :**
 
![sat1](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/imgsat.jpg)

![sat2](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/imgsat2.jpg)

# Running in a raspberry pi 4 

A linux image with the software of this project is already ready.
You just need to install it with the following commands on the raspberry pi sd card:

## SD card detection:

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ lsblk

NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
...
sda           8:0    1  14,8G  0 disk 
├─sda1        8:1    1  61,2M  0 part /media/norian/boot
└─sda2        8:2    1   2,7G  0 part /media/norian/root

```
## Unmounting the sd card

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ umount /dev/sda?
```

## Copy image to card

You can find the Linux image for the raspberry pi 4 in the image folder.

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ sudo sh
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ bzcat radio-image-raspberrypi4-64-20220422203653.rootfs.wic.bz2 > /dev/sda
```
## Connection to the raspberry pi 4 by UART

```
norian@norian-HP-Pavilion:~$ lsusb
...
Bus 001 Device 009: ID 067b:2303 Prolific Technology, Inc. PL2303 Serial Port

norian@norian-HP-Pavilion:~$ modprobe usbserial vendor=0x067b product=0x2303

norian@norian-HP-Pavilion:~$ dmesg | grep 'ttyUSB0'
[  566.914757] usb 1-1: pl2303 converter now attached to ttyUSB0

norian@norian-HP-Pavilion:~$ sudo chmod 777 /dev/ttyUSB0

norian@norian-HP-Pavilion:~$ screen /dev/ttyUSB0 115200

```

The login is root.

# Running on PC

You can compile the software for your Ubuntu machine using the makefile present in the src folder.

## How to use the project

### Hardware
This program has been tested with a DVB-T COFDM rtl2832U demodulator and a dipole antenna.
I also made a system image for a Raspberry pi 4 with yocto.

| ![RTL2832U](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/rtl2832U.jpg "RTL2832U") |  ![antenna](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/V-dipole.png "antenna") |
| :-: | :-: |
|     | Here is a [link](https://lna4all.blogspot.com/2017/02/diy-137-mhz-wx-sat-v-dipole-antenna.html) to the antenna image blog |


### How to use this software

#### First way
Create a file for each recording you want to make. The file must contain the following elements:
* name
* frequency in Hz
* Recording start date in format month, day, hour, minute, seconde (mm-dd-hh-mm-ss)
* Recording end date in format month, day, hour, minute, seconde (mm-dd-hh-mm-ss)

ex:

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ nano noaa

noaa19
137910000
02-13-08-31-00
02-13-08-37-00

norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ radio noaa19
```

#### Second way 

Just launch the software and follow the instructions:

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ radio
Please enter the number of satellites: 
2
Please enter the name of the satellite 
noaa19
Please enter the frequency 
137910000
Enter the start date of the revolution (format = mm-dd-hh-minmin-ss) 
02-13-08-31-00
Enter the end date of the revolution (format = mm-dd-hh-minmin-ss) 
02-13-08-37-00

Please enter the name of the satellite 
noaa18
Please enter the frequency 
137912500
Enter the start date of the revolution (format = mm-dd-hh-minmin-ss) 
02-13-10-16-00
Enter the end date of the revolution (format = mm-dd-hh-minmin-ss) 
02-13-10-29-00
```

#### Watchdog

You can add a watchdog in minute to make sure raspberry pi power off:

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ radio -w 36000
```
or

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ radio noaa19 -w 36000
```

#### Logfile

A log file is generated during the use of the software allowing you to know what happened during the recording.

