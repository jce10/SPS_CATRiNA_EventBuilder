/*
	CompassRun.cpp
	Class designed as abstraction of a collection of binary files that represent the total data in a single
	Compass data run. It handles the user input (shift maps, file collection etc.) and creates a list of
	CompassFiles from which to draw data. It then draws data from these files, organizes them in time,
	and writes to a ROOT file for further processing.

	Written by G.W. McCann Oct. 2020

	Updated to also handle scaler data. -- GWM Oct. 2020
*/
#include "CompassRun.h"
#include "SlowSort.h"
#include "FastSort.h"
#include "SFPAnalyzer.h"
#include "FlagHandler.h"
#include "EVBApp.h"

namespace EventBuilder {

	// Constructor that initializes CompassRun with the given parameters and workspace
	CompassRun::CompassRun(const EVBParameters& params, const std::shared_ptr<EVBWorkspace>& workspace) :
		m_params(params), m_workspace(workspace)
	{
		// Set the time shift map using the provided file
		m_smap.SetFile(m_params.timeShiftFile);
	}
	
	// Destructor
	CompassRun::~CompassRun() {}
	
	/*
		SetScalers() loads scaler data into a map. This data is typically used for monitoring 
		and controlling experiments. The file contains filenames paired with corresponding variables.
	*/
	void CompassRun::SetScalers() 
	{
		// Open the scaler file
		std::ifstream input(m_params.scalerFile);
		if(!input.is_open()) 
			return; // Return if the file cannot be opened
	
		m_scaler_flag = true; // Indicate that scaler data is available
		std::string junk, filename, varname;
		Long64_t init = 0; // Initialize count variable for scalers
		std::getline(input, junk); // Skip the first line
		std::getline(input, junk); // Skip the second line
		m_scaler_map.clear(); // Clear any previous scaler data
	
		// Read each filename and variable name from the scaler file
		while(input>>filename) 
		{
			input>>varname;
			// Construct full filename for the scaler
			filename = m_workspace->GetTempDir()+filename+"_run_"+std::to_string(m_runNum)+".BIN";
			// Insert the scaler information into the map with the variable name and initial value
			m_scaler_map[filename] = TParameter<Long64_t>(varname.c_str(), init);
		}
		input.close(); // Close the file
	}
	
	/*
		GetBinaryFiles() loads binary data files into the CompassRun instance.
		It iterates through available files and reads in the data, including scaler data if present.
	*/
	bool CompassRun::GetBinaryFiles() 
	{
		auto files = m_workspace->GetTempFiles(); // Get the list of files from the workspace

		m_datafiles.clear(); // Clear previous data files
		m_datafiles.reserve(files.size()); // Preallocate memory for data files
		bool scalerd;
		m_totalHits = 0; // Reset the total number of hits (events)
	
		// Loop through the list of files
		for(auto& entry : files) 
		{
			// If scaler data is present, process the scaler file
			if(m_scaler_flag) 
			{
				scalerd = false;
				for(auto& scaler_pair : m_scaler_map) 
				{
					// If this file is a scaler file, process it and skip to the next one
					if(entry == scaler_pair.first) 
					{
						ReadScalerData(entry); // Read scaler data
						scalerd = true;
						break;
					}
				}
				if(scalerd) 
					continue; // Skip the scaler file
			}
	
			// Otherwise, treat it as a data file
			m_datafiles.emplace_back(entry);
			m_datafiles[m_datafiles.size()-1].AttachShiftMap(&m_smap); // Attach the shift map to the data file

			// Check if the file is successfully opened; if not, return false
			if(!m_datafiles[m_datafiles.size() - 1].IsOpen()) 
				return false;
	
			// Increment the total number of hits
			m_totalHits += m_datafiles[m_datafiles.size()-1].GetNumberOfHits();
		}
	
		return true; // Successfully loaded files
	}
	
	/*
		ReadScalerData() counts the number of hits in a scaler file and updates the corresponding scaler map.
	*/
	void CompassRun::ReadScalerData(const std::string& filename) 
	{
		if(!m_scaler_flag) 
			return; // Return if scaler data is not enabled
	
		Long64_t count = 0; // Initialize the hit counter
		CompassFile file(filename); // Open the scaler file
		auto& this_param = m_scaler_map[file.GetName()]; // Get the corresponding parameter for this file
		
		// Loop through the hits in the file and count them
		while(true) 
		{
			file.GetNextHit(); // Get the next hit from the file
			if(file.IsEOF()) // End of file reached
				break;
			count++; // Increment the hit counter
		}
		// Set the final count in the scaler map
		this_param.SetVal(count);
	}
	
	/*
		- GetHitsFromFiles() retrieves and sorts hits from the individual files. It ensures that hits are processed in
		  the correct order based on their timestamp.

		- Once a file has gone EOF, we no longer need it. If this is the first file in the list, we can just skip
		  that index all together. In this way, the loop can go from N times to N-1 times.
	*/
	bool CompassRun::GetHitsFromFiles() 
	{
		// Declare a pointer to store the earliest hit (based on timestamp)
		CompassFile* earliestHit = nullptr;
		
		// Loop through the data files starting from the current start index
		for(unsigned int i=startIndex; i<m_datafiles.size(); i++) 
		{
			// If the hit has already been used, move to the next one
			if(m_datafiles[i].CheckHitHasBeenUsed())
				m_datafiles[i].GetNextHit();

			// If the file has reached EOF, continue to the next file
			if(m_datafiles[i].IsEOF()) 
			{
				if(i == startIndex)
					startIndex++; // Skip the first file index after EOF
				continue;
			}
			// If this is the first file in the list, set it as the earliest hit
			else if(i == startIndex) 
				earliestHit = &m_datafiles[i];
			// Otherwise, compare timestamps to find the earliest hit
			else if(m_datafiles[i].GetCurrentHit().timestamp < earliestHit->GetCurrentHit().timestamp) 
				earliestHit = &m_datafiles[i];
		}
	
		// Return false if no hits were found
		if(earliestHit == nullptr) 
			return false;
		
		// Set the current hit from the earliest hit file
		m_hit = earliestHit->GetCurrentHit();
		// Mark the hit as used
		earliestHit->SetHitHasBeenUsed();
		
		return true; // Successfully retrieved a hit
	}
	
	// Further methods (Convert2RawRoot, Convert2SortedRoot, etc.) would follow a similar pattern, 
	// processing data, sorting events, and writing to ROOT files.


	void CompassRun::Convert2RawRoot(const std::string& name) {
		TFile* output = TFile::Open(name.c_str(), "RECREATE");
		TTree* outtree = new TTree("Data", "Data");
	
		outtree->Branch("Board", &m_hit.board);
		outtree->Branch("Channel", &m_hit.channel);
		outtree->Branch("Energy", &m_hit.energy);
		outtree->Branch("EnergyShort", &m_hit.energyShort);
		outtree->Branch("Timestamp", &m_hit.timestamp);
		outtree->Branch("Flags", &m_hit.flags);
	
		if(!m_smap.IsValid()) 
		{
			EVB_WARN("Bad shift map ({0}) at CompassRun::Convert(), shifts all set to 0.", m_smap.GetFilename());
		}
	
		SetScalers();
	
		if(!GetBinaryFiles()) 
		{
			EVB_ERROR("Unable to find binary files at CompassRun::Convert(), exiting!");
			return;
		}
	
		unsigned int count = 0, flush = m_totalHits*m_progressFraction, flush_count = 0;
	
		startIndex = 0; //Reset the startIndex
		if(flush == 0) 
			flush = 1;
		while(true) 
		{
			count++;
			if(count == flush) 
			{ //Progress Log
				count = 0;
				flush_count++;
				m_progressCallback(flush_count*flush, m_totalHits);
			}
	
			if(!GetHitsFromFiles()) 
				break;
			outtree->Fill();
		}
	
		output->cd();
		outtree->Write(outtree->GetName(), TObject::kOverwrite);
		for(auto& entry : m_scaler_map)
			entry.second.Write();
	
		output->Close();
	}
	
	void CompassRun::Convert2SortedRoot(const std::string& name) 
	{
		TFile* output = TFile::Open(name.c_str(), "RECREATE");
		TTree* outtree = new TTree("SortTree", "SortTree");
	
		CoincEvent event;
		outtree->Branch("event", &event);
	
		if(!m_smap.IsValid()) 
		{
			EVB_WARN("Bad shift map ({0}) at CompassRun::Convert2SortedRoot(), shifts all set to 0.", m_smap.GetFilename());
		}
	
		SetScalers();
	
		if(!GetBinaryFiles()) 
		{
			EVB_ERROR("Unable to find binary files at CompassRun::Convert2SortedRoot(), exiting!");
			return;
		}
	
		unsigned int count = 0, flush = m_totalHits*m_progressFraction, flush_count = 0;
	
		startIndex = 0;
		SlowSort coincidizer(m_params.slowCoincidenceWindow, m_params.channelMapFile);
		bool killFlag = false;
		if(flush == 0) 
			flush = 1;
		while(true) 
		{
			count++;
			if(count == flush) 
			{
				count = 0;
				flush_count++;
				m_progressCallback(flush_count*flush, m_totalHits);
			}
	
			if(!GetHitsFromFiles()) 
			{
				coincidizer.FlushHitsToEvent();
				killFlag = true;
			} 
			else
				coincidizer.AddHitToEvent(m_hit);

			if(coincidizer.IsEventReady()) 
			{
				event = coincidizer.GetEvent();
				outtree->Fill();
				if(killFlag)
					break;
			}
		}
	
		output->cd();
		outtree->Write(outtree->GetName(), TObject::kOverwrite);
		for(auto& entry : m_scaler_map)
			entry.second.Write();
	
		coincidizer.GetEventStats()->Write();
		output->Close();
	}
	
	void CompassRun::Convert2FastSortedRoot(const std::string& name) 
	{
		TFile* output = TFile::Open(name.c_str(), "RECREATE");
		TTree* outtree = new TTree("SortTree", "SortTree");
	
		CoincEvent event;
		outtree->Branch("event", &event);
	
		if(!m_smap.IsValid()) 
		{
			EVB_WARN("Bad shift map ({0}) at CompassRun::Convert2FastSortedRoot(), shifts all set to 0.", m_smap.GetFilename());
		}
	
		SetScalers();
	
		if(!GetBinaryFiles()) 
		{
			EVB_ERROR("Unable to find binary files at CompassRun::Convert2FastSortedRoot(), exiting!");
			return;
		}
	
		unsigned int count = 0, flush = m_totalHits*m_progressFraction, flush_count = 0;
	
		startIndex = 0;
		CoincEvent this_event;
		std::vector<CoincEvent> fast_events;
		SlowSort coincidizer(m_params.slowCoincidenceWindow, m_params.channelMapFile);
		FastSort speedyCoincidizer(m_params.fastCoincidenceWindowSABRE, m_params.fastCoincidenceWindowIonCh);
	
		FlagHandler flagger;
	
		bool killFlag = false;
		if(flush == 0) 
			flush = 1;
		while(true) 
		{
			count++;
			if(count == flush) 
			{
				count = 0;
				flush_count++;
				m_progressCallback(flush_count*flush, m_totalHits);
			}
			
			if(!GetHitsFromFiles()) 
			{
				coincidizer.FlushHitsToEvent();
				killFlag = true;
			} 
			else 
			{
				flagger.CheckFlag(m_hit.board, m_hit.channel, m_hit.flags);
				coincidizer.AddHitToEvent(m_hit);
			}
	
			if(coincidizer.IsEventReady()) 
			{
				this_event = coincidizer.GetEvent();
	
				fast_events = speedyCoincidizer.GetFastEvents(this_event);
				for(auto& entry : fast_events) 
				{
					event = entry;
					outtree->Fill();
				}
				if(killFlag) 
					break;
			}
		}
	
		output->cd();
		outtree->Write(outtree->GetName(), TObject::kOverwrite);
		for(auto& entry : m_scaler_map)
			entry.second.Write();
		
		coincidizer.GetEventStats()->Write();
		output->Close();
	}
	
	
	void CompassRun::Convert2SlowAnalyzedRoot(const std::string& name) 
	{
	
		TFile* output = TFile::Open(name.c_str(), "RECREATE");
		TTree* outtree = new TTree("SPSTree", "SPSTree");
	
		ProcessedEvent pevent;
		outtree->Branch("event", &pevent);
	
		if(!m_smap.IsValid()) 
		{
			EVB_WARN("Bad shift map ({0}) at CompassRun::Convert2SlowAnalyzedRoot(), shifts all set to 0.", m_smap.GetFilename());
		}
	
		SetScalers();
	
		if(!GetBinaryFiles()) 
		{
			EVB_ERROR("Unable to find binary files at CompassRun::Convert2SlowAnalyzedRoot(), exiting!");
			return;
		}
	
		unsigned int count = 0, flush = m_totalHits*m_progressFraction, flush_count = 0;
	
		startIndex = 0;
		CoincEvent this_event;
		SlowSort coincidizer(m_params.slowCoincidenceWindow, m_params.channelMapFile);
		SFPAnalyzer analyzer(m_params.ZT, m_params.AT, m_params.ZP, m_params.AP, m_params.ZE, m_params.AE, m_params.beamEnergy, m_params.spsAngle, m_params.BField,m_params.nudge,m_params.Q);
	
		std::vector<TParameter<Double_t>> parvec;
		parvec.reserve(11); // went from 9 to 11 parameters -JCE June 2024
		parvec.emplace_back("ZT", m_params.ZT);
		parvec.emplace_back("AT", m_params.AT);
		parvec.emplace_back("ZP", m_params.ZP);
		parvec.emplace_back("AP", m_params.AP);
		parvec.emplace_back("ZE", m_params.ZE);
		parvec.emplace_back("AE", m_params.AE);
		parvec.emplace_back("Bfield", m_params.BField);
		parvec.emplace_back("BeamKE", m_params.beamEnergy);
		parvec.emplace_back("Theta", m_params.spsAngle);
		parvec.emplace_back("Nudge", m_params.nudge); // -JCE June 2024
		parvec.emplace_back("Q", m_params.Q); // -JCE June 2024
		
	
		bool killFlag = false;
		if(flush == 0) 
			flush = 1;
		while(true) 
		{
			count++;
			if(count == flush) 
			{
				count = 0;
				flush_count++;
				m_progressCallback(flush_count*flush, m_totalHits);
			}
	
			if(!GetHitsFromFiles()) 
			{
				coincidizer.FlushHitsToEvent();
				killFlag = true;
			} 
			else 
			{
				coincidizer.AddHitToEvent(m_hit);
			}
	
			if(coincidizer.IsEventReady()) 
			{
				this_event = coincidizer.GetEvent();
				pevent = analyzer.GetProcessedEvent(this_event);
				outtree->Fill();
				if(killFlag) 
					break;
			}
		}
	
		output->cd();
		outtree->Write(outtree->GetName(), TObject::kOverwrite);
		for(auto& entry : m_scaler_map)
			entry.second.Write();
	
		for(auto& entry : parvec)
			entry.Write();
	
		coincidizer.GetEventStats()->Write();
		analyzer.GetHashTable()->Write();
		analyzer.ClearHashTable();
		output->Close();
	}
	
	void CompassRun::Convert2FastAnalyzedRoot(const std::string& name) 
	{
	
		TFile* output = TFile::Open(name.c_str(), "RECREATE");
		TTree* outtree = new TTree("SPSTree", "SPSTree");
	
		ProcessedEvent pevent;
		outtree->Branch("event", &pevent);
	
		if(!m_smap.IsValid()) 
		{
			EVB_WARN("Bad shift map ({0}) at CompassRun::Convert2FastAnalyzedRoot(), shifts all set to 0.", m_smap.GetFilename());
		}
	
		SetScalers();
	
		if(!GetBinaryFiles()) 
		{
			EVB_ERROR("Unable to find binary files at CompassRun::Convert2FastAnalyzedRoot(), exiting!");
			return;
		}
	
		unsigned int count = 0, flush = m_totalHits*m_progressFraction, flush_count = 0;
	
		startIndex = 0;
		CoincEvent this_event;
		std::vector<CoincEvent> fast_events;
		SlowSort coincidizer(m_params.slowCoincidenceWindow, m_params.channelMapFile);
		FastSort speedyCoincidizer(m_params.fastCoincidenceWindowSABRE, m_params.fastCoincidenceWindowIonCh);
		SFPAnalyzer analyzer(m_params.ZT, m_params.AT, m_params.ZP, m_params.AP, m_params.ZE, m_params.AE, m_params.beamEnergy, m_params.spsAngle, m_params.BField, m_params.nudge, m_params.Q);
	
		std::vector<TParameter<Double_t>> parvec;
		parvec.reserve(11); // went from 9 to 11 parameters -JCE June 2024
		parvec.emplace_back("ZT", m_params.ZT);
		parvec.emplace_back("AT", m_params.AT);
		parvec.emplace_back("ZP", m_params.ZP);
		parvec.emplace_back("AP", m_params.AP);
		parvec.emplace_back("ZE", m_params.ZE);
		parvec.emplace_back("AE", m_params.AE);
		parvec.emplace_back("Bfield", m_params.BField);
		parvec.emplace_back("BeamKE", m_params.beamEnergy);
		parvec.emplace_back("Theta", m_params.spsAngle);
		parvec.emplace_back("Nudge", m_params.nudge); // -JCE June 2024
		parvec.emplace_back("Q", m_params.Q); // -JCE June 2024
		
	
		FlagHandler flagger;
	
		bool killFlag = false;
		if(flush == 0) 
			flush = 1;
		while(true) 
		{
			count++;
			if(count == flush) 
			{
				count = 0;
				flush_count++;
				m_progressCallback(flush_count*flush, m_totalHits);
			}
	
			if(!GetHitsFromFiles()) 
			{
				coincidizer.FlushHitsToEvent();
				killFlag = true;
			} 
			else 
			{
				flagger.CheckFlag(m_hit.board, m_hit.channel, m_hit.flags);
				coincidizer.AddHitToEvent(m_hit);
			}
	
			if(coincidizer.IsEventReady()) 
			{
				this_event = coincidizer.GetEvent();
	
				fast_events = speedyCoincidizer.GetFastEvents(this_event);
				for(auto& entry : fast_events) 
				{
					pevent = analyzer.GetProcessedEvent(entry);
					outtree->Fill();
				}
				if(killFlag) 
					break;
			}
		}
	
		output->cd();
		outtree->Write(outtree->GetName(), TObject::kOverwrite);
		for(auto& entry : m_scaler_map) 
			entry.second.Write();
	
		for(auto& entry : parvec)
			entry.Write();
	
		coincidizer.GetEventStats()->Write();
		analyzer.GetHashTable()->Write();
		analyzer.ClearHashTable();
		output->Close();
	}
}