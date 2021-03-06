/**
 * File:   teo_packet.cpp
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#include "teo_packet.h"
#include <string.h>

//https://gist.github.com/bellbind/a68620383e0180b3afc6

using namespace Nan;
using namespace v8;

Nan::Persistent<v8::Function> TeoPacket::constructor;

NAN_METHOD(TeoPacket::New) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
	std::vector<v8::Local<v8::Value>> args(info.Length());
	for (std::size_t i = 0; i < args.size(); ++i) args[i] = info[i];
	auto inst = Nan::NewInstance(info.Callee(), args.size(), args.data());
	if (!inst.IsEmpty()) info.GetReturnValue().Set(inst.ToLocalChecked());
	return;
    }

    auto object = new TeoPacket(*Nan::Utf8String(info[1]), *Nan::Utf8String(info[2]));
    object->Wrap(info.This()); // `Wrap` bind C++ object to JS object.
}

NAN_MODULE_INIT(TeoPacket::Init) {
    Nan::HandleScope scope;
    auto cname = Nan::New("TeoPacket").ToLocalChecked();
    auto ctor = Nan::New<v8::FunctionTemplate>(New);
    auto ctorInst = ctor->InstanceTemplate(); // target for member functions
    ctor->SetClassName(cname); // as `ctor.name` in JS
    ctorInst->SetInternalFieldCount(1); // for ObjectWrap, it should set 1

    Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());
    constructor.Reset(ctor->GetFunction());
}

Local<Value> TeoPacket::createNewInstance(const std::string& call_name, const std::string& text) {
    auto rc = Nan::New<v8::Function>(constructor)->NewInstance({}, 0);
    auto This(Nan::ObjectWrap::Unwrap<TeoPacket>(rc));
    return rc;
}
