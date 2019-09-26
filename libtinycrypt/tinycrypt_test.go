package tinycrypt

import (
	"fmt"
	"testing"
)

func TestPacket(t *testing.T) {

	t.Run("New", func(t *testing.T) {
		pk := New()
		if pk == nil {
			t.Errorf("can't create tinycrypt receiver")
		}
	})

	t.Run("Make", func(t *testing.T) {
		pk1 := New()
		pk2 := New()

		pk1.Make(pk2.PubkeyLocal(), pk2.SessionSalt())
		pk2.Make(pk1.PubkeyLocal(), pk1.SessionSalt())

		word := []byte("Hello world!")
		pk1.Crypt(1, word)
		fmt.Printf("Crypted word: %v\n", word)

		pk2.Crypt(1, word)
		fmt.Printf("Decrypted word: %s\n", string(word))
	})

}
