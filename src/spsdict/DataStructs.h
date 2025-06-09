/*DataStructs.h
 * Data structures for analysis. To be implemented as a dictionary for ROOT in LinkDef
 * Based on: FocalPlane_SABRE.h
 * Gordon M. Oct. 2019
 * 
 * Refurbished and modified 2024/2025 by J. Esparza to include CATRiNA detectors and 
 * new diagnostics for the focal plane.
 * 
 */
#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <vector>

struct DPPChannel 
{
  double Timestamp;
  int Channel, Board, Energy, EnergyShort;
  int Flags;
};

struct DetectorHit 
{
  double Long=-1, Short=-1, Time=-1;
  int Ch=-1;
};

// JCE 2025
struct CATRINADetector 
{
  std::vector<DetectorHit> catr;
};


struct SabreDetector 
{
  std::vector<DetectorHit> rings;
  std::vector<DetectorHit> wedges;
};


struct FPDetector 
{
  std::vector<DetectorHit> delayFL, delayFR, delayBL, delayBR;
  std::vector<DetectorHit> anodeF, anodeB, scintL, scintR, cathode;
  std::vector<DetectorHit> monitor;
};

struct CoincEvent 
{
  FPDetector focalPlane;
  SabreDetector sabreArray[5]; //index = ChannelMap Id# -1
  CATRINADetector catrinaArray[7]; //index = ChannelMap Id# -1, JCE 2025
};

struct ProcessedEvent 
{
  double fp1_tdiff = -1e6, fp2_tdiff = -1e6, fp1_tsum = -1, fp2_tsum = -1,
           fp1_tcheck = -1, fp2_tcheck = -1;

  double fp1FL_tdiff_anodeFront = -1e6, fp1FR_tdiff_anodeFront = -1e6; // DL/anode time differences JCE
  double fp2BL_tdiff_anodeBack = -1e6, fp2BR_tdiff_anodeBack = -1e6; // JCE 2025

  double fp1FL_tdiff_tilde = -1e6, fp1FR_tdiff_tilde = -1e6; // building X1 & X2 with only half the DL time
  double fp2BL_tdiff_tilde = -1e6, fp2BR_tdiff_tilde = -1e6;

  double fp1_tsum_FL = -1, fp1_tsum_FR = -1; // time sum for left/right side of X1
  double fp2_tsum_BL = -1, fp2_tsum_BR = -1; // time sum for left/right side of X2
  double fp1_tsumA =-1, fp2_tsumB = -1; // DL/anode time sum 


  
  double fp1_y=-1, fp2_y=-1;
  double anodeFront = -1, anodeBack = -1, scintRight = -1, scintLeft = -1;
  double scintRightShort = -1, scintLeftShort = -1;
  double cathode = -1;

  double xavg = -1e6, x1 = -1e6, x2 = -1e6; // member to calc X1/X2 using DL time diff.
  double x1_sum = -1e6, x2_sum = -1e6; // member to calc X1/X2 using DL time sum.
  double x1_sumA = -1e-6, x2_sumB = -1e-6; // member to calc X1/X2 using DL time sum and anode times
  double x1FL = -1e6, x1FR = -1e6, x2BL = -1e6, x2BR = -1e6; // members to calc different X1/X2 from 4 DL readouts
  double x1FL_sum = -1e6, x1FR_sum = -1e6; // members to calc different X1 from 2 DL readouts
  double x2BL_sum = -1e6, x2BR_sum = -1e6; // members to calc different X2 from 2 DL readouts

  double x1tilde_FL = -1e6, x1tilde_FR = -1e6; // members to calc different X1 from 2 DL readouts
  double x2tilde_BL = -1e6, x2tilde_BR = -1e6; // members to calc different X2 from 2 DL readouts

  double xavg_tildeFRBL = -1e6, xavg_tildeFLBR = -1e6, xavg_tildeFRBR = -1e6, xavg_tildeFLBL = -1e6; // member to calc Xavg using ~X1 and ~X2

  
  double sabreRingE[5] = {-1,-1,-1,-1,-1}, sabreWedgeE[5] = {-1,-1,-1,-1,-1};
  double sabreRingChannel[5] = {-1,-1,-1,-1,-1}, sabreWedgeChannel[5] = {-1,-1,-1,-1,-1};
  double sabreRingTime[5] = {-1,-1,-1,-1,-1}, sabreWedgeTime[5] = {-1,-1,-1,-1,-1};
  
  
  double theta = -1e6;
  double delayFrontRightE = -1, delayFrontLeftE = -1;
  double delayBackRightE = -1, delayBackLeftE = -1;
  double delayFrontRightShort = -1, delayFrontLeftShort = -1;
  double delayBackRightShort = -1, delayBackLeftShort = -1;
  double anodeFrontTime = -1, anodeBackTime = -1;
  double scintRightTime = -1, scintLeftTime = -1;
  double delayFrontMaxTime = -1, delayBackMaxTime = -1;
  double delayFrontLeftTime = -1, delayFrontRightTime = -1;
  double delayBackLeftTime = -1, delayBackRightTime = -1;
  double cathodeTime = -1;

  double monitorE = -1, monitorShort = -1;
  double monitorTime = -1;


  double catrinaE[7] = {-1,-1,-1,-1,-1,-1,-1};
  double catrinaChannel[7] = {-1,-1,-1,-1,-1,-1,-1};
  double catrinaTime[7] = {-1,-1,-1,-1,-1,-1,-1};

  double catrinaE0 = -1;
  double catrinaE1 = -1;
  double catrinaE2 = -1;
  double catrinaE3 = -1;
  double catrinaE4 = -1;
  double catrinaE5 = -1;
  double catrinaE6 = -1;


  double catrinaChannel0 = -1;
  double catrinaChannel1 = -1;
  double catrinaChannel2 = -1;
  double catrinaChannel3 = -1;
  double catrinaChannel4 = -1;
  double catrinaChannel5 = -1;
  double catrinaChannel6 = -1;


  double catrinaTime0 = -1;
  double catrinaTime1 = -1;
  double catrinaTime2 = -1;
  double catrinaTime3 = -1;
  double catrinaTime4 = -1;
  double catrinaTime5 = -1;
  double catrinaTime6 = -1;

  SabreDetector sabreArray[5]; //index = ChannelMap Id# -1
  CATRINADetector catrinaArray[7];// JCE 2025

};

/*
  ROOT does a bad job of ensuring that header-only type dictionaries (the only type they explicity accept)
  are linked when compiled as shared libraries (the recommended method). As a work around, as a dummy function that 
  ensures the library is linked (better than no-as-needed which I dont think is in general supported across platforms)
*/
bool EnforceDictionaryLinked();

#endif
