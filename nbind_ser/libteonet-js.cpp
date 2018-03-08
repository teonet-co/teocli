/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

//#include <teonet>

#include <iostream>
#include "nbind/nbind.h"

struct Teonet {
  
  static int hello(const char *name) {
    // Welcome message
    std::cout << name << "\n";
  }
};

NBIND_CLASS(Teonet) {
  method(hello);
}