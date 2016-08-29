/**
 * File:   connector.h
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#pragma once


#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <mutex>

#include "libteol0/teonet_l0_client.h"

#include <nan.h>
#include "teo_aux.h"

class Connector: public Nan::ObjectWrap {

public:

    class Worker: public Nan::AsyncProgressWorker {

    public:

	Worker(const std::string ip, int port,
               Nan::Callback *progress, Nan::Callback *finished, 
               int milliseconds,
	       v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> owner_);

	virtual ~Worker(){}

    private:

	void Execute (const Nan::AsyncProgressWorker::ExecutionProgress& progress);

	void HandleProgressCallback(const char *data, size_t size);

	void HandleOKCallback();

	std::string ip_;

	int port_;

	Nan::Callback *progress_;

	int milliseconds_;

	v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>> owner_;

	const Nan::AsyncProgressWorker::ExecutionProgress* exec_progress_;

	int errno_;

	virtual void event(void *con, teoLNullEvents event, void *data, size_t data_len);

	static void event_cb(void *con, teoLNullEvents event, void *data, size_t data_len, void *user_data) {
	    static_cast<Worker*>(user_data)->event(con, event, data, data_len);
	}

    };

public:

    Connector()
    : connector_(nullptr)
    , ip_()
    , port_(0)
    {}

    Connector(const std::string& ip, int port)
    : connector_(nullptr)
    , ip_(ip)
    , port_(port)
    {}

    virtual ~Connector();

    static v8::Local<v8::Value> createNewInstance(const Nan::FunctionCallbackInfo<v8::Value>& info);

    static v8::Local<v8::Value> createNewInstanceAsync(const Nan::FunctionCallbackInfo<v8::Value>& info);

    static NAN_MODULE_INIT(Init);

private:

    void attach_params(const std::string& ip, int port) {
	ip_ = ip;
	port_ = port;
    }

    void attach_connect(teoLNullConnectData* connector) {
	connector_ = connector;
    }

    static NAN_METHOD(New); // constructor

    static NAN_METHOD(Disconnect); // Disconnect

    static NAN_METHOD(Login); // Login method

    static NAN_METHOD(Send); // Send

    static NAN_METHOD(Recv); // Rec

    static NAN_METHOD(Sleep); // Sleep

    static NAN_METHOD(IsConnected);

    static Nan::Persistent<v8::Function> constructor;

    bool connect();

    bool is_connected() const {
	return connector_ && connector_->fd > 0;
    }

    void disconnect();

    ssize_t login(const char* host_name);

    ssize_t send(int cmd, const char *peer_name, const void *data, size_t data_length);

    ssize_t recv();

    size_t length() const {
	assert(connector_);
	return reinterpret_cast<teoLNullCPacket*>(connector_->read_buffer)->data_length;
    }

    const char* name() const {
	assert(connector_);
	return reinterpret_cast<teoLNullCPacket*>(connector_->read_buffer)->peer_name;
    }

    int cmd() const {
	assert(connector_);
	return reinterpret_cast<teoLNullCPacket*>(connector_->read_buffer)->cmd;
    }

    const void* arp_data() const {
	assert(connector_);
	auto cp(reinterpret_cast<teoLNullCPacket*>(connector_->read_buffer));
	return cp->peer_name + cp->peer_name_length;
    }

    v8::Local<v8::Value> fnCMD_L_PEERS_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info);

    v8::Local<v8::Value> fnCMD_L_L0_CLIENTS_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info);

    v8::Local<v8::Value> fnCMD_L_ECHO_ANSWER(const Nan::FunctionCallbackInfo<v8::Value>& info);

    //--------------------------------------------------------

    DECL_CONSTANT(CMD_L_ECHO, CMD_L_ECHO);
    DECL_CONSTANT(CMD_L_ECHO_ANSWER, CMD_L_ECHO_ANSWER);
    DECL_CONSTANT(CMD_L_PEERS, CMD_L_PEERS);
    DECL_CONSTANT(CMD_L_PEERS_ANSWER, CMD_L_PEERS_ANSWER);
    DECL_CONSTANT(CMD_L_AUTH, CMD_L_AUTH);
    DECL_CONSTANT(CMD_L_AUTH_ANSWER, CMD_L_AUTH_ANSWER);
    DECL_CONSTANT(CMD_L_L0_CLIENTS, CMD_L_L0_CLIENTS);
    DECL_CONSTANT(CMD_L_L0_CLIENTS_ANSWER, CMD_L_L0_CLIENTS_ANSWER);
    DECL_CONSTANT(CMD_L_SUBSCRIBE_ANSWER, CMD_L_SUBSCRIBE_ANSWER);
    DECL_CONSTANT(CMD_L_END, CMD_L_END);

    DECL_CONSTANT(EV_L_CONNECTED,EV_L_CONNECTED);	///< After connected to L0 server
    DECL_CONSTANT(EV_L_DISCONNECTED, EV_L_DISCONNECTED);///< After disconnected from L0 server
    DECL_CONSTANT(EV_L_RECEIVED, EV_L_RECEIVED);	///< Data received
    DECL_CONSTANT(EV_L_TICK, EV_L_TICK);		///< Send after every teoLNullReadEventLoop calls
    DECL_CONSTANT(EV_L_IDLE, EV_L_IDLE); 		///< Send after teoLNullReadEventLoop calls if data was not received during timeout.

private:

    teoLNullConnectData* connector_;

    std::string ip_;

    int port_;
};

