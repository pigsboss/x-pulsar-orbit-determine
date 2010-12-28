#include "gsl/gsl_cblas.h"
#include "satellite.h"
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define G 6.67428e-11
#define SATELLITECONF "satellite.conf"
#define PULSARCONF "pulsar.conf"
#define SIMSTATE "simstate.out"
#define SIMTOA "simtoa.out"

CSatellite::CSatellite() {
/*
*  Parse satellite configuration file.
*/
  ifstream fSatellite(SATELLITECONF);
  string line;
  size_t pos;
  if(!fSatellite.is_open()) {
    cout << "Error: open satellite configuration file failed.\n";
    exit(1);
  }
  cout << "*\n";
  cout << "*** Satellite configuration ***\n";
  while(fSatellite.good()) {
    getline(fSatellite, line);
    pos = line.find("MASS_CENTRAL_BODY");
    if(pos != string::npos) {
      m_dbMC = atof((line.substr(pos+18)).c_str());
      cout << "Mass of the central body: " << m_dbMC << " kg\n";
    }
    pos = line.find("MASS_SATELLITE");
    if(pos != string::npos) {
      m_dbMS = atof((line.substr(pos+15)).c_str());
      cout << "Mass of the satellite: " << m_dbMS << " kg\n";
    }
    pos = line.find("RX");
    if(pos != string::npos) {
      m_fpState[0] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_x: " << m_fpState[0] << " m\n";
    }
    pos = line.find("RY");
    if(pos != string::npos) {
      m_fpState[1] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_y: " << m_fpState[1] << " m\n";
    }
    pos = line.find("RZ");
    if(pos != string::npos) {
      m_fpState[2] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_z: " << m_fpState[2] << " m\n";
    }
    pos = line.find("VX");
    if(pos != string::npos) {
      m_fpState[3] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_x: " << m_fpState[3] << " m/s\n";
    }
    pos = line.find("VY");
    if(pos != string::npos) {
      m_fpState[4] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_y: " << m_fpState[4] << " m/s\n";
    }
    pos = line.find("VZ");
    if(pos != string::npos) {
      m_fpState[5] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_z: " << m_fpState[5] << " m/s\n";
    }
    pos = line.find("TIMESTEPSIZE");
    if(pos != string::npos) {
      m_dbStep = atof((line.substr(pos+13)).c_str());
      cout << "Time step size: " << m_dbStep << " s\n";
    }
  }
  fSatellite.close();
  cout << "*\n";
  cout << "*** Pulsar configuration ***\n";
/*
*  Parse pulsar configuration file.
*/
  m_u8NumPsrs = 0;
  ifstream fPulsar(PULSARCONF);
  if(!fPulsar.is_open()) {
    cout << "Error: open pulsar configuration file failed.\n";
    exit(1);
  }
  while(fPulsar.good()) {
    getline(fPulsar, line);
    pos = line.find("PULSAR:");
    if(pos != string::npos) {
      m_fpRAPsr[m_u8NumPsrs] = atof((line.substr(pos+8)).c_str());
      pos = line.find(",", pos+1);
      m_fpDecPsr[m_u8NumPsrs] = atof((line.substr(pos+1)).c_str());
      pos = line.find(",", pos+1);
      m_fpPPsr[m_u8NumPsrs] = atof((line.substr(pos+1)).c_str());
      pos = line.find(",", pos+1);
      m_fpOffsetPsr[m_u8NumPsrs] = atof((line.substr(pos+1)).c_str());
      cout << "Pulsar " << m_u8NumPsrs + 1 << ": R.A. = "
        << m_fpRAPsr[m_u8NumPsrs] << " h, Dec = " << m_fpDecPsr[m_u8NumPsrs]
        << " deg, P = " << m_fpPPsr[m_u8NumPsrs] << " s, TOA_0 = "
        << m_fpOffsetPsr[m_u8NumPsrs] << " s.\n";
      m_u8NumPsrs = m_u8NumPsrs + 1;
    }
  }
  fPulsar.close();
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

/*
*  Simulates the dynamics and measurement of TOA.
*  num_records: state vector and TOA measurement records during simulation.
*  num_steps: simulated steps between every other record.
*  total simulated steps = num_steps * num_records.
*  Time of simulation = intrinsic_time_step_size * num_steps * num_records.
*/
void CSatellite::simulate(unsigned long u32NumSteps,
       unsigned long long u64NumRecords) {
  unsigned long long i;
  unsigned long j;
  double fpAcc[3]; // Acceleration vector.
  double dbR; // Norm of position vector.
  ofstream fState(SIMSTATE, ios_base::trunc);
  if(!fState.is_open()) {
    cout << "Error: open simulated satellite state records file failed.\n";
    exit(1);
  }
  fState << "Time, r_x, r_y, r_z, v_x, v_y, v_z\n";
  ofstream fTOA(SIMTOA, ios_base::trunc);
  if(!fTOA.is_open()) {
    cout << "Error: open simulated pulsars TOA records file failed.\n";
    exit(1);
  }
  for(i=0; i<u64NumRecords; i++) {
    for(j=0; j<u32NumSteps; j++) {
/*
*  Dynamics simulation:
*/
      dbR = cblas_dnrm2(3, m_fpState, 1);
      cblas_dscal(3, 0, fpAcc, 1);
      cblas_daxpy(3, (-1.0)*G*m_dbMC / (dbR*dbR*dbR), m_fpState, 1, fpAcc, 1);
      cblas_daxpy(3, m_dbStep, m_fpState+3, 1, m_fpState, 1);
      cblas_daxpy(3, m_dbStep, fpAcc, 1, m_fpState+3, 1);
      m_u64Clock = m_u64Clock + 1;
    }
    fState << m_dbStep * m_u64Clock << ", " << m_fpState[0] << ", "
      << m_fpState[1] << ", " << m_fpState[2] << ", " << m_fpState[3]
      << ", " << m_fpState[4] << ", " << m_fpState[5] << endl;
    
  }
  fState.close();
  fTOA.close();
}