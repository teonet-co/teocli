### Build static library

	gcc -c ../libteol0/teonet_l0_client.c
	ar rcs libteocli.a teonet_l0_client.o
	
### Build application

	go build -o teochatcli

