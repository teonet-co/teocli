/**
 * File:   teo_packet.h
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 3, 2016, 12:32 PM
 */

#pragma once


#include <stddef.h>
#include <sys/types.h>
#include "libteol0/teonet_l0_client.h"
#include <string>

#include <nan.h>


class TeoPacket: public Nan::ObjectWrap {

public:

    static NAN_MODULE_INIT(Init);

    static v8::Local<v8::Value> createNewInstance(const std::string& call_name, const std::string& text);

private:

    TeoPacket(){
	memset(&packet_, 0, sizeof(packet_));
    }

    TeoPacket(const std::string& call_name, const std::string& text){
	memset(&packet_, 0, sizeof(packet_));
    }

    void init(const teoLNullCPacket& packet) {
	packet_ = packet;
    }

    static NAN_METHOD(New); // constructor

    static Nan::Persistent<v8::Function> constructor;

private:

    teoLNullCPacket packet_;

};

