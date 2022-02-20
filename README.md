# Weather satellite
This is a tool for retrieving images sent by radio from weather satellites.

## How to use it

### Harware
This program has been tested with a DVB-T COFDM rtl2832U demodulator and a dipole antenna.
I also made a system image for a Raspberry pi 4 with yocto. The system image is available in the src folder.

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

norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ ./Weather_Sat noaa19
```

#### Second way 

Just launch the software and follow the instructions:

```
norian@norian-HP-Pavilion:~/Documents/github/Weather-Satellite$ ./Weather_Sat
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

#### Results
Here are some images recovered with this software:

![sat1](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/imgsat.jpg)

![sat2](https://github.com/NorianGuernine/Weather-Satellite/blob/main/Pictures/imgsat2.jpg)
