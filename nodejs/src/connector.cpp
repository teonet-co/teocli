/**
 * File:   connector.cpp
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#include "connector.h"
#include "errno_exeption.h"
#include "teo_exeption.h"
#include "teo_packet.h"
#include "teo_aux.h"
#include <sys/timeb.h>
#include <iostream>
#include <thread>

//https://gist.github.com/bellbind/a68620383e0180b3afc6

#define MAKE_THROW_IF_NOT_CONNECTED(METHOS_NAME) \
{\
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));\
    if(!This->is_connected())\
	return Nan::ThrowError(TeoExeption::createNewInstance(METHOS_NAME, "Connection was not been opened."));\
}

#define SET_PROP_STR(OBJ,NAME,VALUE) \
    OBJ->Set(Nan::New<String>(#NAME).ToLocalChecked(), Nan::New<String>(VALUE).ToLocalChecked());
#define SET_PROP_INT(OBJ,NAME,VALUE) \
    OBJ->Set(Nan::New<String>(#NAME).ToLocalChecked(), Nan::New<Integer>(VALUE));
#define SET_PROP_NUMB(OBJ,NAME,VALUE) \
    OBJ->Set(Nan::New<String>(#NAME).ToLocalChecked(), Nan::New<Number>(VALUE));
#define SET_PROP_OBJECT(OBJ,NAME,VALUE) \
    OBJ->Set(Nan::New<String>(#NAME).ToLocalChecked(), VALUE);

using namespace Nan;
using namespace v8;

Nan::Persistent<v8::Function> Connector::constructor;

Connector::~Connector() {
    if(connector_) // Do not call free. Use native disconnect
	free(connector_);
    connector_ = nullptr;
}

bool Connector::connect() {
    assert(!connector_);
    connector_ = teoLNullConnect(ip_.c_str(), port_);
    if(connector_ == nullptr || connector_->fd <= 0) {
	return false;
    }
    return true;
}

void Connector::disconnect() {
    assert(connector_);
    close_socket(connector_->fd);
    connector_->fd = -1;
}

ssize_t Connector::login(const char* host_name) {
    assert(connector_);
    return teoLNullLogin(connector_, host_name);
}

ssize_t Connector::send(int cmd, const char *peer_name, const void *data, size_t data_length) {
    assert(connector_);
    return teoLNullSend(connector_, cmd, peer_name, (void*)data, data_length);
}

ssize_t Connector::recv(const uint32_t& timeout) {
    assert(connector_);
    if(timeout == 0)
	return teoLNullRecv(connector_);
    else {
	uint32_t tmp(0);
	ssize_t rc(0);
	const uint32_t wait_period(50);
	while((rc = teoLNullRecv(connector_)) == -1 && tmp < timeout) {
	    teoLNullSleep(wait_period);
	    tmp += wait_period;
	}
	return rc;
    }
}

/**
 * Login to L0 server
 *.
 * Create and send L0 clients initialization packet
 *.
 * @param con Pointer to teoLNullConnectData
 * @param host_name Client name
 *.
 * @return Length of send data or -1 at error
 */
NAN_METHOD(Connector::Login) {
    Nan::HandleScope scope;
    if(info.Length() < 1 )
	Nan::ThrowError("Not enough parameters");
    MAKE_THROW_IF_NOT_CONNECTED(__FUNCTION__);

    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));
    size_t snd(This->login(*Nan::Utf8String(info[0])));
    if(snd == (size_t)-1)
        Nan::ThrowError(TeoErrnoExeption::createNewInstance(errno, __FUNCTION__));
    else
        info.GetReturnValue().Set((int)snd);
}

/**
 * Send command to L0 server
 *.
 * Create L0 clients packet and send it to L0 server
 *.
 * @param cmd Command
 * @param peer_name Peer name to send to
 * @param data String of data (optioonal)
 *.
 * @return Length of send data or -1 at error
 */
NAN_METHOD(Connector::SendAsString) {
    Nan::HandleScope scope;

    if(info.Length() < 2 )
	Nan::ThrowError("Not enough parameters");
    MAKE_THROW_IF_NOT_CONNECTED(__FUNCTION__);
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));

    auto cmd(info[0]->IntegerValue());
    auto peer_name(*Nan::Utf8String(info[1]));
    const char* buffer = nullptr;
    size_t buffer_len(0);
    if(info.Length() >= 3 ) {
	buffer = *Nan::Utf8String(info[2]);
	buffer_len = Nan::Utf8String(info[2]).length();
    }

    size_t snd(This->send(cmd, peer_name, buffer, buffer_len));

    if(snd == (size_t)-1)
        Nan::ThrowError(TeoErrnoExeption::createNewInstance(errno, __FUNCTION__));
    else
        info.GetReturnValue().Set((int)snd);
}

/**
 * Send command to L0 server
 *.
 * Create L0 clients packet and send it to L0 server
 *.
 * @param cmd Command
 * @param peer_name Peer name to send to
 * @param data Buffer(optioonal)
 *.
 * @return Length of send data or -1 at error
 */
NAN_METHOD(Connector::SendAsBuffer) {
    Nan::HandleScope scope;

    if(info.Length() < 2 )
	Nan::ThrowError("Not enough parameters");
    MAKE_THROW_IF_NOT_CONNECTED(__FUNCTION__);
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));

    auto cmd(info[0]->IntegerValue());
    auto peer_name(*Nan::Utf8String(info[1]));
    const char* buffer = nullptr;
    size_t buffer_len(0);
    if(info.Length() >= 3 ) {
	Local<Object> bufferObj = info[2]->ToObject();
	if(!bufferObj->IsUint8Array()) {
	    return Nan::ThrowError("Invalid parameter type. Must be Buffer.");
	}

	buffer = node::Buffer::Data(bufferObj);
	buffer_len = node::Buffer::Length(bufferObj);
    }
    
    size_t snd(This->send(cmd, peer_name, buffer, buffer_len));

    if(snd == (size_t)-1)
        Nan::ThrowError(TeoErrnoExeption::createNewInstance(errno, __FUNCTION__));
    else
        info.GetReturnValue().Set((int)snd);
}

v8::Local<v8::Value> Connector::fnCMD_BUFFER(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if(length() > 1) {
	return Nan::NewBuffer((char*)clone_buffer(payload_data(),
                              length()),
                              length()).ToLocalChecked();
    }
    else
	return Undefined();
}

Local<Value> Connector::fnCMD_L_ECHO_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if(length() > 1) {
	Local<Object> obj(Object::New(info.GetIsolate()));
	Local<Object> master(Object::New(info.GetIsolate()));
	master->Set(Nan::New<String>("CMD_L_ECHO_ANSWER").ToLocalChecked(), obj);
	return master;
    }
    return Undefined();
}

Local<Value> Connector::fnCMD_L_L0_CLIENTS_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if(length() > 1) {
	const teonet_client_data_ar *client_data_ar(static_cast<const teonet_client_data_ar*>(payload_data()));
	Local<Array> list = Array::New(info.GetIsolate());
	for(int i(0); i < (int)client_data_ar->length; i++) {
	    Local<Object> obj(Object::New(info.GetIsolate()));
	    SET_PROP_STR(obj, name, client_data_ar->client_data[i].name);
	    list->Set(i, obj);
	}
	Local<Object> master(Object::New(info.GetIsolate()));
	master->Set(Nan::New<String>("CMD_L_L0_CLIENTS_ANSWER").ToLocalChecked(), list);
	return master;
    }
    else
	return Undefined();
}

Local<Value> Connector::fnCMD_L_PEERS_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if(length() > 1) {
	const ksnet_arp_data_ar* arp_data_ar(static_cast<const ksnet_arp_data_ar*>(payload_data()));
	Local<Array> list = Array::New(info.GetIsolate());
	for(size_t i(0); i < (int)arp_data_ar->length; i++) {
	    Local<Object> obj(Object::New(info.GetIsolate()));
	    SET_PROP_STR(obj, name, arp_data_ar->arp_data[i].name);
	    SET_PROP_INT(obj, mode, arp_data_ar->arp_data[i].data.mode);
	    SET_PROP_STR(obj, addr, arp_data_ar->arp_data[i].data.addr);
	    SET_PROP_INT(obj, port, arp_data_ar->arp_data[i].data.port);
	    SET_PROP_NUMB(obj, last_activity, arp_data_ar->arp_data[i].data.last_activity);
	    SET_PROP_NUMB(obj, last_triptime_send, arp_data_ar->arp_data[i].data.last_triptime_send);
	    SET_PROP_NUMB(obj, last_triptime_got, arp_data_ar->arp_data[i].data.last_triptime_got);
	    SET_PROP_NUMB(obj, last_triptime, arp_data_ar->arp_data[i].data.last_triptime);
	    SET_PROP_NUMB(obj, triptime, arp_data_ar->arp_data[i].data.triptime);
	    SET_PROP_NUMB(obj, monitor_time, arp_data_ar->arp_data[i].data.monitor_time);
	    SET_PROP_NUMB(obj, connected_time, arp_data_ar->arp_data[i].data.connected_time);
	    list->Set(i, obj);
	}
	Local<Object> master(Object::New(info.GetIsolate()));
	master->Set(Nan::New<String>("CMD_L_PEERS_ANSWER").ToLocalChecked(), list);
	return master;
    }
    return Undefined();
}

NAN_METHOD(Connector::Disconnect) {
    Nan::HandleScope scope;
    MAKE_THROW_IF_NOT_CONNECTED(__FUNCTION__);
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));

    This->disconnect();
}

/**
 * Receive a packet
 *
 * Receive packet from server
 *
 * @param timeout(optional = 0 )
 *.
 * @return Known object of Buffer if unknown
 */
NAN_METHOD(Connector::Recv) {

    Nan::HandleScope scope;

    MAKE_THROW_IF_NOT_CONNECTED(__FUNCTION__);
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));
    uint32_t timeout(0);
    if(info.Length() > 0)
	timeout = info[0]->NumberValue() * 1000.0;

    ssize_t size(This->recv(timeout));
    if(size == -1)
        return (void)Nan::ThrowError(TeoErrnoExeption::createNewInstance(errno, __FUNCTION__));
    else {
	// Parse type
	switch(This->cmd()) {
	    case CMD_L_ECHO_ANSWER:
		info.GetReturnValue().Set(This->fnCMD_L_ECHO_ANSWER(info));
	    return;
	    case CMD_L_PEERS_ANSWER:
		info.GetReturnValue().Set(This->fnCMD_L_PEERS_ANSWER(info));
	    return;
	    case CMD_L_AUTH_ANSWER:
	    break;
	    case CMD_L_L0_CLIENTS_ANSWER:
		info.GetReturnValue().Set(This->fnCMD_L_L0_CLIENTS_ANSWER(info));
	    return;
	    default:
		info.GetReturnValue().Set(This->fnCMD_BUFFER(info));
	    return;
	}
        return info.GetReturnValue().Set(Undefined());
    }
}

NAN_METHOD(Connector::IsConnected) {
    Nan::HandleScope scope;
    auto This(Nan::ObjectWrap::Unwrap<Connector>(info.Holder()));
    bool rc(This->is_connected());
    return info.GetReturnValue().Set(rc);
}

NAN_METHOD(Connector::Sleep) {
    Nan::HandleScope scope;
    if(info.Length() < 1 )
	Nan::ThrowError("Not enough parameters");
    double seconds (info[0]->NumberValue());
    teoLNullSleep(seconds * 1000.0);
}

NAN_METHOD(Connector::New) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
	// [NOTE] generic recursive call with `new`
	std::vector<v8::Local<v8::Value>> args(info.Length());
	for (std::size_t i = 0; i < args.size(); ++i) args[i] = info[i];
	auto inst = Nan::NewInstance(info.Callee(), args.size(), args.data());
	if (!inst.IsEmpty()) info.GetReturnValue().Set(inst.ToLocalChecked());
	return;
    }

    // Create unconnected object and exit 
    if(info.Length() == 0 ) {
	auto object(new Connector);
	object->Wrap(info.This());
	return;
    }

    // Create object and connect it
    if(info.Length() < 2 )
	Nan::ThrowError("Not enough parameters");

    auto object(new Connector(*Nan::Utf8String(info[0]), info[1]->IntegerValue()));

    if(!object->connect()) {
	auto terrno = errno;
	delete object;
	Nan::ThrowError(TeoErrnoExeption::createNewInstance(terrno, __FUNCTION__));
    }
    else
	object->Wrap(info.This());
}

NAN_MODULE_INIT(Connector::Init) {
    Nan::HandleScope scope;
    auto cname = Nan::New("Connector").ToLocalChecked();
    auto ctor = Nan::New<v8::FunctionTemplate>(New);
    auto ctorInst = ctor->InstanceTemplate(); // target for member functions
    ctor->SetClassName(cname); // as `ctor.name` in JS
    ctorInst->SetInternalFieldCount(1); // for ObjectWrap, it should set 1

    // add member functions and accessors
    Nan::SetPrototypeMethod(ctor, "login", Login);
    Nan::SetPrototypeMethod(ctor, "send_as_string", SendAsString);
    Nan::SetPrototypeMethod(ctor, "send_as_buffer", SendAsBuffer);
    Nan::SetPrototypeMethod(ctor, "recv", Recv);
    Nan::SetPrototypeMethod(ctor, "sleep", Sleep);
    Nan::SetPrototypeMethod(ctor, "disconnect", Disconnect);
    Nan::SetPrototypeMethod(ctor, "is_connected", IsConnected);

    Nan::Set(target, cname, Nan::GetFunction(ctor).ToLocalChecked());

    IMPL_CONSTANT(ctorInst, CMD_L_ECHO);
    IMPL_CONSTANT(ctorInst, CMD_L_ECHO_ANSWER);
    IMPL_CONSTANT(ctorInst, CMD_L_PEERS);
    IMPL_CONSTANT(ctorInst, CMD_L_PEERS_ANSWER);
    IMPL_CONSTANT(ctorInst, CMD_L_AUTH);
    IMPL_CONSTANT(ctorInst, CMD_L_AUTH_ANSWER);
    IMPL_CONSTANT(ctorInst, CMD_L_L0_CLIENTS);
    IMPL_CONSTANT(ctorInst, CMD_L_L0_CLIENTS_ANSWER);
    IMPL_CONSTANT(ctorInst, CMD_L_SUBSCRIBE_ANSWER);
    IMPL_CONSTANT(ctorInst, CMD_L_END);

    IMPL_CONSTANT(ctorInst, EV_L_CONNECTED);
    IMPL_CONSTANT(ctorInst, EV_L_DISCONNECTED);
    IMPL_CONSTANT(ctorInst, EV_L_RECEIVED);
    IMPL_CONSTANT(ctorInst, EV_L_TICK);
    IMPL_CONSTANT(ctorInst, EV_L_IDLE);

    constructor.Reset(ctor->GetFunction());
}

v8::Local<v8::Value> Connector::createNewInstance(const Nan::FunctionCallbackInfo<v8::Value>& info) {

    v8::Local<v8::Value> argv[info.Length()];
    for( auto i(0); i < info.Length(); i++)
	argv[i] = info[i];

    return Nan::New<v8::Function>(constructor)->NewInstance(info.Length(), argv);
}

v8::Local<v8::Value> Connector::createNewInstanceAsync(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    auto object(Nan::New<v8::Function>(constructor)->NewInstance(0, {}));
    auto This(Nan::ObjectWrap::Unwrap<Connector>(object));
    auto ip(*Nan::Utf8String(info[0]));
    auto port(info[1]->IntegerValue());
    Callback *progress(new Callback(info[2].As<v8::Function>()));
    Callback *finished(new Callback(info[3].As<v8::Function>()));
    auto milliseconds(info[4]->IntegerValue());
    This->attach_params(ip, port);

    // Save "This" object inside worked to prevent an acting GC to this object.
    v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> pers_value(info.GetIsolate(), object);
    auto worker(new Worker(ip, port, progress, finished, milliseconds, pers_value));
    AsyncQueueWorker(worker);
    return object;
}

struct worker_payload {
    worker_payload()
    : data_(nullptr)
    , len_(0)
    , con_(nullptr) {}

    void* data_;
    size_t len_;
    teoLNullConnectData* con_;
    teoLNullEvents event_;
    int error_;
};

void Connector::Worker::event(void *con, teoLNullEvents event, void *data, size_t data_len) {
    worker_payload wp;

    if(data_len) {
	wp.data_ = malloc(data_len);
	assert(wp.data_);
	memcpy(wp.data_, data, data_len);
	wp.len_ = data_len;
    }

    wp.event_ = event;
    wp.error_ = 0;
    if(event == EV_L_CONNECTED) {
	wp.con_ = reinterpret_cast<teoLNullConnectData*>(con);
    }
    exec_progress_->Send(reinterpret_cast<const char*>(&wp), sizeof(wp));
}

Connector::Worker::Worker(const std::string ip, int port,
                          Nan::Callback *progress,
                          Nan::Callback *finished, int milliseconds,
			  v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> owner)
: AsyncProgressWorker(finished)
, ip_(ip)
, port_(port)
, progress_(progress)
, errno_(0)
, milliseconds_(milliseconds)
, owner_(owner)
, exec_progress_(nullptr) {
}

void Connector::Worker::Execute(const AsyncProgressWorker::ExecutionProgress& progress) {
	
    exec_progress_ = &progress;
    teoLNullConnectData *con(teoLNullConnectE(ip_.c_str(), port_, event_cb, this));

    if(con == nullptr || con->fd <= 0 ) {
	worker_payload wp;
	wp.event_ = EV_L_CONNECTED;
	wp.error_ = errno;
	exec_progress_->Send(reinterpret_cast<const char*>(&wp), sizeof(wp));
    }
    else {
	while(con && con->fd > 0 && teoLNullReadEventLoop(con, milliseconds_)) {
	}
	// save last errno to report it
	errno_ = errno;
    }

    exec_progress_ = nullptr;
}

void Connector::Worker::HandleOKCallback() {
    auto obj(Nan::New(owner_));
    auto This(Nan::ObjectWrap::Unwrap<Connector>(obj));
    v8::Local<v8::Value> argv[] = {Nan::New<v8::Integer>(errno_)};
    callback->Call(1, argv);
}

void Connector::Worker::HandleProgressCallback(const char *buf, size_t size) {
    const worker_payload* wp(reinterpret_cast<const worker_payload*>(buf));

    Nan::HandleScope scope;

    if(wp == nullptr)
	return;

    if(wp->con_) {
	// Finalize connecting process
	auto obj(Nan::New(owner_));
	auto This(Nan::ObjectWrap::Unwrap<Connector>(obj));
	This->attach_connect(wp->con_);
    }

    // generate object
    Local<Object> object(Object::New(Isolate::GetCurrent()));
    SET_PROP_INT(object, event, wp->event_);
    
    // Create a new buffer only if event == EV_L_RECEIVED
    if(wp->data_ && wp->len_ > 0 && wp->event_ == EV_L_RECEIVED) {
	teoLNullCPacket *cp = (teoLNullCPacket*) wp->data_;

	SET_PROP_INT(object, cmd, cp->cmd);
	SET_PROP_STR(object, peer_name, cp->peer_name);

	auto tail_ptr((char*)wp->data_ + cp->data_length);
	auto head_ptr((char*)cp->peer_name + cp->peer_name_length);

	size_t len(tail_ptr - head_ptr);
	char* b((char*)malloc(len));
	memcpy(b, head_ptr, len);
	auto mb(Nan::NewBuffer(b, len));

	SET_PROP_OBJECT(object, buffer, mb.ToLocalChecked());
    }
    v8::Local<v8::Value> argv[] = {
	object,
	Nan::New<v8::Integer>(wp->error_)
    };
    progress_->Call(2, argv);

    if(wp->data_)
	free(wp->data_);
}
