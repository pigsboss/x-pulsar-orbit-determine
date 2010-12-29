#define G 6.67428e-11
#define C0 299792458.0
#define PI 3.141592653589793
#define SATELLITECONF "satellite.conf"
#define PULSARCONF "pulsar.conf"
#define MAXNUMPSR 256

struct SState {
  double m_dbTime;
/*
*   State vector of the satellite in m or m/s. 
*   state = {r_x, r_y, r_z, v_x, v_y, v_z}.
*   (1, 0, 0) is unit vector with RA = 0 and Dec = 0, 
*   (0, 1, 0) is unit vector with RA = 6 h and Dec = 0,
*   (0, 0, 1) is unit vector with RA = arbitrary value and Dec = 90 deg.
*/
  double m_rgdbState[6];
};

struct SPulse {
  double m_dbTOA; // Time of arrival.
  double m_rgdbRADec[2]; // R.A. and Dec of the source in radian.
};

struct SPulsar {
  double m_dbRA; // R.A. of pulsar, in hour.
  double m_dbDec; // Dec of pulsar, in degree.
  double m_rgdbDir[3]; // Direction vector of pulsar.
  double m_dbPeriod; // Period of pulsar.
/*
*   Offset of pulsar TOA (the first TOA to reference according to the model).
*/
  double m_dbOffset;
};

class CSatellite {
  private:
    double m_dbMC; // Mass of the central body in kg.
    double m_dbMS; // Mass of the satellite in kg.
    double m_dbStep; // Time step size of satellite Euler method in second.
    unsigned char m_u8NPsrs; // Number of pulsars.
    SState m_state;
    SPulsar * m_prgPsrRec;
  public:
    CSatellite();
    ~CSatellite();
    void simulate(double, unsigned long);
};
