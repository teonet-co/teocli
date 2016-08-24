teonetClient = require("teonet-client")

teonetClient.init();

try {

    var connector = teonetClient.connect("127.0.0.1", 9000)
    var snd = connector.login("my_name");
    console.log("Login Sent:", snd)

    console.log("Is Connected:", connector.is_connected())

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

}
catch (err) {
    console.log(err)
}

teonetClient.cleanup();
