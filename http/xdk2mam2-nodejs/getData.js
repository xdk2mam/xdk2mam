var exec = require('child_process').exec;

var bundle = process.argv[2];
const node = "node05.iotatoken.nl"
const portNode = 16265

exec('./recv '+ node + ' ' + portNode + ' ' + bundle , (error, stdout, stderr) => {
  if (error) {
    console.error(`exec error: ${error}`);
    return;
  }
  console.log(stdout);
  console.log(stderr);
  console.log("***************************************************")
});