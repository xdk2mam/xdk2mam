var usb = require('usb')
var colors = require('colors')
let sensordata = require('xdk2mam')
var IOTA = require('iota.lib.js')
var Mam = require('./node_modules/xdk2mam/mam.client.js')

// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
  'provider': 'http://you-pow-enabled-node:14265' //(Check https://iota.dance/)  
});

let mamState = Mam.init(iota,undefined,2)
//Check if the 'ids' are correct. Take a look at this function 'usb.getDeviceList()'
var xdk110 = usb.findByIds(4236, 379)

xdk110.open()

if(xdk110.interfaces[0].isKernelDriverActive())
	xdk110.interfaces[0].detachKernelDriver()

var inEndpoint = xdk110.interfaces[1].endpoints[0];

xdk110.interfaces[1].claim()

inEndpoint.startPoll(1, 800)

console.log('*********************')
console.log('*    ' + colors.green.bold('XDK2MAM-USB') + '    *')
console.log('*********************\n')
console.log('Listening...')

inEndpoint.on('data', async function (data) {	
	if(data!=undefined){
		var time = (Math.floor(Date.now() / 1000)).toString()
		var obj = JSON.parse(data.toString('utf8'))
		obj.timestamp = time
		var info  = JSON.stringify(obj)		

		console.log('\n***********************************************************************\n'.green)
		console.log(info)
		console.log('\n***********************************************************************\n'.green)
        await sensordata.saveDataAndPrintRoot(info,mamState,iota).then(ms => {
          mamState = ms;
      	});
      	
	}

});

inEndpoint.on('error', function (error) {
	console.log("XDK110 => ", error);
});

inEndpoint.on('end', function () {
	console.log("XDK110 off.");
});
