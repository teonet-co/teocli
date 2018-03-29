To create make file (first time after clone this project) use:

    touch AUTHORS NEWS README ChangeLog
    ./autogen.sh

To make example application and Teonet L0 client library under Linux use command line:

    make

To run "teocli" example application use command line:

    ./teocli C5 xxx.xxx.xxx.xxx 9000 teostream "Hello world!"

where:

    C5 - client name
    xxx.xxx.xxx.xxx - IP address of Teonet L0 server to connect to
    9000 - servers port
    teostream - name of teonet application to send message to
    "Hello world!" - message to send

Build teocli shared library and example from command line:

    # MinGW
    gcc -o ./libteocli.so ../libteol0/teonet_l0_client.c -DHAVE_MINGW -shared -fPIC -Wl,--out-implib,libteocli.a
    gcc -o ./teocli ../main.c -DHAVE_MINGW libteocli.so
    gcc -o ./teocli_s ../main_select.c -DHAVE_MINGW libteocli.so

    # Linux
    gcc -o ./libteocli.so ../libteol0/teonet_l0_client.c -shared -fPIC
    gcc -o ./teocli_s ../main_select.c ./libteocli.so
    gcc -o ./teocli ../main.c ./libteocli.so
    sudo ldconfig

    # Look at win_vcxproj to use Visual studio projects 