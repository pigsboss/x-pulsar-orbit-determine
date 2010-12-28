#define MAXNUMPSR 256

struct SState {
  double m_dbTime;
  double m_fpState[6];
};

class CSatellite {
  private:
    double m_dbMC; // Mass of the central body in kg.
    double m_dbMS; // Mass of the satellite in kg.
/*
*   State vector of the satellite in m or m/s. 
*   state = {r_x, r_y, r_z, v_x, v_y, v_z}.
*   (1, 0, 0) is unit vector with RA = 0 and Dec = 0, 
*   (0, 1, 0) is unit vector with RA = 6 h and Dec = 0,
*   (0, 0, 1) is unit vector with RA = arbitrary value and Dec = 90 deg.
*/
    SState m_state;
    double m_dbStep; // Time step size of satellite Euler method in second.
    unsigned char m_u8NumPsrs; // Number of pulsars.
    double m_fpRAPsr[MAXNUMPSR]; // RA of pulsar, in hour.
    double m_fpDecPsr[MAXNUMPSR]; // Dec of pulsar, in degree.
    double m_fpPPsr[MAXNUMPSR]; // Period of pulsar.
/*
*   Offset of pulsar TOA (the first TOA to reference according to the model).
*/
    double m_fpOffsetPsr[MAXNUMPSR];
  public:
    CSatellite();
    void simulate(unsigned long, unsigned long long);
};
