# XDK2MAM is open source software to bridge the Bosch XDK 110 and IOTA Tangle

The [Bosch XDK110](https://xdk.bosch-connectivity.com) is a programmable sensor device & a prototyping platform for many IoT use cases, being used in the field of Internet of Production, mainly to measure the performance of industrial machinery.

On the other side, IOTA's [Masked Authenticated Messaging (MAM)](https://blog.iota.org/introducing-masked-authenticated-messaging-e55c1822d50e) is a second layer data communication protocol which adds functionality to emit and access an encrypted data stream over the Tangle.

[XDK2MAM](https://xdk2mam.io) main goal is to provide open source software to allow interaction between this powerful hardware and the promising IOTA Tangle.

The following repository has either files for the Bosch XDK 110 and for the data receiver in Node.js where the attach to Tangle via MAM happens, featuring the different methods the XDK110 is capable of (HTTP/MQTT/BLE)

# Current stable XDK Workbench version to compile this code is 3.4.0

While our goal is to always be up to the latest release of the [XDK Workbench](https://xdk.bosch-connectivity.com/software-downloads) (an Eclipse based IDE that comes with XDK to build software and flash it to the hardware), changes made by the Bosch team from release to release tend to leave our code with compilation errors. 

This is an issue we are reviewing actively with Bosch XDK team. Until we sync we recommend to use our so called **CSVC** (current stable version to compile). This will allow you to build your project without errors so you can start working with your XDK inmmediatly after.

