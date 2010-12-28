#include "satellite.h"
#include <iostream>
using namespace std;
int main() {
  CSatellite sat;
  double fpState[6];
  unsigned long long i;
  sat.simulate(1,100000);
  return 0;
}
