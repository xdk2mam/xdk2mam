var usb = require('usb')
var colors = require('colors')
var crypto = require('crypto');
var exec = require('child_process').exec;

//Check if the ids (idVendor, idProduct) are correct. Take a look at this function 'usb.getDeviceList()'
var xdk110 = usb.findByIds(4236, 379)

xdk110.open()

const keyGen = length => {
    const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
    const values = crypto.randomBytes(length);
    return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};

const seed = keyGen(81)
const node = "node05.iotatoken.nl"
const portNode = 16265

if (xdk110.interfaces[0].isKernelDriverActive())
    xdk110.interfaces[0].detachKernelDriver()

var inEndpoint = xdk110.interfaces[1].endpoints[0];

xdk110.interfaces[1].claim()

inEndpoint.startPoll(1, 800)

console.log('*********************')
console.log('*    ' + colors.green.bold('XDK2MAM-USB') + '    *')
console.log('*********************\n')
console.log('Listening...')

inEndpoint.on('data', async function (data) {
    if (data != undefined) {
        var time = (Math.floor(Date.now() / 1000)).toString()
        var obj = JSON.parse(data.toString('utf8'))
        obj.timestamp = time
        var info = JSON.stringify(obj)

        console.log('\n***********************************************************************\n'.green)
        console.log(info)
        console.log('\n***********************************************************************\n'.green)
    
        exec('./send-msg ' + node + ' ' + portNode + ' ' + seed + ' "' + info + '" "no"', (error, stdout, stderr) => {
            if (error) {
                console.error(`exec error: ${error}`);
                return;
            }
            console.log(stdout);
            console.log(stderr);
        });

    }

});

inEndpoint.on('error', function (error) {
    console.log("XDK110 => ", error);
});

inEndpoint.on('end', function () {
    console.log("XDK110 off.");
});
