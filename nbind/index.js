/* global process */

var nbind = require('nbind');
var lib = nbind.init().lib;

let argv = process.argv;
//console.log("argv:", argv);

if(argv.length < 3) {
    console.log('Usage: node . <client_name> <address> <port> <peer_name> [hello_message]');
    process.exit(0);
}

let name = argv[2]  || 'teocli-nb';
let addr = argv[3]  || 'xxx.xxx.xx.xxx';
let port = argv[4]  || 9010;
let peer = argv[5]  || 'teo-wg-gs-1';
let hello = argv[6] || 'Hello';

var teo = new lib.Teo(name, addr, port, peer, hello, 

    // On connected
    function() {
        console.log("JS message: Connected");
    },
    
    // On disconnected
    function() {
        console.log("JS message: Disconnected");
    },
    
    // On data received 
    function(from, cmd, data, data_length) {
        console.log("JS message: Msaage from: " + from + ", cmd: " + cmd + 
                ", data length: " + data_length + ", data: " + data);
    },
      
    // On interval
    (t) => {
        //console.log("Hello");
        //console.log(teo.hello());
        t.sendEcho(peer, hello);
        
        //setTimeout(function() { console.log("Hello"); }, 1);
    },
    
    1000
);
