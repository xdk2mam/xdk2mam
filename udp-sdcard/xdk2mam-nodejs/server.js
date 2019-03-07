let sensordata = require('xdk2mam');
var IOTA = require('iota.lib.js');
var Mam = require('./node_modules/xdk2mam/mam.client.js');
var crypto = require('crypto');

var dgram = require('dgram');
var server = dgram.createSocket('udp4');


// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
	'provider': 'https://node0.capjupiter.com:14267'
});

const port = 8080

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

server.on('listening', () => {
    var address = server.address();
    console.log('UDP Server listening on port: '+ address.port);
});


server.on('message', async (msg, remote) => {
    console.log('\n*****************************************************************');
    console.log('\n** Data: ', msg.toString('utf8'));
	await sensordata.saveDataAndPrintRoot(msg.toString('utf8'),mamState,iota).then(ms => {
	    mamState = ms;
	  });
});

server.bind(port);