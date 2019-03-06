
const sensordata = require('xdk2mam');
const IOTA = require('iota.lib.js');
const mqtt = require ('mqtt');
var crypto = require('crypto');

var Mam = require('./node_modules/xdk2mam/mam.client.js');

var client = mqtt.connect('mqtt://username:password@IPMQTTHOST:PORT'); //example : mqtt://johndoe:AdnxE1ac2i91@123.321.1.1:17567
var jsonData;

// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
	'provider': 'https://node0.capjupiter.com:14267'
});

client.on('connect', function () {
  client.subscribe('xdk2mam');
  console.log('MQTT client has been subscribed to the topic successfully!');
});

const keyGen = length => {
    var charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9'
    var values = crypto.randomBytes(length)
    var result = new Array(length)
    for (var i = 0; i < length; i++) {
    result[i] = charset[values[i] % charset.length]
    }
    return result.join('')
}

const seed = keyGen(81);


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
