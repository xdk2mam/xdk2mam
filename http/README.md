# XDK2MAM HTTP Protocol
HTTP is a common protocol to transfer data and files over the network. The XDK supports HTTP natively and offers two modules to make HTTP requests. This guide will provide an introduction to both of them and will demonstrate how to use them to make GET and POST request.

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens. 



- xdk2mam-c (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to start a listener server)

# Instructions

## Requirements
In order to be able to run the code on this repo you will to [download XDK Workbench](https://xdk.bosch-connectivity.com/software-downloads), have a XDK 110 and insall Node on the computer you are going to use as listener server.

## Flashing your XDK: wifi and sensors configuration
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder ***xdk2mam-c***. Accept to import project. Navigate to the source folder and edit the following lines at ***xdk2mam.h***


```
#define DEVICE_NAME      "enter-your-device-id"
```

```
#define WLAN_SSID        "enter-your-wifi-ssid"
```

```
#define WLAN_PSK         "enter-your-wifi-password"
```
```
#define DEST_SERVER_HOST         "192.168.7.181"
```

```
#define INTER_REQUEST_INTERVAL   UINT32_C(30000)
```
By default this code will stream data for every sensor built in in the XDK 110. If you want to only use some sensors, edit the file ***xdk2mam.c*** and switch to false on the ones you don't want to use. For instance, following edit will not send data for  Light and Magnometer sensors

```
// Global array of all sensors => true : enable -- false : disable
bool typesSensors[6] = {
						true, //ENVIROMENTAL
						true, //ACCELEROMETER
						true, //GYROSCOPE
						true, //INERTIAL
						false, //LIGHT
						false  //MAGNETOMETER
					};
```

### Clear, Build and Flash
Once changes are to this files are saved, right click on ***xdk2mam*** folder in your Workbench Project Explorer and select ***Clean project***. Once this is done, repat and select ***Build Project***. Finally, once the project has been built, connect your XDK 110 via USB and click the ***Flash*** button to install the software on the board.


