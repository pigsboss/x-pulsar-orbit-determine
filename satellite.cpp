#include "cblas.h"
#include "satellite.h"
#include <iostream>
using namespace std;
CSatellite::CSatellite() {
  m_fpState[0] = 0;
  m_fpState[1] = 1;
  m_fpState[2] = 0;
  m_fpState[3] = 1;
  m_fpState[4] = 0;
  m_fpState[5] = 0;
  m_dbStep = 1e-3;
  m_u64Clock = 0;
}
CSatellite::CSatellite(double * fpInitState, double dbStep) {
  cblas_dcopy(6, fpInitState, 1, m_fpState, 1);
  m_dbStep = dbStep;
  m_u64Clock = 0;
}
void CSatellite::getState(double * fpState) {
  cblas_dcopy(6, m_fpState, 1, fpState, 1);
}
double CSatellite::getTime() {
  return m_dbStep * double(m_u64Clock);
}
void CSatellite::go(unsigned long long u64Steps) {
  unsigned long long i;
  for(i=0; i<u64Steps; i++) {
    cblas_daxpy(3, m_dbStep, m_fpState+3, 1, m_fpState, 1);
  }
  m_u64Clock = m_u64Clock + u64Steps;
}

