PACKAGES := \
	github.com/eliasson/foo \
	github.com/eliasson/bar
DEPENDENCIES := github.com/eliasson/acme

all: teocli build

build:
	go build -o teochatcli
	rm libteocli.a

test:
	go test -v $(PACKAGES)

silent-test:
	go test $(PACKAGES)

format:
	go fmt $(PACKAGES)

deps:
	go get $(DEPENDENCIES)

teocli:
	gcc -c ../libteol0/teonet_l0_client.c
	ar rcs libteocli.a teonet_l0_client.o
	rm teonet_l0_client.o