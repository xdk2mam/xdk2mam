# XDK2MAM HTTP Protocol
HTTP is a common protocol to transfer data and files over the network. The XDK supports HTTP natively and offers two modules to make HTTP requests. This guide will provide an introduction to both of them and will demonstrate how to use them to make GET and POST request.

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens. 

- xdk2mam-c (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to start a listener server)
- xdk2mam2-nodejs (Node code to start a listener server and publish to MAM2)

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
Once changes are to this files are saved, right click on ***xdk2mam*** folder in your Workbench Project Explorer and select ***Clean project***. Once this is done, repat and select ***Build Project***. This process can take some minutes depending on your hardware and you should see any problems at the Workbench Console.

Finally, once the project has been built, connect your XDK110 via USB and click the ***Flash*** button to install the software on the board. If everything went fine the XDK110 should now be sending its sensors data to the given destination server. 


## Setting up Nodejs Servers for MAM1 and MAM2

### MAM 1: Setting up your Node listener

Because as soon as you flash the C program to your XDK it starts sending the sensor's data, it might be a good idea to start first the Node server that will be listening. Download and install Node.js and be sure to include npm package manager.

Navigate to your xdk2mam-nodejs folder and run the following command

```
npm install
```
Once the installation finishes, edit the server.js file to add your Full Node (be sure to use one with PoW enabled).

```
let iota = new IOTA({
  'host': 'https://your-node.com',
  'port': '14265'
});
```
And also change the seed value to your seed

```
const seed = "ENTERYOURSEEDITBWTGFTAFBZ9SXDSUNANZA9TGAOSIICFFOBHNUXQCFZWO9DSPUQUIZIJXOPHBY99999";
```

Once this is done, start the node server

```
npm start
```
If everything went fine and your XDK110 is on you should be getting its datasets on your console.


![Sensors data on listening server](https://xdk2mam.io/assets/images/server-node.png)

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

Once this is done, start the node server

```
npm start
```
If everything went fine, you should be able to see data on your console. Together with the sensor's dataset you will get an Address, Message ID and Bundle.

![Sensors data on MQTT Broker](https://xdk2mam.io/assets/images/http-mam2.png)

### Getting the data
To get data published with the server.js script you will need to run the getData.js passing the Bundle as argument. 

![Sensors data on MQTT Broker](https://xdk2mam.io/assets/images/http-mam2-fetch.png)


