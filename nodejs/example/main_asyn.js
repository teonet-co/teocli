
teonetClient = require('teonet-client');

teonetClient.init();

try {
    var counter = 0;
    var status = 0;
    var connector = teonetClient.connectAsync('127.0.0.1', 9000,
	function(obj, err) { // Progress
	    try {
		if(obj.event !== connector.EV_L_TICK ) {
		    console.log(counter++, 'PROGRESS', obj.event, obj.peer_name, obj.cmd, obj.buffer, err);
		    if(counter === 100)
			connector.disconnect();
		}
		if(status === 0) {
		    if(obj.event === connector.EV_L_CONNECTED) {
			connector.login('my_name');
			status = 1;
		    }
		}
		else if(status === 1) {
		    connector.send_as_buffer(connector.CMD_L_L0_CLIENTS, 'teotest');
		}
	    } catch (e) {
		console.log(counter, e);
	    }
      },
      function(error) { // Finished
	console.log('DISCONNECTED', error);
      },
      300
    );
}
catch (err) {
    console.log(err);
}

teonetClient.cleanup();
