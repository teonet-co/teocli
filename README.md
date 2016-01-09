# Teonet L0 client library

## Library folders structure description

### Sources folder

The folder ```libteol0``` contain source code of the teocli library


### Linux

The ```linux``` folder contain Makefile to make Linux shared library and 
application. See [README](linux/README) in that folder to additional information.


### Python

The ```python``` folder contain make_python.sh file to make python module. See 
[README](python/README) in that folder to additional information.


### Java Script and Node.js

The ```TeocliJS``` subproject content code to connect to Teonet with websocket 
from Java Script or Node.js. See the [TeocliJS](https://gitlab.ksproject.org/teonet/TeocliJS) 
project.


### Windows with MinGw-64

The ```win_mingw``` folder contain make_exe.sh file to make Windows DLL and 
tests application under Linux. See [README](win_mingw/README) in that folder to 
additional information.


### Windows with Visual Studio

The ```win_vcxproj``` folder contain solution and projects to make Windows DLL 
and tests under Windows Visual Studio. See [README](win_vcxproj/README) in that folder to 
additional information.


## Basic teocli example

### Test L0 client/server connection:
  
L0 Server: ```examples/teostream teo-str NULL NULL -p 9000 --l0_allow```  

Teonet Peer: ```examples/teostream teostream teo-str str -r 9000 -a 127.0.0.1```  

L0 client: ```examples/teol0cli C1 127.0.0.1 9000 teostream "Hello world!"```  
  or  
L0 client: ```examples/teol0cli C1 127.0.0.1 9000 teo-str "Hello world!"```  
  
### L0 Client packet structure:  
  
![l0-client-packet__1_](https://gitlab.ksproject.org/teonet/teonet/uploads/3db51c37f422e8dd91912a0de9122a0a/l0-client-packet__1_.png)