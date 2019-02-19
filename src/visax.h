#ifndef __VISAX_H_
#define __VISAX_H_

#include "visa.h"
#include "visatype.h"

#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


  class RigolSpectrum    {
  public:
    RigolSpectrum();
    ~RigolSpectrum();
  private:
    ViSession vi;
    ViSession rm;
    bool ConnectInstrument();
    bool  DisconnectInstrument();
    bool ReadInstrument(string strCmd, string &strOut);
    bool WriteInstrument(string strCmd);
      
  public:
    bool CapturePeak(string line, double &power, double &frequency);
    bool InitSpectrum(string centerfrequency, string span, string powerThreshold);
    bool InitSpectrum(string centerfrequency, string span, string powerThreshold, string yScale, string rbw, string vbw, string swt);
  };


  
#ifdef __cplusplus
}
#endif
#endif
    
