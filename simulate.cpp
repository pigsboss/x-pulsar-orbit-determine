#include "satellite.h"
#include <iostream>
using namespace std;
int main() {
  CSatellite sat;
  sat.simulate(86400.0, 1000);
  return 0;
}
