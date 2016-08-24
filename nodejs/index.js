/**
 * File:   index.js
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 10, 2016, 14:56 PM
 */

var debug, withDebug;

debug = false;

if (typeof v8debug !== "undefined" && v8debug !== null) {
//    console.log("v8 debug detected");
    debug = true;
}

withDebug = process.execArgv.indexOf('--debug') > -1 || process.execArgv.indexOf('--debug-brk') > -1;

if (withDebug) {
//    console.log("started with debug flag, port: " + process.debugPort);
    debug = true;
}

if ((typeof v8debug === "undefined" || v8debug === null) && !withDebug) {
//    console.log("neither detected");
}

if(debug)
    module.exports = require("./build/Debug/teonet_client.node")
else
    module.exports = require("./build/Release/teonet_client.node")
