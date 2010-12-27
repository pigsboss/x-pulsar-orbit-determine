#include "gsl/gsl_cblas.h"
#include "satellite.h"
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define G 6.67428e-11

CSatellite::CSatellite() {
  ifstream conf("satellite.conf");
  string line;
  size_t pos;
  if(conf.is_open()) {
    while(conf.good()) {
      getline(conf, line);
      pos = line.find("MASS_CENTRAL_BODY");
      if(pos != string::npos) {
        m_dbMC = atof((line.substr(pos+18)).c_str());
        cout << "Mass of the central body: " << m_dbMC << " kg" << endl;
      }
      pos = line.find("MASS_SATELLITE");
      if(pos != string::npos) {
        m_dbMS = atof((line.substr(pos+15)).c_str());
        cout << "Mass of the satellite: " << m_dbMS << " kg" << endl;
      }
      pos = line.find("RX");
      if(pos != string::npos) {
        m_fpState[0] = atof((line.substr(pos+3)).c_str());
        cout << "Initial r_x: " << m_fpState[0] << " m" << endl;
      }
      pos = line.find("RY");
      if(pos != string::npos) {
        m_fpState[1] = atof((line.substr(pos+3)).c_str());
        cout << "Initial r_y: " << m_fpState[1] << " m" << endl;
      }
      pos = line.find("RZ");
      if(pos != string::npos) {
        m_fpState[2] = atof((line.substr(pos+3)).c_str());
        cout << "Initial r_z: " << m_fpState[2] << " m" << endl;
      }
      pos = line.find("VX");
      if(pos != string::npos) {
        m_fpState[3] = atof((line.substr(pos+3)).c_str());
        cout << "Initial v_x: " << m_fpState[3] << " m/s" << endl;
      }
      pos = line.find("VY");
      if(pos != string::npos) {
        m_fpState[4] = atof((line.substr(pos+3)).c_str());
        cout << "Initial v_y: " << m_fpState[4] << " m/s" << endl;
      }
      pos = line.find("VZ");
      if(pos != string::npos) {
        m_fpState[5] = atof((line.substr(pos+3)).c_str());
        cout << "Initial v_z: " << m_fpState[5] << " m/s" << endl;
      }
      pos = line.find("TIMESTEPSIZE");
      if(pos != string::npos) {
        m_dbStep = atof((line.substr(pos+13)).c_str());
        cout << "Time step size: " << m_dbStep << " s" << endl;
      }
    }
  }else {
    cout << "Error: open satellite configuration file failed." << endl;
  }
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
  double fpAcc[3]; // Acceleration vector.
  double dbR; // Norm of position vector.
  for(i=0; i<u64Steps; i++) {
    dbR = cblas_dnrm2(3, m_fpState, 1);
    cblas_dscal(3, 0, fpAcc, 1);
    cblas_daxpy(3, (-1.0) * G * m_dbMC / (dbR*dbR*dbR), m_fpState, 1, fpAcc, 1);
    cblas_daxpy(3, m_dbStep, m_fpState+3, 1, m_fpState, 1);
    cblas_daxpy(3, m_dbStep, fpAcc, 1, m_fpState+3, 1);
  }
  m_u64Clock = m_u64Clock + u64Steps;
}