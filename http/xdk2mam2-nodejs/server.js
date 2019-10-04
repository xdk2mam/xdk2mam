var bodyParser = require('body-parser');
var express = require('express');
var exec = require('child_process').exec;
const crypto = require('crypto')

// Start Express on given port
var app = express();
var port = process.env.PORT || 8080;


app.use(express.static(__dirname));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({	extended: true,
                                limit: "1mb"}));

const keyGen = length => {
  const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
  const values = crypto.randomBytes(length);
  return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};

const seed = keyGen(81)
const node = "node05.iotatoken.nl"
const portNode = 16265



app.post('/sensors', async function(req, res) {

  const data = JSON.stringify(req.body)
  console.log("Please wait...")
  console.log(data)
  exec('./send-msg '+ node +' ' + portNode + ' ' +seed + ' ' + data + ' "no"' , (error, stdout, stderr) => {
    if (error) {
      console.error(`exec error: ${error}`);
      return;
    }
    console.log(stdout);
    console.log(stderr);
    console.log("***************************************************")
  });
  res.send("OK");

});

app.get('/status', function(req, res) {
    res.send("OK");
});

app.listen(port);
console.log('Server started! At http://localhost:' + port);
