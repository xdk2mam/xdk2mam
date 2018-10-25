# XDK2MAM Datamarketplace (HTTP/SD/Node.js Template)
The Data Marketplace is IOTA’s most comprehensive pilot study thus far. The goal is to enable a truly decentralized data marketplace to open up the data silos that currently keep data limited to the control of a few entities. Data is one of the most imperative ingredients in the machine economy and the connected world.

HTTP is a common protocol to transfer data and files over the network. The XDK supports HTTP natively and offers two modules to make HTTP requests. This guide will provide an introduction to both of them and will demonstrate how to use them to make GET and POST request.

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js in charge to publish sensor's data to the Datamarketplace. 

**This package is a variation of the HTTP one that allows to use WLAN SSID, Password, Host and other needed values from a config file on a micro sd card, which makes possible to use the XDK in diferent networks without need to recompile (you just change values in the config file and you are ready to go)**



- xdk2mam-c (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to publish data to DMP)
- config.cfg (Config file for XDK to read values from microSD card)

# Instructions

## Requirements
In order to be able to run the code on this repo you will to [download XDK Workbench](https://xdk.bosch-connectivity.com/software-downloads), have an XDK 110 and insall Node on the computer/server you are going to use as listener server.
To be able to publish data to the DMP you will need access to the Dashboard to create the devices that will recieve the XDK data. 

## 1. Access Datamarketplace dashboard to create a device
First things first. Go to the DMP Dashboard, click on **Add Device** and complete the form with the corresponding values. 
For this example we will be only using data for a Weather Station but you could add all the sensor's to the stream by entering the proper **Field ID**.

![Sensors data on listening server](https://xdk2mam.io/assets/images/create-device.png)

Once you have your device created copy your the Device ID (xdk2mam-001) and the SecretKey. You will need this values later.

![Sensors data on listening server](https://xdk2mam.io/assets/images/SK.png)


## 2. Setting up your Node DMP data publisher
Navigate to your xdk2mam-nodejs folder and run the following command

```
npm install
```
Once the installation finishes, [edit the dmp.js file to add your Device ID and SecretKey](https://github.com/xdk2mam/xdk2mam/blob/df7aae49b72d03108d62af06854cc7daed1b796b/dmp/xdk2mam-nodejs/dmp.js#L22)

```
let uuid = 'xdk2mam-001' // Your device ID is here.
let secretKey = 'YOURSECRETKEY' // Your device's secret key here
```

Once this is done, start the node server

```
node dmp.js
```

![Sensors data on listening server](https://xdk2mam.io/assets/images/console-listening.png)


You will see how the Node.js is now waiting to get data from the XDK110. **Let's get that done!**

## 3. Flashing your XDK: wifi and sensors configuration
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
ENVIROMENTAL=YES
ACCELEROMETER=YES
GYROSCOPE=YES
INERTIAL=YES
LIGHT=YES
MAGNETOMETER=YES
```

Save the values, extract the micro SD card and carefully insert it into the XDK SD slot (contacts up). 
Turn on the XDK and you are good to go! 
If everything went fine you should see your **Weather station data on your console**. 

![Sensors data on listening server](https://xdk2mam.io/assets/images/console-fetching.png)

Finally, point your browser to https://datamarket-url/#/sensor/xdk2mam-001, fund your wallet and purchase access to the data stream to check everything is working fine

![Sensors data on listening server](https://xdk2mam.io/assets/images/sensor-stream.png)

## Adding more sensors to the stream
You can add data for other XDK110 sensors to your DMP stream by listing them in the **dmp.js** file. You just need to follow every sensor naming convention. For instance, if you were to add data from the Light sensor, you should just include it like this: 

```
app.post('/sensors', async function(req, res) {
  var temp,press,hum,lux;
  req.body.xdk2mam.forEach(function(element){
    element.data.forEach(function(data){
      if(data.name == 'Temperature')
        temp = data.value;
      else if(data.name == 'Humidity')
        hum = data.value;
      else if(data.name == 'Pressure')
        press = data.value;
      else if(data.name == 'milliLux')
        light = data.value;
    });
  });
```
In this case, the Light sensor value must be retrieved as **milliLux** acording to [how it was named in the JSON object created by the XDK110 sensor](https://github.com/xdk2mam/xdk2mam/blob/1232407c86dd2540aeda7653faed6b27e5c2b1ae/dmp/xdk2mam-c/source/sensors/LightSensor.c#L67).

**Note that in order to see this value on the DMP stream you will have to define it together with the others.**

![Added milliLux value to DMP device](https://xdk2mam.io/assets/images/milli-lux.png)


