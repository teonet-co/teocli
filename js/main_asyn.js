var sleep = require('sleep');

console.info("node.js adapter to teo");

teoclient = require("./node_modules/teoclient/build/Debug/teoclient.node")

teoclient.init();

try {

    var ttt = 10;

while(ttt) {
    var connector = teoclient.connectAsync("127.0.0.1", 9000, function(connector, error) {
	console.log(connector)
	connector.login("my_name", function (){
	})
	snd = connector.send(connector.CMD_L_PEERS, "teotest");
	connector.sleep(.5);
	console.log("Recv:", connector.recv());
	console.log(ttt);
	connector.disconnect();
    })
}

//    setTimeout(suspend.resume(), 10000);
//    sleep.sleep(10)

/*
    var snd = connector.login("my_name");
    console.log("Login Sent:", snd)

    snd = connector.send(connector.CMD_L_PEERS, "teotest");
    console.log("CMD_L_PEERS Sent:", snd)

    snd = connector.send(connector.CMD_L_L0_CLIENTS, "teotest");
    console.log("CMD_L_L0_CLIENTS Sent:", snd)

    snd = connector.send(connector.CMD_L_ECHO, "teotest", "hello");
    console.log("CMD_L_ECHO Sent:", snd)

    connector.sleep(.5);
    console.log("-------------------------------------------------------");
    console.log("Recv:", connector.recv());

    console.log("-------------------------------------------------------");
    console.log("Recv:", connector.recv());

//    console.log("-------------------------------------------------------");
//    console.log("Recv:", connector.recv());
    connector.disconnect();
//    connector.disconnect();
//    snd = connector.send(connector.CMD_L_ECHO, "teotest", "hello");
*/
}
catch (err) {
    console.log(err)
}

teoclient.cleanup();
