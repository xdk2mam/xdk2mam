
const sensordata = require('xdk2mam');
const IOTA = require('iota.lib.js');
const mqtt = require ('mqtt');

var Mam = require('./node_modules/xdk2mam/mam.client.js');

var client  = mqtt.connect('mqtt://username:password@IPMQTTHOST:PORT'); //example : mqtt://johndoe:AdnxE1ac2i91@123.321.1.1:17567
var jsonData;

// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
  'host': 'http://your-node.com',
  'port': '14265'
});

client.on('connect', function () {
  client.subscribe('xdk2mam');
  console.log('MQTT client has been subscribed to the topic successfully!');
});

// Replace by your seed
const seed = "BIXGXUSFAULKSQXBABITBITGFTAFGZ9SXDSUNANZA9ATAOSIICFFOBHNTGQCFZWO9DSPUQUIZIJXOPHBY";

let mamState = Mam.init(iota, seed, 2);

function getMyInfo(info){
 return info.toString();
};


client.on('message', async function (topic, message){
  jsonData = getMyInfo(message);

  await sensordata.saveDataMqtt(jsonData,mamState,iota).then(ms => {
    mamState = ms;
  });

});
