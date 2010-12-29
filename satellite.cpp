#include "gsl/gsl_cblas.h"
#include "satellite.h"
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

#define G 6.67428e-11
#define PI 3.141592653589793
#define SATELLITECONF "satellite.conf"
#define PULSARCONF "pulsar.conf"

CSatellite::CSatellite() {
/*
*   Parse satellite configuration file.
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
      m_state.m_rgdbState[0] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_x: " << m_state.m_rgdbState[0] << " m\n";
    }
    pos = line.find("RY");
    if(pos != string::npos) {
      m_state.m_rgdbState[1] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_y: " << m_state.m_rgdbState[1] << " m\n";
    }
    pos = line.find("RZ");
    if(pos != string::npos) {
      m_state.m_rgdbState[2] = atof((line.substr(pos+3)).c_str());
      cout << "Initial r_z: " << m_state.m_rgdbState[2] << " m\n";
    }
    pos = line.find("VX");
    if(pos != string::npos) {
      m_state.m_rgdbState[3] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_x: " << m_state.m_rgdbState[3] << " m/s\n";
    }
    pos = line.find("VY");
    if(pos != string::npos) {
      m_state.m_rgdbState[4] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_y: " << m_state.m_rgdbState[4] << " m/s\n";
    }
    pos = line.find("VZ");
    if(pos != string::npos) {
      m_state.m_rgdbState[5] = atof((line.substr(pos+3)).c_str());
      cout << "Initial v_z: " << m_state.m_rgdbState[5] << " m/s\n";
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
*   Parse pulsar configuration file.
*/
  m_u8NPsrs = 0;
  double rgdbRA[MAXNUMPSR], rgdbDec[MAXNUMPSR];
  double rgdbPeriod[MAXNUMPSR], rgdbOffset[MAXNUMPSR];
  ifstream fPulsar(PULSARCONF);
  if(!fPulsar.is_open()) {
    cout << "Error: open pulsar configuration file failed.\n";
    exit(1);
  }
  while(fPulsar.good()) {
    getline(fPulsar, line);
    pos = line.find("PULSAR:");
    if(pos != string::npos) {
      rgdbRA[m_u8NPsrs] = atof((line.substr(pos+8)).c_str());
      pos = line.find(",", pos+1);
      rgdbDec[m_u8NPsrs] = atof((line.substr(pos+1)).c_str());
      pos = line.find(",", pos+1);
      rgdbPeriod[m_u8NPsrs] = atof((line.substr(pos+1)).c_str());
      pos = line.find(",", pos+1);
      rgdbOffset[m_u8NPsrs] = atof((line.substr(pos+1)).c_str());
      cout << "Pulsar " << m_u8NPsrs + 1 << ": R.A. = "
        << rgdbRA[m_u8NPsrs] << " h, Dec = " << rgdbDec[m_u8NPsrs]
        << " deg, P = " << rgdbPeriod[m_u8NPsrs]*1.0e3 << " ms, TOA_0 = "
        << rgdbOffset[m_u8NPsrs]*1.0e6 << " mus.\n";
      m_u8NPsrs = m_u8NPsrs + 1;
    }
  }
  m_prgPsrRec = new SPulsar[m_u8NPsrs];
  unsigned char k;
  for(k=0; k<m_u8NPsrs; k++) {
    (m_prgPsrRec[k]).m_dbRA = rgdbRA[k];
    (m_prgPsrRec[k]).m_dbDec = rgdbDec[k];
    (m_prgPsrRec[k]).m_dbPeriod = rgdbPeriod[k];
    (m_prgPsrRec[k]).m_dbOffset = rgdbOffset[k];
    (m_prgPsrRec[k]).m_rgdbDir[0] = cos(rgdbDec[k] * PI / 180.0) *
      cos(rgdbRA[k] * PI / 12.0);
    (m_prgPsrRec[k]).m_rgdbDir[1] = cos(rgdbDec[k] * PI / 180.0) *
      sin(rgdbRA[k] * PI / 12.0);
    (m_prgPsrRec[k]).m_rgdbDir[2] = sin(rgdbDec[k] * PI / 180.0);
    cout << (m_prgPsrRec[k]).m_rgdbDir[0] << ", " << (m_prgPsrRec[k]).m_rgdbDir[1] << ", " << (m_prgPsrRec[k]).m_rgdbDir[2] << endl;
  }
  fPulsar.close();
  m_state.m_dbTime = 0.0;
}

CSatellite::~CSatellite() {
  delete [] m_prgPsrRec;
}
/*
*   Simulates the dynamics and measurement of TOA.
*   num_records: state vector and TOA measurement records during simulation.
*   num_steps: simulated steps between every other record.
*   total simulated steps = num_steps * num_records.
*   Time of simulation = intrinsic_time_step_size * num_steps * num_records.
*/
void CSatellite::simulate(unsigned long u32NumSteps,
       unsigned long long u64NumRecords) {
  unsigned long long i, u64NumPhase;
  unsigned long j;
  double fpAcc[3]; // Acceleration vector.
  double dbR; // Norm of position vector.
  ofstream fStateText("simstate.out", ios::trunc);
  if(!fStateText.is_open()) {
    cout << "Error: open simulated satellite state records file failed.\n";
    exit(1);
  }
  ofstream fStateBin("simstate.dat", ios::trunc|ios::binary);
  if(!fStateBin.is_open()) {
    cout << "Error: open simulated satellite state records file failed.\n";
    exit(1);
  }
  fStateText.precision(15);
  fStateText << "Time, r_x, r_y, r_z, v_x, v_y, v_z\n";
  for(i=0; i<u64NumRecords; i++) {
    for(j=0; j<u32NumSteps; j++) {
/*
*
*   Dynamics simulation:
*/
      dbR = cblas_dnrm2(3, m_state.m_rgdbState, 1);
      cblas_dscal(3, 0, fpAcc, 1);
      cblas_daxpy(3, (-1.0)*G*m_dbMC /
        (dbR*dbR*dbR), m_state.m_rgdbState, 1, fpAcc, 1);
      cblas_daxpy(3, m_dbStep, m_state.m_rgdbState+3, 1, m_state.m_rgdbState, 1);
      cblas_daxpy(3, m_dbStep, fpAcc, 1, m_state.m_rgdbState+3, 1);
      m_state.m_dbTime = m_state.m_dbTime + m_dbStep;
    }
    fStateText << m_state.m_dbTime << ", " << m_state.m_rgdbState[0] << ", "
      << m_state.m_rgdbState[1] << ", " << m_state.m_rgdbState[2] << ", " 
      << m_state.m_rgdbState[3] << ", " << m_state.m_rgdbState[4] << ", "
      << m_state.m_rgdbState[5] << endl;
    fStateBin.write((char *)(&m_state), sizeof(SState));
  }
  fStateBin.close();
  fStateText.close();
/*
*
*   TOA measurements simulation:
*/
  unsigned char k;
  char pcFilename[32];
  double dbTOASSB;
  ofstream fTOASSBBin;
  ofstream fTOASSBText;
  ofstream fTOASatelliteBin;
  ofstream fTOASatelliteText;
  sprintf(pcFilename, "simtoasat%03d.dat", k);
  fTOASatelliteBin.open(pcFilename, ios::binary|ios::trunc);
  fTOASatelliteText << "TOA_SSB, R.A., Dec,\n";
  if(!fTOASatelliteBin.is_open()) {
    cout << "Error: open simulated pulse TOA at satellite file failed.\n";
    exit(1);
  }
  sprintf(pcFilename, "simtoasat%03d.out", k);
  fTOASatelliteText.open(pcFilename, ios::trunc);
  if(!fTOASatelliteText.is_open()) {
    cout << "Error: open simulated pulse TOA at satellite file failed.\n";
    exit(1);
  }
  fTOASatelliteText.precision(15);
  for(k=0; k < m_u8NPsrs; k++) {
    sprintf(pcFilename, "simtoassb%03d.dat", k);
    fTOASSBBin.open(pcFilename, ios::binary|ios::trunc);
    if(!fTOASSBBin.is_open()) {
      cout << "Error: open simulated pulse TOA at SSB file failed.\n";
      exit(1);
    }
    sprintf(pcFilename, "simtoassb%03d.out", k);
    fTOASSBText.open(pcFilename, ios::trunc);
    if(!fTOASSBText.is_open()) {
      cout << "Error: open simulated pulse TOA at SSB file failed.\n";
      exit(1);
    }
    fTOASSBText.precision(15);
    fTOASSBText << "TOA_SSB\n";
    for(dbTOASSB = (m_prgPsrRec[k]).m_dbOffset; dbTOASSB < m_state.m_dbTime; 
      dbTOASSB = dbTOASSB + (m_prgPsrRec[k]).m_dbPeriod) {
      fTOASSBBin.write((char *)(&dbTOASSB), sizeof(double));
      fTOASSBText << dbTOASSB << endl;
      
    }
    fTOASSBBin.close();
    fTOASSBText.close();
  }
  fTOASatelliteBin.close();
  fTOASatelliteText.close();
}
