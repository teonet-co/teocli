'use strict';

const teonet = require('./teonet');
var nbind = require('nbind');
var lib = nbind.init().lib;
var Buffer = require('buffer').Buffer;

/**
 * This application API commands
 */
const teoApi = {
    /**
     * data: 'accessToken'
     */
    CMD_CHECK_USER: 129
};

lib.Teonet_light.hello('Hello!');
//lib.Teonet.start('teo-node,teo-auth', '0.0.21', 3, 5, teoEventCb);

lib.Teonet_light.start(process.argv.splice(1), function(teo, event, data, data_len, user_data) {

    //console.log('js event: ' + event);
    //let rd;
    
    switch(event) {
        
        // EV_K_STARTED
        case teonet.ev.EV_K_STARTED:
            console.log('EV_K_STARTED');
            break;    
            
        // EV_K_CONNECTED #3 New peer connected to host event
        case teonet.ev.EV_K_CONNECTED: {                
            let rd = new teonet.packetData(Buffer.from(data));
            console.log('EV_K_CONNECTED, peer "' + rd.from + '" connected');
        }   break;    
            
        // EV_K_IDLE
        case teonet.ev.EV_K_IDLE:
            console.log('EV_K_IDLE');
            break;    
            
        // EV_K_RECEIVED
        case teonet.ev.EV_K_RECEIVED: {
            let rd = new teonet.packetData(Buffer.from(data));
            console.log('EV_K_RECEIVED' + ', cmd: ' + rd.cmd + ', from' + rd.from, rd.data);
            
            switch(rd.cmd) {
                
                case 66: { //teoApi.CMD_CHECK_USER: {
                    //const buf = Buffer.from('Hello\0');
                    lib.Teonet_light.sendAnswerTo(teo);//, rd);//, rd.from);//, 'Hello\0');
                }   break;
                    
                default:
                    break
            }
            
        }   break;    
    }
});
