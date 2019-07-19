# XDK LoRa Extension package

The XDK LoRa-Extension is a wireless and secure communication solution that can exchange data up to a range of 40 km. Public or private LPWANs can be joined or created.

The following repository has instructions to setup an application and device using [The Things Network](https://www.thethingsnetwork.org/), code for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens. 

**This package uses a microSD card with a config file to specify which sensor's you want to use, the intervals and LoRa frequency, which makes possible to use the XDK in diferent networks without need to recompile (you just change values in the config file and you are ready to go)**


- xdk2mam-lora (C Code to build and flash to your XDK)
- xdk2mam-nodejs (Node code to start a listener server)
- config.cfg (Configuration values stored on the microSD)

# Instructions

## Requirements
- [XDK110](https://developer.bosch.com/web/xdk/overview)
- [XDK LoRa Extension](https://xdk.bosch-connectivity.com/extensions)
- [XDK Workbench](https://developer.bosch.com/web/xdk/downloads), 
- [Node.js](https://nodejs.org/en/download/)


## Setting up an application and device at TTN

The Things Network has open LoRa Gateways all over the world and, therefore it is a good option to use with this module. If you are running your own 
gateway modifications to the code should not be that extensive. 

Enter the TTN web, create an account to login and head to **Console**. There you will need to create first an application in order to register 
a new device. Be sure to select a Handler that is on your region. 

![Create a new application at TTN](https://xdk2mam.io/assets/images/Pic1.png)

Once you create your application, you will need to register a new device. The **Device EUI** field requires an EUI identifier located at the back of your XDK LoRa Extension. 

![Register a new device at TTN](https://xdk2mam.io/assets/images/Pic2.png)

Once you finish creating the device you will get two values that the C application will need in order to connect: 

- **Application EUI** 
- **App Key (make sure to click the code icon to get the values with the correct format )**

![Register a new device at TTN](https://xdk2mam.io/assets/images/Pic3.png)

## Setting the values on the config file

Open the config.cfg file on this repo and replace the values for APP_LORA_APP_EUI and APP_LORA_APP_KEY to reflect what you got on yout TTN device. Save the changes, copy that file to a microSD card and insert it into your XDK110. 
You are now ready to compile and flash the LoRa pckg.

**Notice that the App Key value needs to be inserted without the brackets and with no spaces.** 

```
APP_LORA_RX_WINDOW_FREQ=869525000
APP_LORA_FREQUENCY=868
APP_CONTROLLER_LORA_TX_DELAY=30000
APP_LORA_APP_EUI=70B3D57ED001F8E1
APP_LORA_APP_KEY=0x5D,0x04,0xC0,0x4C,0xF2,0x5A,0x94,0x6C,0x04,0x6E,0x7B,0xCC,0x57,0x1C,0x14,0x40
ENVIROMENTAL=YES
ACCELEROMETER=YES
GYROSCOPE=YES
LIGHT=YES
```

## Compile and flash code to XDK110

Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive, head to the lora-sdcard folder and select ***xdk2mam-lora***. Accept to import project. 


### Clear, Build and Flash
Open XDK Workbench and go to File -> Import. Choose General > Projects from Folder or Archive and select the folder **xdk2mam-lora**. Accept to import project. Once project is imported, right click on **xdk2mam** folder in your Workbench Project Explorer and select **Clean project**. When the clean is done, repat and select **Build Project**. This process can take some minutes depending on your hardware and you should see any problems at the Workbench Console.

Finally, once the project has been built, connect your XDK 110 via USB and click the ***Flash*** button to install the software on the board. If everything went fine, you should be able to see the sensor data on your console.

If everything went fine, you should see the message **LoRa Join Success**. 

![Network connection succesfully](https://xdk2mam.io/assets/images/WbConsole.png)


### Install and run the Node Server

Navigate to your xdk2mam-nodejs folder inside lora-sdcard and run the following command to install all needed dependencies. 

```
npm install
```
Once the installation finishes, you can start fetching the data from TTN to publish it to the Tangle using MAM. 
You can change the Full Node used to do the proof of work on the **config.json** file. 
Make sure to have your XDK110 switched on and start the Node.js publisher with:

```
node lora-xdk2mam.js
```

![Network connection succesfully](https://xdk2mam.io/assets/images/PicNodeConsole.png)

If everything went fine you should get the datasets on your console together with a root per message to verify publication on the Tangle. Here is one [one dataset](https://devnet.thetangle.org/mam/VFVLI9H9TRNAPSYYSJYEJDJNSTSHYXRCWMLCRVZNEEUY9WF99RSVTAJDCIXRSVNEWBKGMTXKFDZCWTHIK) published by us while doing this guide. 

