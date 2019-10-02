const mqtt = require('mqtt');
var exec = require('child_process').exec;
const crypto = require('crypto')

var client = mqtt.connect('mqtt://username:password@IPMQTTHOST:PORT'); //example : mqtt://johndoe:AdnxE1ac2i91@123.321.1.1:17567 
var jsonData;

const keyGen = length => {
  const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
  const values = crypto.randomBytes(length);
  return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};

const seed = keyGen(81)
const node = "node05.iotatoken.nl"
const portNode = 16265

client.on('connect', function () {
  client.subscribe('xdk2mam');
  console.log('MQTT client has been subscribed to the topic successfully!');
});

function getMyInfo(info) {
  return info.toString();
};


client.on('message', async function (topic, message) {
  jsonData = getMyInfo(message);

  console.log("Please wait...")
  console.log(jsonData)
  exec('./send-msg ' + node + ' ' + portNode + ' ' + seed + ' "' + jsonData + '" "no"', (error, stdout, stderr) => {
    if (error) {
      console.error(`exec error: ${error}`);
      return;
    }
    console.log(stdout);
    console.log(stderr);
    console.log("***************************************************")
  });

});

