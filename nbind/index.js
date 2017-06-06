var nbind = require('nbind');
var lib = nbind.init().lib;

lib.Teo.main_select('teocli-nb', 'gt1.kekalan.net', 9010, 'ps-server', 'Hello', 
    function() {
        console.log("JS message: Connected");
    },
    function() {
        console.log("JS message: Disconnected");
    },
    function(from, cmd, data, data_length) {
        console.log("JS message: Msaage from: " + from + ", cmd: " + cmd + ", data length: " + data_length + ", data: " + data);
    }
);

