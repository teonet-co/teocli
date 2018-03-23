/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>
#include <teonet>

#include "nbind/nbind.h"

class Ref {
  
private:  
  
  void *ptr;
  
public:
  
  void *getPtr() { return ptr; }
  void setPtr(void *p) { ptr = p; }
};

struct Teonet_light {

  static void hello(const char *name) {
    // Welcome message
    std::cout << name << "\n";
  }

  #define T_VERSION "0.0.1"

  static int start(std::vector<std::string> arg, nbind::cbFunction &cbnbind) {

    std::cout << "Teo example ver " T_VERSION ", "
                 "based on teonet ver. " VERSION "\n";
    
    // Vector to argc argv
    int i = 0, argc = arg.size();
    char** argv = (char**)malloc(argc * sizeof(char*));
    for(auto const& value: arg) {
      argv[i++] = strdup(value.c_str());
    }
    
    // Store callback
    static nbind::cbFunction &cb = cbnbind;

    // Application parameters
    const char *app_argv[] = { "", "teodb_peer"};
    teo::teoAppParam app_param = { 2, app_argv, NULL };

    // Initialize teonet event manager and Read configuration
    auto *teo = new teo::Teonet(argc, argv, [](teo::Teonet &teo, 
      teo::teoEvents event, void *data, size_t data_len, void *user_data) {
      
      std::vector<unsigned char> d(static_cast<char*>(data), static_cast<char*>(data) + data_len);
      cb.call<void>(teo, (int) event, d, data_len, 0);
      
    }, READ_OPTIONS|READ_CONFIGURATION|APP_PARAM, 0, &app_param);

    teo->getKe()->user_data = (void*)&cbnbind; 
    
    // Set application type
    teo->setAppType("teo-nbind-ex");
    teo->setAppVersion(T_VERSION);

    // Start teonet
    teo->run();

    delete teo;    
    
    // Free argv memory
    for(int i = 0; i < argc; i++) {
      free((void*)argv[i]);
    }
    free((void*)argv);

    return (EXIT_SUCCESS);
  }
  
  static int sendAnswerTo(teo::Teonet &teo) { //, teo::teoPacket &rd){//, const char *to) {//, const std::vector<unsigned char> &buf) {
    
    return 0;
  }

};

//NBIND_CLASS(Ref) {
//  construct<>();
//  getset(getPtr, setPtr);
//}

NBIND_CLASS(Teonet_light) {
  method(hello);
  method(start);
  method(sendAnswerTo);
}

using namespace teo;
NBIND_CLASS(Teonet) {
  method(sendAnswerTo);
}
