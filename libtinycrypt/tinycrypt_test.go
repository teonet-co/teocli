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

		ch := make(chan []byte)
		serverData := []byte("Hello world!")
		clientString := "Hello answer!"

		// In server
		server := func() {

			// Create servers Tcrypt object
			server := New()

			// Got public keys in binary buffer from client and apply it in
			// servers Tcrypt
			buf := <-ch
			err := server.UnmarshalBinary(buf)
			if err != nil {
				t.Error(err)
				return
			}

			// Send local public keys to client in binary buffer
			buf, err = server.MarshalBinary()
			if err != nil {
				t.Error(err)
				return
			}
			ch <- buf

			// Send encrypted data to client and receive encrypted answer
			for num := 0; num < 5; num++ {
				wordCrypted := server.Crypt(serverData)
				fmt.Printf("Crypted word: %v\n", wordCrypted)
				ch <- wordCrypted
				wordAnswer := <-ch
				server.CryptInPlace(wordAnswer)
				if string(wordAnswer) != clientString {
					t.Error("data encrypted on server not equal source data")
				}
				fmt.Printf("Decrypted word answer: %s\n", string(wordAnswer))
			}

			close(ch)
		}

		// In client
		client := func() {

			// Create clients Tcrypt object
			client := New()

			// Send local keys to server in binary buffer
			buf, err := client.MarshalBinary()
			if err != nil {
				t.Error(err)
				return
			}
			ch <- buf

			// Got public keys in binary buffer from server and apply it in
			// clients Tcrypt
			if err = client.UnmarshalBinary(<-ch); err != nil {
				t.Error(err)
				return
			}

			// God encrypted data from server and send encrypted answer
			num := 0
			for word := range ch {
				client.CryptInPlace(word)
				if string(word) != string(serverData) {
					t.Error("data encrypted on client not equal source data")
				}
				fmt.Printf("Decrypted word #%d: %s\n", num, string(word))
				ch <- client.CryptInPlace([]byte(clientString))
				num++
			}
		}

		go server()
		client()
	})
}
