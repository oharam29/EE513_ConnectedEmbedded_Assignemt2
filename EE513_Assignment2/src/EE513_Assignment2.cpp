//============================================================================
// Name        : EE513_Assignment2.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ADXL345.h"
#include <unistd.h>
#include <pthread.h>

using namespace std;

int main() {
   ADXL345 sensor(1,0x53);
   sensor.setResolution(ADXL345::NORMAL);
   sensor.setRange(ADXL345::PLUSMINUS_4_G);
   sensor.displayPitchAndRoll();
   return 0;
}
