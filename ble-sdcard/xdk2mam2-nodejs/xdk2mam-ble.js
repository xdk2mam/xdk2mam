var exec = require('child_process').exec;
var Noble = require('noble/lib/noble');
var bindings = require('noble/lib/resolve-bindings')();
var colors = require('colors');
var crypto = require('crypto');

var noble = new Noble(bindings);

const keyGen = length => {
    const charset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ9';
    const values = crypto.randomBytes(length);
    return Array.from(new Array(length), (x, i) => charset[values[i] % charset.length]).join('');
};

const seed = keyGen(81)
const node = "node05.iotatoken.nl"
const portNode = 16265

var peripheralName = process.argv[2];

noble.on('stateChange', function (state) {
    if (state === 'poweredOn') {
        noble.startScanning();
    } else {
        noble.stopScanning();
    }
});


noble.on('discover', function (peripheral) {

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

    peripheral.connect(function (error) {
        peripheral.discoverServices([], function (error, services) {
            var c;
            var body = '';
            var streamToTangle = '';
            services[services.length - 1].discoverCharacteristics([], function (error, characteristics) {
                c = characteristics[characteristics.length - 1];
                console.log(colors.green.bold(c.uuid.toString()) + '\n');

                console.log('**********************************************************************\n'.green);

                c.on('data', async function (data, isNoti) {
                    if (data.toString().replace(/\0/g, '').length == 20) {
                        process.stdout.write(data.toString());
                        body += data.toString();
                    }
                    else {

                        if (data.toString().replace(/\0/g, '').length == 19 && data.toString().replace(/\0/g, '') == "@@@_finish_data_@@@") {
                            streamToTangle = body.replace(/\0/g, '');
                            var time = '"' + (Math.floor(Date.now() / 1000)).toString() + '"' + '}';
                            console.log(time);
                            console.log('***********************************************************************\n'.green);
                            streamToTangle += time;
                            body = '';
                            exec('./send-msg ' + node + ' ' + portNode + ' ' + seed + ' "' + streamToTangle + '" "no"', (error, stdout, stderr) => {
                                if (error) {
                                    console.error(`exec error: ${error}`);
                                    return;
                                }
                                console.log(stdout);
                                console.log(stderr);
                            });
                            streamToTangle = '';
                            console.log('***********************************************************************\n'.green);
                        } else {
                            process.stdout.write(data.toString());
                            body += data.toString();
                        }

                    }
                });


            });

        });
    });
}
