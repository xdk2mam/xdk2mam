# XDK2MAM USB WITH SD CARD
USB is an uniform, user-friendly interface for all peripheral devices which can be connected to a computer for both data
transfer and power supply purposes. The XDK110 supports connectivity via USB allowing the sensors node to send data to a machine without the need of being connected to the internet. 

The following repository has either files for the Bosch XDK110 and for the data receiver in Nodejs where the publish to Tangle via MAM happens.

- xdk2mam-c (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to start a listener server)
- xdk2mam2-nodejs (Node code to start a listener server and publish to MAM2)

**The Node code provided here was tested in Linux (Debian and Raspberry). If you want to fetch USB data on other OS you can extend the solution by visiting the [Tessel Repository](https://github.com/tessel/node-usb).**

**This package uses a config file on a micro sd card, which makes possible to alter some values such as the data relay interval and the sensors used without need to recompile (you just change values in the config file and you are ready to go)**


# Instructions

## Requirements
In order to be able to run the code on this repo you will need to [download XDK Workbench](https://xdk.bosch-connectivity.com/software-downloads), have a XDK110 and install [Nodejs](https://nodejs.org/en/download/) on the computer you are going to use as listener server.

Tessel Node-USB requires the install of build-essential and libudev-dev packages on Linux, so proceed to install them if you don't have those packages. For this, run the following commands. 

```
sudo apt-get install build-essential
sudo apt-get install libusb-1.0-0-dev
sudo apt-get install libudev-dev
```

## Flashing your XDK: sensors configuration
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder ***xdk2mam-c***. Accept to import project. 


### Clear, Build and Flash
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder **xdk2mam-c**. Accept to import project. Once project is imported, right click on **xdk2mam** folder in your Workbench Project Explorer and select **Clean project**. When the clean is done, repat and select **Build Project**. This process can take some minutes depending on your hardware and you should see any problems at the Workbench Console.

Finally, once the project has been built, connect your XDK 110 via USB and click the ***Flash*** button to install the software on the board. If everything went fine, you should be able to see the sensor data on your console.

### Editing config data

Open the **config.cfg** file on your computer and change the values the define an id, and the sensors you want to use.

```
DEVICE_NAME=enter-your-device-id
INTER_REQUEST_INTERVAL=30000
ENVIROMENTAL=YES
ACCELEROMETER=YES
GYROSCOPE=YES
INERTIAL=YES
LIGHT=YES
MAGNETOMETER=YES
ACOUSTIC=YES
```

Save the values, extract the micro SD card and carefully insert it into the XDK110 MicroSD slot (contacts up). 
Turn on the XDK and you are good to go! 
If everything went fine the XDK110 should now be sending its sensors data to the given destination server using the USB port. 


## Setting up Nodejs Servers for MAM1 and MAM2

### MAM 1: Setting up your Node listener
Because as soon as you flash the C program to your XDK it starts sending the sensor's data, it might be a good idea to start first the Node server that will be listening. Download and install Node.js and be sure to include npm package manager.

Navigate to your xdk2mam-nodejs folder and run the following command

```
npm i
```
Once the installation finishes, edit the xdk2mam-usb.js file to add a Full Node (be sure to use one with PoW enabled). You can browse public Full Nodes at [IOTA Dance](https://iota.dance)

```
let iota = new IOTA({
  'provider': 'http://you-pow-enabled-node:14265' //(Check https://iota.dance/)  
});
```

You will also need to check that the USB Device vid (idVendor) and pid (idProduct) parameters at **usb.findByIds(vid, pid)** match your system USB port being used by the XDK110.

Default values provided on our code (usb.findByIds(4236, 379)) seem to work for Linux. If they don't you can explore your devices buy loggin into the console the results of **usb.getDeviceList()** as follows. 

Unplug the XDK110 from the USB port, print usb.getDeviceList() on console and check the Devices listed. Plug the XDK110 back and run the usb.getDeviceList() again. The USB port in which your XDK110 is connected should now be added to the previous devices list.

![USB Device ids](https://xdk2mam.io/assets/images/XDK-USB-DEVICES.jpg)

Once this is done, start the node server (notice that you need to be rooted or use sudo)

```
sudo node xdk2mam-usb.js
```

If everything went fine and your XDK110 is on you should be getting its datasets on your console.

![Sensors data on listening server](https://xdk2mam.io/assets/images/XDK-USB-DATA.jpg)

You can verify that the sensors data has been published to the Tangle by entering the root at [thetangle.org/mam](https://thetangle.org/mam) MAM Decoder. 
For instance, this example dataset is at [YNXLOUYHPLGHQUPGQZFOOQYFYUDCUMMZKKELGIXWGQWINHAG9XDNXWBXBISCNGBHPXMVLVPNGLQDRTYBK](https://thetangle.org/mam/YNXLOUYHPLGHQUPGQZFOOQYFYUDCUMMZKKELGIXWGQWINHAG9XDNXWBXBISCNGBHPXMVLVPNGLQDRTYBK
)

### MAM 2: Setting up your Node listener
Because as soon as you flash the C program to your XDK it starts sending the sensor's data, it might be a good idea to start first the Node server that will be listening. Download and install Node.js and be sure to include npm package manager.

### Building MAM binaries
This process involves the compilation of Entangled MAM2 binaries that will be used from Node to send messages via MAM2. For this we will be using [IOT2TANGLE Cmake Entangled MAM](https://github.com/iot2tangle/cmake-mam) The given code works for UNIX based systems (Linux/MacOS). Be sure to have [GNU Compiler Collection (GCC)](https://gcc.gnu.org/) and [Cmake](https://cmake.org/install/) installed before running the following commands. 

Navigate to your xdk2mam2-nodejs folder and run the installation script (install.sh) to compile the send-msg and recv applications needed to publish and fetch data on MAM2. 

```
./install.sh
```
After the installation is done, you should be able to see the **send-msg** and **recv** executables on your **xdk2mam2-nodejs** folder. The Node scripts (server.js and getData.js) will execute this programs to publish and fetch data. 

### Setting up the Nodejs files

Once the installation finishes, edit the **server.js** file to use a valid Full Node (be sure to use one with PoW enabled). 
**Important:** pick a node that do not use SSL and skip the http://

```
const node = "node05.iotatoken.nl"
const portNode = 16265
```

You will also need to check that the USB Device vid (idVendor) and pid (idProduct) parameters at **usb.findByIds(vid, pid)** match your system USB port being used by the XDK110.

Default values provided on our code (usb.findByIds(4236, 379)) seem to work for Linux. If they don't you can explore your devices buy loggin into the console the results of **usb.getDeviceList()** as follows. 

Unplug the XDK110 from the USB port, print usb.getDeviceList() on console and check the Devices listed. Plug the XDK110 back and run the usb.getDeviceList() again. The USB port in which your XDK110 is connected should now be added to the previous devices list.

![USB Device ids](https://xdk2mam.io/assets/images/XDK-USB-DEVICES.jpg)

Once this is done, start the node server (notice that you need to be rooted or use sudo)
```
sudo node xdk2mam-usb.js
```
If everything went fine, you should be able to see data on your console. Together with the sensor's dataset you will get an Address, Message ID and Bundle.

![Sensors data on MQTT Broker](https://xdk2mam.io/assets/images/http-mam2.png)

### Getting the data
To get data published with the server.js script you will need to run the getData.js passing the Bundle as argument. 

![Sensors data on MQTT Broker](https://xdk2mam.io/assets/images/http-mam2-fetch.png)


