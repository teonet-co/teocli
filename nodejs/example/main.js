teonetClient = require('teonet-client');

teonetClient.init();

//teocli C5 gt1.kekalan.net 9010 ps-server Hello
//teocli C5 gt1.kekalan.net 9010 teo-gbs Hello
//teocli C5 gt1.kekalan.net 9010 teonet-11 Hello
//teocli C5 gt1.kekalan.net 9010 teonet-17 Hello

try {

//    var connector = teonetClient.connect('127.0.0.1', 9000);
    var connector = teonetClient.connect('gt1.kekalan.net', 9010);
    var snd = connector.login('C5');

    rc = connector.recv(1.1);
    console.log(rc.toString())

    console.log('Login Sent:', snd);

    console.log('Is Connected:', connector.is_connected());

    snd = connector.send_as_buffer(connector.CMD_L_PEERS, 'teonet-17');
    console.log('CMD_L_PEERS Sent:', snd);

    snd = connector.send_as_buffer(connector.CMD_L_L0_CLIENTS, 'teonet-17');
    console.log('CMD_L_L0_CLIENTS Sent:', snd);

    snd = connector.send_as_string(connector.CMD_L_ECHO, 'teonet-17', 'hello');
    console.log('CMD_L_ECHO Sent:', snd);

    connector.sleep(.5);
    console.log('-------------------------------------------------------');
    console.log('Recv:', connector.recv());

    console.log('-------------------------------------------------------');
    console.log('Recv:', connector.recv());

//    console.log('-------------------------------------------------------');
//    console.log('Recv:', connector.recv());
    connector.disconnect();
//    connector.disconnect();
//    snd = connector.send(connector.CMD_L_ECHO, 'teotest', 'hello');

}
catch (err) {
    console.log(err);
}

teonetClient.cleanup();
