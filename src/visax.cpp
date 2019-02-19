#include "visax.h"

#include <windows.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <sstream>

using namespace std;

RigolSpectrum::RigolSpectrum() {
  vi = NULL;
  rm = NULL;
  ConnectInstrument();
}
RigolSpectrum::~RigolSpectrum() {
  DisconnectInstrument();
  vi = NULL;
  rm = NULL;
}

bool RigolSpectrum::ConnectInstrument() {
  // Open a default session
  ViStatus status = viOpenDefaultRM(&rm);
  if (status < VI_SUCCESS) {
    cout << "viOpenDefaultRM" << endl;
    return false;
  }

  ViChar desc[256];
  ViFindList list;
  ViUInt32 itemCnt;  
  // Find all USB devices
  status = viFindRsrc(rm, "USB?*INSTR", &list, &itemCnt,desc);
  if (status < VI_SUCCESS || itemCnt < 0) {
    cout << "viFindRsrc" << endl;
    viClose(rm);
    rm = VI_NULL;
    return false;
  }

  // Open the GPIB device at primary address 1, GPIB board 8
  //status = viOpen(rm, "GPIB8::1::INSTR", VI_NULL, VI_NULL, &vi);
  status = viOpen(rm, desc, VI_NULL, VI_NULL, &vi);  
  if (status < VI_SUCCESS) {
    cout << "viOpen" << endl;
    viClose(rm);
    rm = VI_NULL;
    return false;
  }

  #if 0
  ViUInt32 retCnt;  
  // Send an ID query.
  status = viWrite(vi, (ViBuf) "*idn?", 5, &retCnt);
  if (status < VI_SUCCESS) {
    cout << "viWrite" << endl;
    viClose(rm);
    viClose(vi);
    rm = VI_NULL;        
    vi = VI_NULL;
    return false;
  }

  ViChar id[256];
  // Clear the buffer and read the response
  memset(id, 0, sizeof(id));
  status = viRead(vi, (ViBuf) id, sizeof(id), &retCnt);
  if (status < VI_SUCCESS) {
    cout << "viRead" << endl;
    viClose(rm);
    viClose(vi);
    rm = VI_NULL;        
    vi = VI_NULL;    
    return false;    
  }
  cout << id << endl;
  #endif

  return true;
}


bool RigolSpectrum::DisconnectInstrument()  {
  if (vi != VI_NULL) {
    viClose(vi);
    vi = VI_NULL;
  }
  if (rm != VI_NULL) {
    viClose(rm);
    rm = VI_NULL;
  }
  return true;
}

bool RigolSpectrum::ReadInstrument(string strCmd, string &strOut) {
  ViUInt32 retCnt = 0;
  ViStatus status = viWrite(vi, (ViBuf)strCmd.c_str(), strCmd.length(),&retCnt);
  if (status < VI_SUCCESS) {
    cout << "viRead - viWrite" << endl;
    return false;
  }
  
  ViChar buf[256];
  memset(buf, 0, sizeof(buf));
  status = viRead(vi, (ViBuf)buf, sizeof(buf), &retCnt);
  if (status < VI_SUCCESS) {
    cout << "viRead Error: " << hex << status << endl;
    return false;
  }
  
  strOut = buf;

  //cout << strOut << endl;
  return true;
}
bool RigolSpectrum::WriteInstrument(string strCmd) {
  ViUInt32 retCnt;
  ViStatus status = viWrite(vi, (ViBuf)strCmd.c_str(), strCmd.length(),&retCnt);
  if (status < VI_SUCCESS) {
    cout << "viWrite" << endl;
    return false;
  }
  return true;
}

double sciToDub(const string & str) {
  stringstream ss(str);
  double d = 0;
  ss >> d;
  
  if (ss.fail()) {
    return d;
  }
  return(d);
}


bool RigolSpectrum::CapturePeak(string line, double &power, double &frequency) {
  if (rm == VI_NULL || vi == VI_NULL) {
    cout << "rm == VI_NULL, vi == VI_NULL" << endl;
    return false;
  }
  
  string cmd = string(":CALCulate:MARKer") + line + ":MAXimum:MAX";
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;
    return false;
  }

  string res;
  cmd = string(":CALCulate:MARKer") + line + ":Y?";
  if (!ReadInstrument(cmd, res)) {
    cout << cmd << " error" << endl;
    return false;
  }
  power = sciToDub(res);


  cmd = string(":CALCulate:MARKer") + line + ":X?";
  if (!ReadInstrument(cmd, res)) {
    cout << cmd << " error" << endl;
    return false;    
  }
  frequency = sciToDub(res);

  return true;
}


bool RigolSpectrum::InitSpectrum(string centerfrequency, string span, string powerThreshold) {
  if (rm == VI_NULL || vi == VI_NULL) {
    cout << "rm == VI_NULL, vi == VI_NULL" << endl;
    return false;
  }
  
  string cmd = string(":FREQ:CENT ") + centerfrequency;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;
    return false;
  }

  cmd = string(":FREQ:SPAN ") + span;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;
    return false;
  }
    
  cmd = string(":DISP:WIN:TRAC:Y:SCAL:RLEV ") + powerThreshold;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;    
    return false;
  }

  cmd = string(":TRACe:MATH:PEAK:SORT AMPLitude");
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }
  
  return true;
}
bool RigolSpectrum::InitSpectrum(string centerfrequency, string span, string powerThreshold, string yScale, string rbw, string vbw, string swt) {
  if (rm == VI_NULL || vi == VI_NULL) {
    cout << "rm == VI_NULL, vi == VI_NULL" << endl;
    return false;
  }
  
  string cmd = string(":FREQ:CENT ") + centerfrequency;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;    
    return false;
  }

  cmd = string(":FREQ:SPAN ") + span;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;  
    return false;
  }

  cmd = string(":DISP:WIN:TRAC:Y:SCAL:RLEV ") + powerThreshold;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;      
    return false;
  }

  cmd = string("DISPlay:WINdow:TRACe:Y:SCALe:PDIVision ") + yScale;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;    
    return false;
  }

  /*
  cmd = ":BANDwidth:RESolution:AUTO OFF";
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;        
    return false;
  }
  */

  cmd = string(":BANDwidth:RESolution ") + rbw;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;        
    return false;
  }

  /*
  cmd = ":BANDwidth:VIDeo:AUTO OFF";
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }
  */

  cmd = string(":BANDwidth:VIDeo ") + vbw;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }

  /*
  cmd = ":SWEep:TIME:AUTO OFF";
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }
  */

  cmd = string(":SWEep:TIME ") + swt;
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }

  cmd = string("TRACe:MATH:PEAK:SORT AMPLitude");
  if (!WriteInstrument(cmd)) {
    cout << cmd << " error" << endl;          
    return false;
  }            
  
  return true;
}

