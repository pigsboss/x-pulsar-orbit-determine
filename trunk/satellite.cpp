#include "satellite.h"
#include "gsl/gsl_cblas.h"
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

/*
*
*   Constructor:
*/
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
  }
  fPulsar.close();
  m_state.m_dbTime = 0.0;
}
/*
*
*   Destructor:
*/
CSatellite::~CSatellite() {
  delete [] m_prgPsrRec;
}
/*
*
*   Simulates the dynamics and measurement of TOA.
*/
void CSatellite::simulate(double dbSimTime, unsigned long u32Inc) {
  char szFilename[32];
  unsigned char k; // Index of pulsars.
  double fpAcc[3]; // Acceleration vector.
  double dbR; // Norm of position vector.
  double dbTOASSB; // TOA at SSB.
  double dbDist; // Distance between SSB and satellite along the direction
                 // to the pulsar.
  double dbTOASat; // TOA at satellite.
  double dbLenPulse; // Length of pulse. L = c_0 * period.
  double dbNPulses; // Number of integral pulses between SSB and satellite.
  double dbFrac; // Length of fraction of pulse (round to the nearest
                 // integral pulse).
  double rgdbState[6]; // Initial state of satellite.
  ofstream fStateBin;
  ofstream fStateText;
  ofstream fTOASSBBin;
  ofstream fTOASSBText;
  ofstream fTOASatBin;
  ofstream fTOASatText;
  cblas_dcopy(6, m_state.m_rgdbState, 1, rgdbState, 1); // Save initial state.
  for(k=0; k < m_u8NPsrs; k++) {
    sprintf(szFilename, "simstate%03d.dat", k);
    fStateBin.open(szFilename, ios::trunc|ios::binary);
    if(!fStateBin.is_open()) {
      cout << "Error: open simulated satellite state records file failed.\n";
      exit(1);
    }
    sprintf(szFilename, "simstate%03d.out", k);
    fStateText.open(szFilename, ios::trunc);
    if(!fStateText.is_open()) {
      cout << "Error: open simulated satellite state records file failed.\n";
      exit(1);
    }
    fStateText.precision(15);
    fStateText << "Time, r_x, r_y, r_z, v_x, v_y, v_z\n";
    sprintf(szFilename, "simtoassb%03d.dat", k);
    fTOASSBBin.open(szFilename, ios::binary|ios::trunc);
    if(!fTOASSBBin.is_open()) {
      cout << "Error: open simulated pulse TOA at SSB file failed.\n";
      exit(1);
    }
    sprintf(szFilename, "simtoassb%03d.out", k);
    fTOASSBText.open(szFilename, ios::trunc);
    if(!fTOASSBText.is_open()) {
      cout << "Error: open simulated pulse TOA at SSB file failed.\n";
      exit(1);
    }
    fTOASSBText.precision(15);
    fTOASSBText << "TOA@SSB\n";
    sprintf(szFilename, "simtoasat%03d.dat", k);
    fTOASatBin.open(szFilename, ios::binary|ios::trunc);
    if(!fTOASatBin.is_open()) {
      cout << "Error: open simulated pulse TOA at satellite file failed.\n";
      exit(1);
    }
    sprintf(szFilename, "simtoasat%03d.out", k);
    fTOASatText.open(szFilename, ios::trunc);
    if(!fTOASatText.is_open()) {
      cout << "Error: open simulated pulse TOA at satellite file failed.\n";
      exit(1);
    }
    fTOASatText.precision(15);
    fTOASatText << "TOA@Sat\n";
    dbLenPulse = C0 * (m_prgPsrRec[k]).m_dbPeriod;
    cblas_dcopy(6, rgdbState, 1, m_state.m_rgdbState, 1); // Load saved initial
                                                          // state.
    m_state.m_dbTime = 0; // Reset satellite time.
    for(dbTOASSB = (m_prgPsrRec[k]).m_dbOffset; dbTOASSB < dbSimTime; 
      dbTOASSB = dbTOASSB + u32Inc * (m_prgPsrRec[k]).m_dbPeriod) {
/*
*   TOA at SSB simulation:
*/
      fTOASSBBin.write((char *)(&dbTOASSB), sizeof(double));
      fTOASSBText << dbTOASSB << endl;
/*
*   Orbital dynamics simulation (satellite state update):
*/
      dbR = cblas_dnrm2(3, m_state.m_rgdbState, 1); // Calculate norm of
                                                    // (r_x, r_y, r_z).
      cblas_dscal(3, 0, fpAcc, 1); // Set acceleration to 0.
      cblas_daxpy(3, (-1.0)*G*m_dbMC /
        (dbR*dbR*dbR), m_state.m_rgdbState, 1, fpAcc, 1); // Calculate two-body
                                                          // acceleration.
      cblas_daxpy(3, dbTOASSB - m_state.m_dbTime, m_state.m_rgdbState+3, 1,
        m_state.m_rgdbState, 1); // Update position.
      cblas_daxpy(3, dbTOASSB - m_state.m_dbTime, fpAcc, 1,
        m_state.m_rgdbState+3, 1); // Update velocity.
      m_state.m_dbTime = dbTOASSB; // Update time.
      fStateText << m_state.m_dbTime << ", " << m_state.m_rgdbState[0] << ", "
        << m_state.m_rgdbState[1] << ", " << m_state.m_rgdbState[2] << ", " 
        << m_state.m_rgdbState[3] << ", " << m_state.m_rgdbState[4] << ", "
        << m_state.m_rgdbState[5] << endl;
      fStateBin.write((char *)(&m_state), sizeof(SState));
/*
*   TOA at satellite simulation:
*/
      dbDist = cblas_ddot(3, (m_prgPsrRec[k]).m_rgdbDir, 1,
        m_state.m_rgdbState, 1); 
      dbFrac = modf(dbDist/dbLenPulse, &dbNPulses);
      if(dbFrac >= 0.5) {
        dbTOASat = dbTOASSB - (1.0 - dbFrac) * (m_prgPsrRec[k]).m_dbPeriod;
      }else if(dbFrac >= 0.0) {
        dbTOASat = dbTOASSB + dbFrac * (m_prgPsrRec[k]).m_dbPeriod;
      }else if(dbFrac >= -0.5) {
        dbTOASat = dbTOASSB - (1.0 + dbFrac) * (m_prgPsrRec[k]).m_dbPeriod;
      }else {
        dbTOASat = dbTOASSB - dbFrac * (m_prgPsrRec[k]).m_dbPeriod;
      }
      fTOASatBin.write((char *)(&dbTOASat), sizeof(double));
      fTOASatText << dbTOASat << endl;
    }
    fStateBin.close();
    fStateText.close();
    fTOASSBBin.close();
    fTOASSBText.close();
    fTOASatBin.close();
    fTOASatText.close();
  }
}
