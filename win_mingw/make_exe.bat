rm libteocli.dll teocli.exe

x86_64-w64-mingw32-gcc -o ./teocli.exe ../libteol0/teonet_l0_client.c ../main.c -DHAVE_MINGW -lws2_32
x86_64-w64-mingw32-gcc -o ./libteocli.dll ../libteol0/teonet_l0_client.c -DHAVE_MINGW -lws2_32 -shared -fPIC -Wl,--out-implib,libteocli.a