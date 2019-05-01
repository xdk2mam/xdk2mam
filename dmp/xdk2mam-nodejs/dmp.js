var bodyParser = require('body-parser')
var express = require('express')
var colors = require('colors')

var app = express();
var port = process.env.PORT || 8080;


app.use(express.static(__dirname));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true,
                                limit: "1mb"}))


var fetch = require('node-fetch')
const crypto = require('crypto')
const Mam = require('@iota/mam')
const { asciiToTrytes } = require('@iota/converter')
const { debug, endpoint, secretKey, provider, sensorId } = require('./config.json')


const keyGen = length => {
  const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
  const values = crypto.randomBytes(length);
  return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};


let mamState = Mam.init(provider)


const storeKey = async (sidekey, root, time) => {
  if (debug) return 'Debug mode';

  const packet = {
    sidekey,
    root,
    time,
  };

  try {
    const resp = await fetch(endpoint, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ id: sensorId, packet, sk: secretKey }),
    });
    return await resp.json();
  } catch (error) {
    console.log('storeKey error', error);
    return error;
  }
};


const publish = async payload => {
  const time = Date.now();
  const packet = { time, data: { ...payload } };
  let mamKey = keyGen(81);
  mamState = Mam.changeMode(mamState, 'restricted', mamKey);
  const trytes = asciiToTrytes(JSON.stringify(packet));
  const message = Mam.create(mamState, trytes);
  mamState = message.state;
  await Mam.attach(message.payload, message.address);
  const callbackResponse = await storeKey(mamKey, message.root, time);

  console.log('Payload:', packet);
  console.log(callbackResponse);
  console.log('==============================================================');
};

app.post('/sensors', async function(req, res) {
  var temp,press,hum;
  req.body.xdk2mam.forEach(function(element){
    element.data.forEach(function(data){
      if(data.hasOwnProperty('Temp'))
        temp = data.Temp;
      else if(data.hasOwnProperty('Humidity'))
        hum = data.Humidity;
      else if(data.hasOwnProperty('Pressure'))
        press = data.Pressure;
    });
  });
  console.log('Temperature: ' + colors.green.bold(temp) + " ° mC");
  console.log('Humidity: ' + colors.green.bold(hum) + " Hg");
  console.log('Pressure: ' + colors.green.bold(press) + " Pa");
  
  publish({ 
            temp: temp,
            hum: hum,
            press: press
          })

});

app.listen(port);
console.log('Server started! At http://localhost:' + port);