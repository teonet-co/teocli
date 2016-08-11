/**
 * File:   errno_exeption.cpp
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#include "errno_exeption.h"
#include <string.h>

//https://gist.github.com/bellbind/a68620383e0180b3afc6

using namespace Nan;
using namespace v8;

Nan::Persistent<v8::Function> TeoErrnoExeption::constructor;

NAN_METHOD(TeoErrnoExeption::New) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
	std::vector<v8::Local<v8::Value>> args(info.Length());
	for (std::size_t i = 0; i < args.size(); ++i) args[i] = info[i];
	auto inst = Nan::NewInstance(info.Callee(), args.size(), args.data());
	if (!inst.IsEmpty()) info.GetReturnValue().Set(inst.ToLocalChecked());
	return;
    }

    auto object = new TeoErrnoExeption(info[0]->IntegerValue(), *Nan::Utf8String(info[1]), *Nan::Utf8String(info[2]));
    object->Wrap(info.This()); // `Wrap` bind C++ object to JS object.
}

NAN_MODULE_INIT(TeoErrnoExeption::Init) {
    Nan::HandleScope scope;
    auto cname = Nan::New("TeoErrnoExeption").ToLocalChecked();
    auto ctor = Nan::New<v8::FunctionTemplate>(New);
    auto ctorInst = ctor->InstanceTemplate(); // target for member functions
    ctor->SetClassName(cname); // as `ctor.name` in JS
    ctorInst->SetInternalFieldCount(1); // for ObjectWrap, it should set 1

    constructor.Reset(ctor->GetFunction());

    auto perrno(Nan::New("errno").ToLocalChecked());
    auto pcall_name(Nan::New("call_name").ToLocalChecked());
    auto ptext(Nan::New("text").ToLocalChecked());

    Nan::SetAccessor(ctorInst, perrno, ErrnoGet, ErrnoSet);
    Nan::SetAccessor(ctorInst, pcall_name, CallNameGet, CallNameSet);
    Nan::SetAccessor(ctorInst, ptext, TextGet, TextSet);

    Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());
}


NAN_GETTER(TeoErrnoExeption::ErrnoGet) {
    Nan::HandleScope scope;
    auto object(Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder()));
    auto val = Nan::New<Integer>(object->errno_);
    info.GetReturnValue().Set(val);
}

NAN_SETTER(TeoErrnoExeption::ErrnoSet) {
    Nan::HandleScope scope;
    auto object = Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder());
    object->errno_ = value->IntegerValue();
}

NAN_GETTER(TeoErrnoExeption::CallNameGet) {
    Nan::HandleScope scope;
    auto object(Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder()));
    info.GetReturnValue().Set(Nan::New(object->call_name_).ToLocalChecked());
}

NAN_SETTER(TeoErrnoExeption::CallNameSet) {
    Nan::HandleScope scope;
    auto object(Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder()));
    object->call_name_ = *Nan::Utf8String(Nan::To<v8::String>(value).ToLocalChecked());
}

NAN_GETTER(TeoErrnoExeption::TextGet) {
    Nan::HandleScope scope;
    auto object(Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder()));
    info.GetReturnValue().Set(Nan::New(object->text_).ToLocalChecked());
}

NAN_SETTER(TeoErrnoExeption::TextSet) {
    Nan::HandleScope scope;
    auto object(Nan::ObjectWrap::Unwrap<TeoErrnoExeption>(info.Holder()));
    object->text_ = *Nan::Utf8String(Nan::To<v8::String>(value).ToLocalChecked());
}

Local<Value> TeoErrnoExeption::createNewInstance(int errn, const std::string& call_name) {
    const int argc(3);
    char buf [4096];
    char* ptr = strerror_r(errn, buf, sizeof(buf));

    v8::Local<v8::Value> argv[argc] = {
	Nan::New<Integer>(errn),
	Nan::New<String>(call_name).ToLocalChecked(),
	Nan::New<String>(ptr).ToLocalChecked()
    };
    return Nan::New<v8::Function>(constructor)->NewInstance(argc, argv);
}
