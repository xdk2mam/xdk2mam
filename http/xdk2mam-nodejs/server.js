let sensordata = require('xdk2mam');
var bodyParser = require('body-parser');
var express = require('express');
var IOTA = require('iota.lib.js');
var Mam = require('./node_modules/xdk2mam/mam.client.js');

// Start Express on given port
var app = express();
var port = process.env.PORT || 8080;


app.use(express.static(__dirname));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({	extended: true,
                                limit: "1mb"}));

// Enter your Node URL and port (be sure to use a node with PoW enabled)
let iota = new IOTA({
  'host': 'http://your-node.com',
  'port': '14265'
});

// Replace by your seed
const seed = "BIXGXUSFAJLKSQXBABITBWTGFTAFBZ9SXDSUNANZA9TGAOSIICFFOBHNUXQCFZWO9DSPUQUIZIJXOPHBY";


let mamState = Mam.init(iota, seed, 2);


app.post('/sensors', async function(req, res) {

  await sensordata.saveData(req.body,mamState,iota).then(ms => {
    mamState = ms;
    res.send("OK");
  });

});

app.get('/status', function(req, res) {
    res.send("OK");
});

app.listen(port);
console.log('Server started! At http://localhost:' + port);
