/**.
 * File:   teo_main.cpp
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#include <stddef.h>
#include <sys/types.h>

#include "libteol0/teonet_l0_client.h"
#include "connector.h"
#include "errno_exeption.h"
#include "teo_exeption.h"
#include "teo_packet.h"
#include <iostream>
#include <nan.h>

//https://github.com/nodejs/nan/blob/master/doc/methods.md#api_nan_method

using namespace Nan;
using namespace v8;

NAN_METHOD(version) {
    info.GetReturnValue().Set(Nan::New<String>("0.0.1.1").ToLocalChecked());
}

NAN_METHOD(init) {
    teoLNullInit();
}

NAN_METHOD(cleanup) {
    teoLNullCleanup();
}

NAN_METHOD(connect) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(Connector::createNewInstance(info));
}

NAN_METHOD(connect_async) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(Connector::createNewInstanceAsync(info));
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, New<String>("version").ToLocalChecked(),
	GetFunction(New<FunctionTemplate>(version)).ToLocalChecked());
    Nan::Set(target, New<String>("init").ToLocalChecked(),
	GetFunction(New<FunctionTemplate>(init)).ToLocalChecked());
    Nan::Set(target, New<String>("cleanup").ToLocalChecked(),
	GetFunction(New<FunctionTemplate>(cleanup)).ToLocalChecked());
    Nan::Set(target, New<String>("connect").ToLocalChecked(),
	GetFunction(New<FunctionTemplate>(connect)).ToLocalChecked());
    Nan::Set(target, New<String>("connectAsync").ToLocalChecked(),
	GetFunction(New<FunctionTemplate>(connect_async)).ToLocalChecked());

    Connector::Init(target);
    TeoErrnoExeption::Init(target);
    TeoExeption::Init(target);
    TeoPacket::Init(target);
}

NODE_MODULE(teonet_client , Init)

