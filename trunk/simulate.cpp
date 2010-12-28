#include "satellite.h"
#include <iostream>
using namespace std;
int main() {
  CSatellite sat;
  double fpState[6];
  unsigned long long i;
  for(i=0;i<100;i++) {
    sat.simulate(1,1);
    sat.getState(fpState);
    cout << sat.getTime() << ", " << fpState[0] << ", " << fpState[1] << ", " << fpState[2] << ", "
      << fpState[3] << ", " << fpState[4] << ", " << fpState[5] << endl;
  }
  return 0;
}
