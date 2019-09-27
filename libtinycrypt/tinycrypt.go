// Package tinycrypt used to tiny cript(decrypt) outgoung(incoming) data.
//
// See the packets test to understand client server encryption workflow
package tinycrypt

//// CGO definition (don't delay or edit it):
//#include "tinycrypt.h"
//#include "tiny-ECDH-c/ecdh.c"
//#include "tiny-AES-c/aes.c"
import "C"
import (
	"errors"
	"unsafe"
)

// Tcrypt is the tinycrypt receiver
type Tcrypt C.PeerKeyset

// binBuffer is structure used in Marshal binary buffer
type binBuffer struct {
	proto uint16
	key   C.ECDHPubkey
	salt  C.AES128_1_BLOCK
}

// New create and initialize Tcrypt packet receiver
func New() (pk *Tcrypt) {
	pk = &Tcrypt{}
	C.initPeerKeys((*C.PeerKeyset)(pk))
	return
}

// MarshalBinary marshal Tcrypt keys to binary buffer
func (pk *Tcrypt) MarshalBinary() (data []byte, err error) {
	nb := binBuffer{}
	l := unsafe.Sizeof(nb)
	nb.proto = 1
	nb.key = pk.pubkeylocal
	nb.salt = pk.sessionsalt
	data = (*[1 << 28]byte)(unsafe.Pointer(&nb))[:l:l]
	return
}

// UnmarshalBinary unmarshal keys from binary buffer and applay it on Tcrypt
func (pk *Tcrypt) UnmarshalBinary(data []byte) (err error) {
	var nb binBuffer
	if data == nil || len(data) == 0 {
		err = errors.New("input data is empty")
		return
	}
	if len(data) != int(unsafe.Sizeof(nb)) {
		err = errors.New("wrong size of input data")
		return
	}
	nbptr := (*binBuffer)(unsafe.Pointer(&data[0]))
	if cstr := C.initApplyRemoteKey((*C.PeerKeyset)(pk), &nbptr.key,
		&nbptr.salt); unsafe.Pointer(cstr) != C.NULL {
		err = errors.New(C.GoString(cstr))
	}
	return
}

// CryptInPlace encode (or decode) input data. The input data wiil be replaced with
// encrypted (or decrypted) data.
func (pk *Tcrypt) CryptInPlace(num uint32, data []byte) []byte {
	if data != nil && len(data) > 0 {
		C.XCrypt_AES128_1(&pk.sessionkey, C.uint32_t(num),
			(*C.uint8_t)(unsafe.Pointer(&data[0])), C.size_t(len(data)))
	}
	return data
}

// Crypt encoded (or decode) input data and return encrypted (or decrypted) data
// in created new data buffer
func (pk *Tcrypt) Crypt(num uint32, d []byte) (data []byte) {
	if d == nil {
		return nil
	}
	data = append([]byte{}, d...)
	pk.CryptInPlace(num, data)
	return
}
