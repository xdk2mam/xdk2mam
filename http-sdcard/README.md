# XDK2MAM HTTP Protocol WITH SD CARD
HTTP is a common protocol to transfer data and files over the network. The XDK supports HTTP natively and offers two modules to make HTTP requests. This guide will provide an introduction to both of them and will demonstrate how to use them to make GET and POST request.

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens. 

**This package is a variation of the HTTP one that allows to use WLAN SSID, Password, Host and other needed values from a config file on a micro sd card, which makes possible to use the XDK in diferent networks without need to recompile (you just change values in the config file and you are ready to go)**



- xdk2mam-c (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to start a listener server)

# Instructions

## Requirements
In order to be able to run the code on this repo you will to [download XDK Workbench](https://xdk.bosch-connectivity.com/software-downloads), have a XDK 110 and insall Node on the computer you are going to use as listener server.

## Setting up your Node listener
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
Now we are ready to start with the XDK software.


## Flashing your XDK: wifi and sensors configuration
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder ***xdk2mam-c***. Accept to import project. 


### Clear, Build and Flash
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder **xdk2mam-c**. Accept to import project. Once project is imported, right click on **xdk2mam** folder in your Workbench Project Explorer and select **Clean project**. When the clean is done, repat and select **Build Project**. This process can take some minutes depending on your hardware and you should see any problems at the Workbench Console.

Finally, once the project has been built, connect your XDK 110 via USB and click the ***Flash*** button to install the software on the board. If everything went fine, you should be able to see the sensor data on your console.

### Editing config data

Open the **config.cfg** file on your computer and change the values to match your WLAN data, host, port and the sensors you want to use.

```
DEVICE_NAME=enter-your-device-id
WLAN_SSDI=enter-your-wifi-ssid
WLAN_PSK=enter-your-wifi-password
DEST_SERVER_HOST=192.168.0.4
DEST_SERVER_PORT=8080
INTER_REQUEST_INTERVAL=3000
DEST_POST_PATH=/sensors
ENVIROMENTAL=YES
ACCELEROMETER=YES
GYROSCOPE=YES
INERTIAL=YES
LIGHT=YES
MAGNETOMETER=YES
ACOUSTIC=YES
```

Save the values, extract the micro SD card and carefully insert it into the XDK SD slot (contacts up). 
Turn on the XDK and you are good to go! 
If everything went fine you should see data on your console. 



![Sensors data on listening server](https://xdk2mam.io/assets/images/server-node.png)
