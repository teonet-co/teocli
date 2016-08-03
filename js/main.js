console.info("node.js adapter to teo");

teoclient = require("/home/aksenofo/PROJECT/TEOCLI/teocli/js/node_modules/teoclient/build/Release/teoclient.node")

console.log(teoclient.version())
teoclient.init();
teoclient.cleanup();
