
teonetClient = require('teonet-client');

teonetClient.init();

try {
    var counter = 0;
    var status = 0;
    var connector = teonetClient.connectAsync('127.0.0.1', 9000,
	function(ev, buf, err) { // Progress
	    try {
		if(ev !== connector.EV_L_TICK ) {
		    console.log(counter++, 'PROGRESS', ev, buf, err);
		    if(counter === 100)
			connector.disconnect();
		}
		if(status === 0) {
		    if(ev === connector.EV_L_CONNECTED) {
			connector.login('my_name');
			status = 1;
		    }
		}
		else if(status === 1) {
		    connector.send(connector.CMD_L_L0_CLIENTS, 'teotest');
		}
	    } catch (e) {
		console.log(counter, e);
	    }
      },
      function() { // Finished
	console.log('DISCONNECTED');
      },
      300
    );
}
catch (err) {
    console.log(err);
}

teonetClient.cleanup();
