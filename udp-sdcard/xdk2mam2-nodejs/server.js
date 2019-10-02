var crypto = require('crypto');
var dgram = require('dgram');
var server = dgram.createSocket('udp4');
var exec = require('child_process').exec;
const port = 8080

const keyGen = length => {
    const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
    const values = crypto.randomBytes(length);
    return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};

const seed = keyGen(81)
const node = "node05.iotatoken.nl"
const portNode = 16265

server.on('listening', () => {
    var address = server.address();
    console.log('UDP Server listening on port: ' + address.port);
});


server.on('message', async (msg, remote) => {
    console.log('\n*****************************************************************');
    console.log('\n** Data: ', msg.toString('utf8'));

    exec('./send-msg ' + node + ' ' + portNode + ' ' + seed + ' "' + msg.toString('utf8') + '" "no"', (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
        console.log(stdout);
        console.log(stderr);
        console.log("***************************************************")
    });
});

server.bind(port);