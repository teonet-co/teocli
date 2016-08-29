# Teonet client node.js module [![NPM version][npm-image]][npm-url]

Teonet client node.js module used in 
[generator-teonet-client](https://www.npmjs.com/package/generator-teonet-client),
[generator-teonet-client-webkit](https://www.npmjs.com/package/generator-teonet-client-webkit) 
to connect node.js client applications with Teonet. 


## Installation
```
npm install teonet-client
```

## Developer notes

### Use before build this package:

    sudo apt-get install build-essential

### To run projects examples in developer folder use:

    npm link teonet-client

[npm-image]: https://badge.fury.io/js/teonet-client.svg
[npm-url]: https://npmjs.org/package/teonet-client
[travis-image]: https://travis-ci.org//teonet-client.svg?branch=master
[travis-url]: https://travis-ci.org//teonet-client
[daviddm-image]: https://david-dm.org//teonet-client.svg?theme=shields.io
[daviddm-url]: https://david-dm.org//teonet-client
[coveralls-image]: https://coveralls.io/repos//teonet-client/badge.svg
[coveralls-url]: https://coveralls.io/r//teonet-client

### Usage

1. Export module
teonetClient = require('teonet-client');

2. initialize
teonetClient.init();

3. Establish a connection
var connector = 
    teonetClient.connectAsync(
	host_id,                      // host ip
	port,                         // port
	function(object, error_code) {// select-like method
	    ....................
	},
	function(error_code) {        // Connection lost 
	    ....................
	}
    )

where:
     error_code is a code of last operartion ( 0 - success  otherwise errno )
     object:
	obj.event  - event id
	obj.peer_name - peer name
	obj.cmd	- cmd ???
	obj.buffer  - binary buffer
