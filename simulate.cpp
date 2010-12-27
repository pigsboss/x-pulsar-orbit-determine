#include "satellite.h"
#include <iostream>
using namespace std;
int main() {
  CSatellite sat;
  double fpState[6];
  unsigned long long i;
  for(i=1;i<100000;i++) {
    sat.go(100);
    sat.getState(fpState);
    cout << fpState[0] << ", " << fpState[1] << ", " << fpState[2] << ", " << fpState[3] << ", " << fpState[4] << ", " << fpState[5] << "." << endl;
  }
  return 0;
}
