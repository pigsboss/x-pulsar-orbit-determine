#define MAXNUMPSR 256

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
    double m_fpState[6];
    double m_dbStep; // Time step size of satellite Euler method in second.
    unsigned long long m_u64Clock; // Intrinsic clock. time = clock * step.
    unsigned char m_u32NumPsrs; // Number of pulsars.
    double m_fpRAPsr[MAXNUMPSR]; // RA of pulsar, in hour.
    double m_fpDecPsr[MAXNUMPSR]; // Dec of pulsar, in degree.
    double m_fpPPsr[MAXNUMPSR]; // Period of pulsar.
    double m_fpOffsetPsr[MAXNUMPSR]; // Offset of pulsar TOA (the first TOA according to the model).
  public:
    CSatellite();
    CSatellite(double *, double);
    void getState(double *);
    double getTime();
    void go(unsigned long long);
};
