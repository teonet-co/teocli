/**
 * File:   teo_aux.h
 * Author: Alexander Ksenofontov <aksenofo@yahoo.ru>
 *
 * Created on August 5, 2016, 12:32 PM
 */


#pragma once

#include <string.h>

#define DECL_CONSTANT(NAME, VALUE)\
static NAN_GETTER(GETTER_##NAME) {\
    info.GetReturnValue().Set(VALUE);\
}\


#define IMPL_CONSTANT(TARGET,NAME) \
Nan::SetAccessor(TARGET, Nan::New(#NAME).ToLocalChecked(), GETTER_##NAME, nullptr);

#if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    #define close_socket(sd) closesocket(sd)
#else
    #define close_socket(sd) close(sd)
#endif

inline
void* clone_buffer(const void* ptr, const size_t& len) {
    auto rc(malloc(len));
    if(rc)
	memcpy(rc, ptr, len);
    return rc;
}