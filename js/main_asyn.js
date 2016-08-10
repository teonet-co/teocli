
teoclient = require("./node_modules/teoclient/build/Debug/teoclient.node")

teoclient.init();

var t = new Buffer(10);

try {
    var counter = 0;
    var status = 0;
    var connector = teoclient.connectAsync("127.0.0.1", 9000,
	function(ev, buf, err) { // Progress
	    try {
		if(ev != connector.EV_L_TICK )
		    console.log(counter++, "PROGRESS", ev, buf, err)
		if(status == 0) {
		    if(ev == connector.EV_L_CONNECTED) {
//		    	connector.disconnect();
			connector.login("my_name");
			status = 1;
		    }
		}
		else if(status == 1) {
		    connector.send(connector.CMD_L_L0_CLIENTS, "teotest");
		}
	    } catch (e) {
		console.log(e)
	    }
    },
    function() { // Finished
	console.log("FINISHED")
    },
    100
    )


//while(1) {
//    console.log("id connected:")
//}

/*
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
*/

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
