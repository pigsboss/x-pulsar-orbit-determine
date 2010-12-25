class CSatellite {
  private:
    double m_fpState[6]; // State vector of the satellite. state = {r_x, r_y, r_z, v_x, v_y, v_z}.
    double m_dbStep; // Time step size of satellite Euler method.
    unsigned long long m_u64Clock; // Intrinsic clock. time = clock * step.
  public:
    CSatellite();
    CSatellite(double *, double);
    void getState(double *);
    double getTime();
    void go(unsigned long long);
};
