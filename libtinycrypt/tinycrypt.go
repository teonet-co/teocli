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

// PeerKeys ...
type PeerKeys C.PeerKeyset

// PublicKey ...
type PublicKey C.ECDHPubkey

// CommonSalt ...
type CommonSalt C.AES128_1_BLOCK

// New own keys
func New() (pk *PeerKeys) {
	pk = &PeerKeys{}
	C.initPeerKeys((*C.PeerKeyset)(pk))
	return
}

// Make keys
func (pk *PeerKeys) Make(remote *PublicKey, salt *CommonSalt) (err error) {
	if cstr := C.initApplyRemoteKey((*C.PeerKeyset)(pk),
		(*C.ECDHPubkey)(remote),
		(*C.AES128_1_BLOCK)(salt),
	); unsafe.Pointer(cstr) != C.NULL {
		err = errors.New(C.GoString(cstr))
	}
	return
}

// Crypt input data
func (pk *PeerKeys) Crypt(num uint32, data []byte) {
	C.XCrypt_AES128_1(&pk.sessionkey, C.uint32_t(num), (*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)))
}

// PubkeyLocal return local public key
func (pk *PeerKeys) PubkeyLocal() *PublicKey {
	ppk := ((*C.PeerKeyset)(pk))
	return (*PublicKey)(&ppk.pubkeylocal)
}

// SessionSalt return salt
func (pk *PeerKeys) SessionSalt() *CommonSalt {
	ppk := ((*C.PeerKeyset)(pk))
	return (*CommonSalt)(&ppk.sessionsalt)
}
