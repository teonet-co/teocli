teonetClient = require('teonet-client');

teonetClient.init();

try {

    var connector = teonetClient.connect('18.196.250.18', 9010);
    var snd = connector.login('C5');

    rc = connector.recv(1.1);
    console.log(rc.toString());

    console.log('Login Sent:', snd);

    console.log('Is Connected:', connector.is_connected());

    snd = connector.send_as_buffer(connector.CMD_L_PEERS, 'teo-db');
    console.log('CMD_L_PEERS Sent:', snd);

    snd = connector.send_as_buffer(connector.CMD_L_L0_CLIENTS, 'teo-db');
    console.log('CMD_L_L0_CLIENTS Sent:', snd);

    snd = connector.send_as_string(connector.CMD_L_ECHO, 'teo-db', 'hello');
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
