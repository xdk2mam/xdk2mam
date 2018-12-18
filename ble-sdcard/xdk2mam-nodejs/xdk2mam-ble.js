
var Noble = require('./node_modules/noble/lib/noble');
var bindings = require('./node_modules/noble/lib/resolve-bindings')();
var colors = require('colors');

var noble = new Noble(bindings);

let sensordata = require('xdk2mam');
var IOTA = require('iota.lib.js');
var Mam = require('./node_modules/xdk2mam/mam.client.js');

// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
  'host': 'https://iota-3.de',//'http://173.212.193.59',
  'port': '14267'
});

// Replace by your seed
const seed = "ANTERYOURSEEDITBWTGFTAFBC9SXDSUNDNZA9TGAOSIICFFOBHNUOQCTSWO9DSWUQUIZIJBOPHBYA9999";

let mamState = Mam.init(iota, seed, 2);

var peripheralName= process.argv[2];

noble.on('stateChange', function(state) {
  if (state === 'poweredOn') {
    noble.startScanning();
  } else {
    noble.stopScanning();
  }
});


noble.on('discover', function(peripheral) {



  if (peripheral.advertisement.localName === undefined ? peripheral.advertisement.localName = '' : 
    peripheral.advertisement.localName.toString().replace(/\0/g, '') == 
    peripheralName.toString().replace(/\0/g, '')) {
    
    noble.stopScanning();    

    console.log('*  Peripheral with ID ' + colors.green.bold(peripheral.id.toString()) + ' found:');
    var advertisement = peripheral.advertisement;

    var localName = advertisement.localName;
    var serviceUuids = advertisement.serviceUuids;

    if (localName) {
      console.log('  Local Name        = ' + colors.green.bold(localName.toString()));
    }

    if (serviceUuids) {
      console.log('  Service UUIDs     = ' + colors.green.bold(serviceUuids.toString()));
    }

    console.log();

    explore(peripheral);
  }
});

function explore(peripheral) {
  console.log('*  Services and characteristics:');  

  peripheral.connect(function(error) {
    peripheral.discoverServices([], function(error, services) {      
      var c;
      var body = '';
      var streamToTangle = '';  
      services[services.length-1].discoverCharacteristics([], function(error, characteristics) {   
        c = characteristics[characteristics.length-1];
        console.log(colors.green.bold(c.uuid.toString())+'\n');

        console.log('**********************************************************************\n'.green);          

        c.on('data',async function(data,isNoti){          
          if(data.toString().replace(/\0/g, '').length == 20){
            process.stdout.write(data.toString());
            body += data.toString();
          }            
          else{

            if(data.toString().replace(/\0/g, '').length == 19 && data.toString().replace(/\0/g, '') == "@@@_finish_data_@@@"){                                          
              streamToTangle = body.replace(/\0/g, '');
              var time = '"'+(Math.floor(Date.now() / 1000)).toString()+'"' + '}';
              console.log(time);  
              console.log('***********************************************************************\n'.green);          
              streamToTangle +=  time;              
              body = '';
              await sensordata.saveDataBLE(streamToTangle,mamState,iota).then(ms => {
                  mamState = ms;
              });
              streamToTangle = '';
              console.log('***********************************************************************\n'.green);          
            }else{
              process.stdout.write(data.toString());
              body += data.toString(); 
            }    

          }
        });

        
      });

    });
  });
}
