#include "satellite.h"
#include <cstdlib>
#include <iostream>
using namespace std;
int main(int argc, char * argv[]) {
  double dbSimTime;
  unsigned long u32Inc;
  bool bCmdIsGood = true;
  if(argc == 3) {
    dbSimTime = atof(argv[1]);
    u32Inc = (unsigned long)(atol(argv[2]));
    if(dbSimTime == 0.0 || u32Inc == 0) {
      bCmdIsGood = false;
    }
  }else {
    bCmdIsGood = false;
  }
  if(bCmdIsGood) {
    CSatellite sat;
    sat.simulate(dbSimTime, u32Inc);
  }else {
    cout << "Usage: \n";
    cout << "======\n";
    cout << "simulate T N\n";
    cout << "  T - End time of simulation.\n";
    cout << "  N - Stepsize is set to integral N times period of pulsars.\n";
  }
  return 0;
}
