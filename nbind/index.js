/* global process */

var argv = process.argv;
if(argv.length < 3) {
    console.log('Usage: node . <client_name> <address> <port> <peer_name> [hello_message]');
    process.exit(0);
}

let name = argv[2]  || 'teocli-nb';
let addr = argv[3]  || 'xxx.xxx.xx.xxx';
let port = argv[4]  || 9010;
let peer = argv[5]  || 'teo-wg-gs-1';
let hello = argv[6] || 'Hello';

var nbind = require('nbind');

var lib = nbind.init().lib;
var teo = new lib.Teo(name, addr, port, peer, hello, 

    // On connected
    (t) => { console.log("JS message: Connected"); teo = t; },

    // On disconnected
    () => console.log("JS message: Disconnected"),

    // On data received 
    (from, cmd, data, data_length) =>
        console.log("JS message: Message from: " + from + ", cmd: " + cmd + 
                ", data length: " + data_length + ", data: " + data),

    // On interval
    () => teo.sendEcho(peer, hello),

    1000
);

// Uncomment in Async mode
//console.log('Teonet client initialized and started');
//setInterval(function() {
//  console.log('JS Tik:', teo.getClientName());
//},1000);
