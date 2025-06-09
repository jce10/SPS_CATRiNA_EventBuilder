/*

SFPAnalyzer.cpp

Class designed to analyze coincidence events. Currently only implemented for focal plane
data. Additional changes for SABRE would include this file and the sructure ProcessedEvent
in DataStructs.h. Based on code written by S. Balak, K. Macon, and E. Good.

Gordon M. Oct. 2019
 
Refurbished and updated Jan 2020 by GWM. Now uses both focal plane and SABRE data
 
Position calibrations swapped as of Aug. 2021 due to detector fixes -- GWM

Heavily modified by J. Esparza 2024/2025 to include plane reconstruction, focal plane diagnostics and CATRiNA event building.

*/

#include "SFPAnalyzer.h"

namespace EventBuilder {

	/*Constructor takes in kinematic parameters for generating focal plane weights*/
	SFPAnalyzer::SFPAnalyzer(int zt, int at, int zp, int ap, int ze, int ae, double ep,
								double angle, double b, double nudge, double Q) 
	{
		zfp = Delta_Z(zt, at, zp, ap, ze, ae, ep, angle, b*1000.0, nudge, Q); //Convert kG to G
		//EVB_INFO("Nudge factor and Q value are {0} and {1} respectively",nudge,Q);
		EVB_INFO("the kinematic inputs are: zt={0}, at={1}, zp={2}, ap={3}, ze={4}, ae={5}, ep={6}, angle={7}, b={8}, nudge={9}, and Q={10}",
					zt,at,zp,ap,ze,ae,ep,angle,b,nudge,Q);
		EVB_INFO("Focal plane shift is {0} cm",zfp);
		event_address = new CoincEvent();
		rootObj = new THashTable();
		GetWeights();
	}
	
	SFPAnalyzer::~SFPAnalyzer() 
	{
		rootObj->Clear();
		delete rootObj;
		delete event_address;
	}
	
	void SFPAnalyzer::Reset() 
	{
		pevent = blank; //set output back to blank
	}
	
	/*Use functions from FP_kinematics to calculate weights for xavg
	 *While this seems kind of funny, it is mathematically equivalent to making a line
	 *from the two focal plane points and finding the intersection with 
	 *the kinematic focal plane
	 */
	void SFPAnalyzer::GetWeights() 
	{
		w1 = (Wire_Dist()/2.0-zfp)/Wire_Dist();
		w2 = 1.0-w1;
		EVB_INFO("Calculated X-Avg weights of w1={0} and w2={1}",w1,w2);
	}
	
	/*2D histogram fill wrapper for use with THashTable (faster)*/
	void SFPAnalyzer::MyFill(const std::string& name, int binsx, double minx, double maxx, double valuex,
								int binsy, double miny, double maxy, double valuey) 
	{
		TH2F *histo = (TH2F*) rootObj->FindObject(name.c_str());
		if(histo != nullptr) 
			histo->Fill(valuex, valuey);
		else 
		{
			TH2F *h = new TH2F(name.c_str(), name.c_str(), binsx, minx, maxx, binsy, miny, maxy);
			h->Fill(valuex, valuey);
			rootObj->Add(h);
		}
	}
	
	/*1D histogram fill wrapper for use with THashTable (faster)*/
	void SFPAnalyzer::MyFill(const std::string& name, int binsx, double minx, double maxx, double valuex)
	{
		TH1F *histo = (TH1F*) rootObj->FindObject(name.c_str());
		if(histo != nullptr)
			histo->Fill(valuex);
		else 
		{
			TH1F *h = new TH1F(name.c_str(), name.c_str(), binsx, minx, maxx);
			h->Fill(valuex);
			rootObj->Add(h);
		}
	}
	
	void SFPAnalyzer::AnalyzeEvent(CoincEvent& event) 
	{
		//Set the address of the event to be analyzed. 

		Reset();

		// anodes 
		if(!event.focalPlane.anodeF.empty()) 
		{
			pevent.anodeFront = event.focalPlane.anodeF[0].Long;
			pevent.anodeFrontTime = event.focalPlane.anodeF[0].Time;
		}
		if(!event.focalPlane.anodeB.empty()) 
		{
			pevent.anodeBack = event.focalPlane.anodeB[0].Long;
			pevent.anodeBackTime = event.focalPlane.anodeB[0].Time;
		}

		// scinitillators
		if(!event.focalPlane.scintL.empty()) 
		{
			pevent.scintLeft = event.focalPlane.scintL[0].Long;
			pevent.scintLeftShort = event.focalPlane.scintL[0].Short;
			pevent.scintLeftTime = event.focalPlane.scintL[0].Time;
		}
		if(!event.focalPlane.scintR.empty()) 
		{
			pevent.scintRight = event.focalPlane.scintR[0].Long;
			pevent.scintRightShort = event.focalPlane.scintR[0].Short;
			pevent.scintRightTime = event.focalPlane.scintR[0].Time;
		}

		// cathode and monitor
		if(!event.focalPlane.cathode.empty()) 
		{
			pevent.cathode = event.focalPlane.cathode[0].Long;
			pevent.cathodeTime = event.focalPlane.cathode[0].Time;
		}
		if(!event.focalPlane.monitor.empty()) 
		{
			pevent.monitorE = event.focalPlane.monitor[0].Long;
			pevent.monitorShort = event.focalPlane.monitor[0].Short;
			pevent.monitorTime = event.focalPlane.monitor[0].Time;
		}
	
		/*Delay lines and all that*/
		if(!event.focalPlane.delayFR.empty()) 
		{
			pevent.delayFrontRightE = event.focalPlane.delayFR[0].Long;
			pevent.delayFrontRightTime = event.focalPlane.delayFR[0].Time;
			pevent.delayFrontRightShort = event.focalPlane.delayFR[0].Short;
		}
		if(!event.focalPlane.delayFL.empty()) 
		{
			pevent.delayFrontLeftE = event.focalPlane.delayFL[0].Long;
			pevent.delayFrontLeftTime = event.focalPlane.delayFL[0].Time;
			pevent.delayFrontLeftShort = event.focalPlane.delayFL[0].Short;
		}
		if(!event.focalPlane.delayBR.empty()) 
		{
			pevent.delayBackRightE = event.focalPlane.delayBR[0].Long;
			pevent.delayBackRightTime = event.focalPlane.delayBR[0].Time;
			pevent.delayBackRightShort = event.focalPlane.delayBR[0].Short;
		}
		if(!event.focalPlane.delayBL.empty()) 
		{
			pevent.delayBackLeftE = event.focalPlane.delayBL[0].Long;
			pevent.delayBackLeftTime = event.focalPlane.delayBL[0].Time;
			pevent.delayBackLeftShort = event.focalPlane.delayBL[0].Short;
		}
		
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		// build X1 from the delay line times
		if(!event.focalPlane.delayFL.empty() && !event.focalPlane.delayFR.empty()) 
		{ 
			pevent.fp1_tdiff = (event.focalPlane.delayFL[0].Time - event.focalPlane.delayFR[0].Time)*0.5;
			pevent.fp1_tsum = (event.focalPlane.delayFL[0].Time + event.focalPlane.delayFR[0].Time) - (2*event.focalPlane.scintL[0].Time);// sum rel to scint

			pevent.fp1_tcheck = (pevent.fp1_tsum)/2.0 - pevent.anodeFrontTime;
			pevent.delayFrontMaxTime = std::max(event.focalPlane.delayFL[0].Time, event.focalPlane.delayFR[0].Time);

			pevent.x1 = pevent.fp1_tdiff*1.0/2.10; //position from time, based on total delay
			pevent.x1_sum = pevent.fp1_tsum; // testing JCE 2025


			MyFill("x1",1200,-600,600,pevent.x1);
			//MyFill("x1_tsum",512,0,16000,pevent.x1_sum);
			MyFill("x1 vs tsum scint",600,-300,300,pevent.x1,512,0,16000,pevent.fp1_tsum);
			MyFill("x1 vs anodeBack",600,-300,300,pevent.x1,512,0,4096,pevent.anodeBack);
		}
		
		// build X2 from the delay line times
		if(!event.focalPlane.delayBL.empty() && !event.focalPlane.delayBR.empty()) 
		{
			pevent.fp2_tdiff = (event.focalPlane.delayBL[0].Time - event.focalPlane.delayBR[0].Time)*0.5;
			pevent.fp2_tsum = (event.focalPlane.delayBL[0].Time + event.focalPlane.delayBR[0].Time) - (2*event.focalPlane.scintL[0].Time);

			pevent.fp2_tcheck = (pevent.fp2_tsum)/2.0 - pevent.anodeBackTime;
			pevent.delayBackMaxTime = std::max(event.focalPlane.delayBL[0].Time, event.focalPlane.delayBR[0].Time);

			pevent.x2 = pevent.fp2_tdiff*1.0/1.98; //position from time, based on total delay
			pevent.x2_sum = pevent.fp2_tsum; // testing JCE 2025


			MyFill("x2",1200,-600,600,pevent.x2);
			//MyFill("x2_tsum",512,0,16000,pevent.x2_sum);
			MyFill("x2 vs tsum scint",600,-300,300,pevent.x2,512,0,16000,pevent.fp2_tsum);
			MyFill("x2 vs anodeBack",600,-300,300,pevent.x2,512,0,4096,pevent.anodeBack);
		}


		/*SABRE data*/
		for(int j=0; j<5; j++) 
		{
			if(!event.sabreArray[j].rings.empty()) 
			{
				pevent.sabreRingE[j] = event.sabreArray[j].rings[0].Long;
				pevent.sabreRingChannel[j] = event.sabreArray[j].rings[0].Ch;
				pevent.sabreRingTime[j] = event.sabreArray[j].rings[0].Time;
			}
			if(!event.sabreArray[j].wedges.empty()) 
			{
				pevent.sabreWedgeE[j] = event.sabreArray[j].wedges[0].Long;
				pevent.sabreWedgeChannel[j] = event.sabreArray[j].wedges[0].Ch;
				pevent.sabreWedgeTime[j] = event.sabreArray[j].wedges[0].Time;
			}
			/*Aaaand passes on all of the rest. 4/24/20 GWM*/
			pevent.sabreArray[j] = event.sabreArray[j];
		}


		/*
		
			//here we place the CATRiNA detector analysis into the event structure

		for(int j=0; j<7; j++) {
           
            if(!event.catrinaArray[j].cebr.empty())
            {
                // for(unsigned int l=0; l<=event.cebraArray[j].cebr.size(); l++){
                //    if(j==0){
                //         MyFill("CebraE0",4096,0,4096,event.cebraArray[j].cebr[l].Long);}
                //     else if(j==1){
                //         MyFill("CebraE1",4096,0,4096,event.cebraArray[j].cebr[l].Long);}    
                //     else if(j==2){
                //         MyFill("CebraE2",4096,0,4096,event.cebraArray[j].cebr[l].Long);}
                //     else if(j==3){
                //         MyFill("CebraE3",4096,0,4096,event.cebraArray[j].cebr[l].Long);}
                //     else if(j==4){
                //         MyFill("CebraE4",4096,0,4096,event.cebraArray[j].cebr[l].Long);}
                   
                //    }

                if(j==0){
                    pevent.cebraE0 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel0 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime0 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==1){
                    pevent.cebraE1 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel1 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime1 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==2){
                    pevent.cebraE2 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel2 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime2 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==3){
                    pevent.cebraE3 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel3 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime3 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==4){
                    pevent.cebraE4 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel4 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime4 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==5){
                    pevent.cebraE5 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel5 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime5 = event.cebraArray[j].cebr[0].Time;
                }

                else if(j==6){
                    pevent.cebraE6 = pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
                    pevent.cebraChannel6 = event.cebraArray[j].cebr[0].Ch;
                    pevent.cebraTime6 = event.cebraArray[j].cebr[0].Time;
                }
     
            pevent.cebraE[j] = event.cebraArray[j].cebr[0].Long;
            pevent.cebraChannel[j] = event.cebraArray[j].cebr[0].Ch;
            pevent.cebraTime[j] = event.cebraArray[j].cebr[0].Time;

            j=j++;
     
    //   MyFill("CeBrAE vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);
    //   if(j==0){
    //         MyFill("CeBrA.E.0 vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);}
    //         else if(j==1){
    //         MyFill("CeBrA.E.1 vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);}
    //         else if(j==2){
    //         MyFill("CeBrA.E.2 vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);}
    //         else if(j==3){
    //         MyFill("CeBrA.E.3 vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);}
    //         else if(j==4){
    //         MyFill("CeBrA.E.4 vs CeBrA_channel",4096,0,4096,pevent.cebraE[j],200,0,200,pevent.cebraChannel[j]);}
    }

    // if(pevent.cebraE[0]!=-1 && pevent.cebraE[1]!=-1){
    //         MyFill("cebraE0_vs_cebraE1_noCuts",4096,0,4096,pevent.cebraE[0],4096,0,4096,pevent.cebraE[1]);
    //         MyFill("cebraTime0-cebraTime1_noCuts",3000,-1500,1500,pevent.cebraTime[0]-pevent.cebraTime[1]);
    //         }
    // if(pevent.cebraE[0]!=-1 && pevent.cebraE[2]!=-1){
    //         MyFill("cebraE0_vs_cebraE2_noCuts",4096,0,4096,pevent.cebraE[0],4096,0,4096,pevent.cebraE[2]);
    //         MyFill("cebraTime0-cebraTime2_noCuts",3000,-1500,1500,pevent.cebraTime[0]-pevent.cebraTime[2]);
    //         }
    // if(pevent.cebraE[0]!=-1 && pevent.cebraE[3]!=-1){
    //         MyFill("cebraE0_vs_cebraE3_noCuts",4096,0,4096,pevent.cebraE[0],4096,0,4096,pevent.cebraE[3]);
    //         MyFill("cebraTime0-cebraTime3_noCuts",3000,-1500,1500,pevent.cebraTime[0]-pevent.cebraTime[3]);
    //         }
    // if(pevent.cebraE[0]!=-1 && pevent.cebraE[4]!=-1){
    //         MyFill("cebraE0_vs_cebraE4_noCuts",4096,0,4096,pevent.cebraE[0],4096,0,4096,pevent.cebraE[4]);
    //         MyFill("cebraTime0-cebraTime4_noCuts",3000,-1500,1500,pevent.cebraTime[0]-pevent.cebraTime[4]);
    //         }
    /*Aaaand passes on all of the rest. 4/24/20 GWM // adjusted Mark
    pevent.cebraArray[j] = event.cebraArray[j];
  }

  
        if(pevent.cebraE[0]!=-1){ 
            MyFill("CebraE0",4096,0,4096,pevent.cebraE[0]);}
        if(pevent.cebraE[1]!=-1){ 
            MyFill("CebraE1",4096,0,4096,pevent.cebraE[1]);}
        if(pevent.cebraE[2]!=-1){ 
            MyFill("CebraE2",4096,0,4096,pevent.cebraE[2]);}
        if(pevent.cebraE[3]!=-1){ 
            MyFill("CebraE3",4096,0,4096,pevent.cebraE[3]);}
        if(pevent.cebraE[4]!=-1){ 
            MyFill("CebraE4",4096,0,4096,pevent.cebraE[4]);}
        if(pevent.cebraE[5]!=-1){ 
            MyFill("CebraE5",4096,0,4096,pevent.cebraE[5]);}
        if(pevent.cebraE[6]!=-1){ 
            MyFill("CebraE6",4096,0,4096,pevent.cebraE[6]);}



		
		
	*/

	
		/*Make some histograms and xavg*/
		MyFill("anodeBack vs scintLeft",512,0,4096,pevent.scintLeft,512,0,4096,pevent.anodeBack);


		if(pevent.x1 != -1e6 && pevent.x2 != -1e6) 
		{
			// calculate xavg
			pevent.xavg = pevent.x1*w1 + pevent.x2*w2;
			MyFill("xavg",1200,-400,400,pevent.xavg);

			if((pevent.x2 - pevent.x1) > 0) 
				pevent.theta = std::atan((pevent.x2 - pevent.x1)/36.0);
			else if((pevent.x2 - pevent.x1) < 0)
				pevent.theta = TMath::Pi() + std::atan((pevent.x2 - pevent.x1)/36.0);
			else 
				pevent.theta = TMath::Pi()/2.0;
			MyFill("xavg vs theta",600,-300,300,pevent.xavg,314,0,3.14,pevent.theta);
			MyFill("x1 vs x2",600,-300,300,pevent.x1,600,-300,300,pevent.x2);

		}

//#################################### X1/2 from anode times ####################################

		// build X1_FL from the delay line and front anode times
		if(!event.focalPlane.delayFL.empty() && !event.focalPlane.anodeF.empty()) 
		{ 
			pevent.fp1FL_tdiff_anodeFront = (event.focalPlane.delayFL[0].Time - pevent.anodeFrontTime);
			pevent.fp1_tsum_FL = (event.focalPlane.delayFL[0].Time + pevent.anodeFrontTime) - (2*event.focalPlane.scintL[0].Time);
			pevent.x1FL = pevent.fp1FL_tdiff_anodeFront*1.0/2.10; //position from time, based on delayFL and anodeFront
			pevent.x1FL_sum = pevent.fp1_tsum_FL; // testing JCE 2025
			
			MyFill("x1_FL",1200,-150,700,pevent.x1FL);
			//MyFill("x1_FL_tsum",512,0,16000,pevent.x1FL_sum);
			//MyFill("x1_FL vs tsum",600,-300,300,pevent.x1FL,512,0,16000,pevent.fp1_tsum_FL);
			// MyFill("x1_FL vs anodeFront",600,-300,300,pevent.x1FL,512,0,4096,pevent.anodeFront);
			// MyFill("x1_FL vs anodeBack",600,-300,300,pevent.x1FL,512,0,4096,pevent.anodeBack);
			// MyFill("x1_FL vs scintLeft",600,-300,300,pevent.x1FL,512,0,4096,pevent.scintLeft);
			// MyFill("x1_FL vs scintRight",600,-300,300,pevent.x1FL,512,0,4096,pevent.scintRight);
		}

		// build X1_FR from the delay line and front anode times
		if(!event.focalPlane.delayFR.empty() && !event.focalPlane.anodeF.empty()) 
		{ 
			pevent.fp1FR_tdiff_anodeFront = (event.focalPlane.delayFR[0].Time - pevent.anodeFrontTime);
			pevent.x1FR = pevent.fp1FR_tdiff_anodeFront*1.0/2.10; //position from time, based on delayFR and anodeFront
			pevent.fp1_tsum_FR = (event.focalPlane.delayFR[0].Time + pevent.anodeFrontTime) - (2*event.focalPlane.scintL[0].Time);
			pevent.x1FR_sum = pevent.fp1_tsum_FR; // testing JCE 2025

			MyFill("x1_FR",1200,-100,600,pevent.x1FR);
			//MyFill("x1_FR_tsum",512,0,16000,pevent.x1FR_sum);
			//MyFill("x1_FR vs tsum",600,-300,300,pevent.x1FR,512,0,16000,pevent.fp1_tsum_FR);
			// MyFill("x1_FR vs anodeFront",600,-300,300,pevent.x1FR,512,0,4096,pevent.anodeFront);
			// MyFill("x1_FR vs anodeBack",600,-300,300,pevent.x1FR,512,0,4096,pevent.anodeBack);
			// MyFill("x1_FR vs scintLeft",600,-300,300,pevent.x1FR,512,0,4096,pevent.scintLeft);
			// MyFill("x1_FR vs scintRight",600,-300,300,pevent.x1FR,512,0,4096,pevent.scintRight);			
		}


		// build X1 with the tsum from the delay line and front anode times
		if(!event.focalPlane.delayFL.empty() && !event.focalPlane.delayFR.empty() && !event.focalPlane.anodeF.empty()) 
		{ 
			pevent.fp1_tsumA = (pevent.fp1FL_tdiff_anodeFront + pevent.fp1FR_tdiff_anodeFront);
			pevent.x1_sumA = pevent.fp1_tsumA; // testing JCE 2025

			MyFill("x1 vs tsum anode",600,-300,300,pevent.x1,1200,0,2000,pevent.fp1_tsumA);
		}


		// build X2_BL from the delay line and back anode times
		if(!event.focalPlane.delayBL.empty() && !event.focalPlane.anodeB.empty()) 
		{ 
			pevent.fp2BL_tdiff_anodeBack = (event.focalPlane.delayBL[0].Time - pevent.anodeBackTime); // back left time

			pevent.x2BL = pevent.fp2BL_tdiff_anodeBack*1.0/1.98; //position from time, based on delayBL and anodeBack
			pevent.fp2_tsum_BL = (event.focalPlane.delayBL[0].Time + pevent.anodeBackTime) - (2*event.focalPlane.scintL[0].Time); // sum relative to scint
			pevent.x2BL_sum = pevent.fp2_tsum_BL; // testing JCE 2025


			MyFill("x2_BL",1200,-300,800,pevent.x2BL);
			//MyFill("x2_BL_tsum",512,0,16000,pevent.x2BL_sum);
			//MyFill("x2_BL vs tsum",600,-300,300,pevent.x2BL,512,0,16000,pevent.fp2_tsum_BL);
			// MyFill("x2_BL vs anodeFront",600,-300,300,pevent.x2BL,512,0,4096,pevent.anodeFront);
			// MyFill("x2_BL vs anodeBack",600,-300,300,pevent.x2BL,512,0,4096,pevent.anodeBack);
			// MyFill("x2_BL vs scintLeft",600,-300,300,pevent.x2BL,512,0,4096,pevent.scintLeft);
			// MyFill("x2_BL vs scintRight",600,-300,300,pevent.x2BL,512,0,4096,pevent.scintRight);			
		}

		// build X2_BR from the delay line and back anode times
		if(!event.focalPlane.delayBR.empty() && !event.focalPlane.anodeB.empty()) 
		{ 
			pevent.fp2BR_tdiff_anodeBack = (event.focalPlane.delayBR[0].Time - pevent.anodeBackTime); // back right time

			pevent.x2BR = pevent.fp2BR_tdiff_anodeBack*1.0/1.98; //position from time, based on delayBR and anodeBack
			pevent.fp2_tsum_BR = (event.focalPlane.delayBR[0].Time + pevent.anodeBackTime) - (2*event.focalPlane.scintL[0].Time); // sum relative to scint
			pevent.x2BR_sum = pevent.fp2_tsum_BR; // testing JCE 2025


			MyFill("x2_BR",1200,-300,800,pevent.x2BR);
			//MyFill("x2_BR_tsum",512,0,16000,pevent.x2BR_sum);
			//MyFill("x2_BR vs tsum",600,-300,300,pevent.x2BR,512,0,16000,pevent.fp2_tsum_BR);
			//MyFill("x2_BR vs anodeFront",600,-300,300,pevent.x2BR,512,0,4096,pevent.anodeFront);
			// MyFill("x2_BR vs anodeBack",600,-300,300,pevent.x2BR,512,0,4096,pevent.anodeBack);
			// MyFill("x2_BR vs scintLeft",600,-300,300,pevent.x2BR,512,0,4096,pevent.scintLeft);
			// MyFill("x2_BR vs scintRight",600,-300,300,pevent.x2BR,512,0,4096,pevent.scintRight);
		}

		// build X2 with the tsum from the delay line and back anode times
		if(!event.focalPlane.delayBL.empty() && !event.focalPlane.delayBR.empty() && !event.focalPlane.anodeB.empty()) 
		{ 
			pevent.fp2_tsumB = (pevent.fp2BL_tdiff_anodeBack + pevent.fp2BR_tdiff_anodeBack);
			pevent.x2_sumB = pevent.fp2_tsumB; // testing JCE 2025
						
			MyFill("x2 vs tsum anode",600,-300,300,pevent.x2,500,950,1450,pevent.fp2_tsumB);
		}

//#############################################################################################################


//######################################## X1/2_tildes #########################################################
		
		// build X1 with only half the delay line time (left side)
		if(!event.focalPlane.delayFL.empty() && !event.focalPlane.anodeF.empty()) 
		{ 
			pevent.fp1FL_tdiff_tilde = (pevent.fp1FL_tdiff_anodeFront - 1200/2.0);
			pevent.x1tilde_FL = pevent.fp1FL_tdiff_tilde*1.0/2.10; //position from time, based on delayFL and anodeFront
						
			MyFill("x1_tilde_FL",1200,-500,500,pevent.x1tilde_FL);
		}

		// build X1 with only half the delay line time (right side)
		if( !event.focalPlane.delayFR.empty() && !event.focalPlane.anodeF.empty()) 
		{ 
			pevent.fp1FR_tdiff_tilde = (1200/2.0 - pevent.fp1FR_tdiff_anodeFront);
			pevent.x1tilde_FR = pevent.fp1FR_tdiff_tilde*1.0/2.10; //position from time, based on delayFR and anodeFront
						
			MyFill("x1_tilde_FR",1200,-300,500,pevent.x1tilde_FR);
		}
		
		// build X2 with only half the delay line time (left side)
		if(!event.focalPlane.delayBL.empty()  && !event.focalPlane.anodeB.empty()) 
		{ 
			pevent.fp2BL_tdiff_tilde = (pevent.fp2BL_tdiff_anodeBack - 1154/2.0);
			pevent.x2tilde_BL = pevent.fp2BL_tdiff_tilde*1.0/1.98; //position from time, based on delayBL and anodeBack
						
			MyFill("x2_tilde_BL",1200,-400,400,pevent.x2tilde_BL);
		}

		// build X2 with only half the delay line time (right side)
		if(!event.focalPlane.delayBR.empty() && !event.focalPlane.anodeB.empty()) 
		{ 
			pevent.fp2BR_tdiff_tilde = (1154/2.0 - pevent.fp2BR_tdiff_anodeBack); 
			pevent.x2tilde_BR = pevent.fp2BR_tdiff_tilde*1.0/1.98; //position from time, based on delayBR and anodeBack
						
			MyFill("x2_tilde_BR",1200,-400,400,pevent.x2tilde_BR);
		}

//#############################################################################################################


//######################################## Xavg_tildes #########################################################
		
		// make a new Xavg with the ~x1_FR and ~x2_BL
		if(pevent.x1tilde_FR != -1e6 && pevent.x2tilde_BL != -1e6) 
		{
			// calculate xavg_tilde
			pevent.xavg_tildeFRBL = pevent.x1tilde_FR*w1 + pevent.x2tilde_BL*w2; 
			MyFill("xavg_tilde_FRBL",1200,-400,400,pevent.xavg_tildeFRBL);
		}

		// make a new Xavg with the ~x1_FL and ~x2_BR
		if(pevent.x1tilde_FL != -1e6 && pevent.x2tilde_BR != -1e6) 
		{
			// calculate xavg_tilde
			pevent.xavg_tildeFLBR = pevent.x1tilde_FL*w1 + pevent.x2tilde_BR*w2;
			MyFill("xavg_tilde_FLBR",1200,-400,400,pevent.xavg_tildeFLBR);
		}

		// make a new Xavg with the ~x1_FL and ~x2_BL
		if(pevent.x1tilde_FL != -1e6 && pevent.x2tilde_BL != -1e6) 
		{
			// calculate xavg_tilde
			pevent.xavg_tildeFLBL = pevent.x1tilde_FL*w1 + pevent.x2tilde_BL*w2;
			MyFill("xavg_tilde_FLBL",1200,-400,400,pevent.xavg_tildeFLBL);
		}

		// make a new Xavg with the ~x1_FR and ~x2_BR
		if(pevent.x1tilde_FR != -1e6 && pevent.x2tilde_BR != -1e6) 
		{
			// calculate xavg_tilde
			pevent.xavg_tildeFRBR = pevent.x1tilde_FR*w1 + pevent.x2tilde_BR*w2;
			MyFill("xavg_tilde_FRBR",1200,-400,400,pevent.xavg_tildeFRBR);
		}

//#############################################################################################################


		/* 
			note to future self: if i want number of counts of just x1 object for comparing to x1_only1plane,
			i will add the counter here. 
			
			x1_only1plane + x1_counts = losses


			Left + Right + anode events
			
			JCE 04/2025
		*/


		if(pevent.anodeFrontTime != -1 && pevent.scintRightTime != -1)
			pevent.fp1_y = pevent.anodeFrontTime - pevent.scintRightTime;

		if(pevent.anodeBackTime != -1 && pevent.scintRightTime != -1)
			pevent.fp2_y = pevent.anodeBackTime - pevent.scintRightTime;


	}
	
	ProcessedEvent SFPAnalyzer::GetProcessedEvent(CoincEvent& event)
	{
		AnalyzeEvent(event);
		return pevent;
	}

}