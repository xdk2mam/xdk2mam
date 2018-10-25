var bodyParser = require('body-parser');
var express = require('express');

var app = express();
var port = process.env.PORT || 8080;


app.use(express.static(__dirname));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({	extended: true,
                                limit: "1mb"}));


var fetch = require('node-fetch')
var crypto = require('crypto')
var Mam = require('./node_modules/xdk2mam/mam.client.js')
var IOTA = require('iota.lib.js')
var iota = new IOTA({ provider: 'https://nodes.testnet.iota.org/' })


var debug = false
let uuid = 'xdk2mam-001' // Your device ID is here.
let secretKey = 'AYMOGQYCKZMZEYJ' // Your device's secret key here


let endpoint = 'https://us-central1-marketplacev2.cloudfunctions.net/newData'


const keyGen = length => {
    var charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9'
    var values = crypto.randomBytes(length)
    var result = new Array(length)
    for (var i = 0; i < length; i++) {
    result[i] = charset[values[i] % charset.length]
    }
    return result.join('')
}


let mamState = Mam.init(iota, keyGen(81))
let mamKey = keyGen(81)


const publish = async packet => {
    mamState = Mam.changeMode(mamState, 'restricted', mamKey)
    var trytes = iota.utils.toTrytes(JSON.stringify(packet))
    var message = Mam.create(mamState, trytes)
    mamState = message.state
    await Mam.attach(message.payload, message.address)
    console.log('Attached Message')
    if (!debug) {
      let pushToDemo = await pushKeys(message.root, mamKey)
      console.log(pushToDemo)
      mamKey = keyGen(81)
    }
}

const pushKeys = async (root, sidekey) => {
    const packet = {
      sidekey: sidekey,
      root: root,
      time: Date.now()
    }
    var resp = await fetch(endpoint, {
      method: 'post',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify({ id: uuid, packet, sk: secretKey })
    })
    return resp.json()
}



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
    });
  });
  console.log('Temperature: ' + temp);
  console.log('Humidity: ' + hum);
  console.log('Pressure: ' + press);
  
  publish({
          time: Date.now(),
          data: {
            temp: temp,
            hum: hum,
            press: press
      }})

});

app.listen(port);
console.log('Server started! At http://localhost:' + port);
