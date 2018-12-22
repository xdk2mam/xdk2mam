# XDK2MAM Bluethooth Low Energy (BLE) WITH SD CARD
The XDK provides several BLE APIs to manage the BLE functionality on the device. The interfaces can be used by applications in order to communicate via the ALPWISE BLE stack with surrounding BLE devices.
A wide range of BLE functionalities can be achieved using the XDK, from configuration of the BLE controller according to the requirements of the designed XDK application, up to active BLE connections, including data
exchange in both ways between the XDK and other BLE devices. 

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens.

**Note that the Node code provided here works only in Linux (aimed to Raspberry Pi mainly). If you want to set a receiver on other OS you can extend the solution by visiting the [Noble Repository](https://github.com/noble/noble)**


**This package uses a config file on a micro sd card, which makes possible to alter some values as the interval or the sensors used without need to recompile (you just change values in the config file and you are ready to go)**



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
Once the installation finishes, edit the xdk2mam-ble.js file to add your Full Node (be sure to use one with PoW enabled).

```
let iota = new IOTA({
  'host': 'https://your-node.com',
  'port': '14265'
});
```
And also change the seed value to your seed so you can open your own channel

```
const seed = "ENTERYOURSEEDITBWTGFTAFBZ9SXDSUNANZA9TGAOSIICFFOBHNUXQCFZWO9DSPUQUIZIJXOPHBY99999";
```

Once this is done, start the node server

```
node xdk2mam-ble.js enter-your-device-id (this value is defined on the first line of the config file)
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
INTER_REQUEST_INTERVAL=30000
INTERVAL_STREAM_DIVIDER_BLE=250
ENVIROMENTAL=YES
ACCELEROMETER=YES
GYROSCOPE=YES
INERTIAL=YES
LIGHT=YES
MAGNETOMETER=YES
```

Save the values, extract the micro SD card and carefully insert it into the XDK SD slot (contacts up). 
Turn on the XDK and you are good to go! 
If everything went fine you should see data on your console. 



![Sensors data on listening server](https://xdk2mam.io/assets/images/ble-screen.png)
