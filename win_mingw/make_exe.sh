rm libteocli.dll libteocli.a teocli.exe teocli_exe.zip

x86_64-w64-mingw32-gcc -o ./libteocli.dll ../libteol0/teonet_l0_client.c -DHAVE_MINGW -lws2_32 -shared -fPIC -Wl,--out-implib,libteocli.a
#x86_64-w64-mingw32-gcc -o ./teocli.exe ../libteol0/teonet_l0_client.c ../main.c -DHAVE_MINGW -lws2_32
x86_64-w64-mingw32-gcc -o ./teocli.exe ../main.c -DHAVE_MINGW libteocli.dll -lws2_32

zip teocli_exe README teocli.exe libteocli.dll

