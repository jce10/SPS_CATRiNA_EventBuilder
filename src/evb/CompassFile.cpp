/*
    CompassFile.cpp
    Wrapper class around a shared pointer to an ifstream. The shared pointer is used
    to overcome limitations of ifstream, especially that it cannot be moved. This class
    parses a binary CompassFile and extracts relevant data like board/channel numbers,
    timestamps, energy, flags, and more.
    
    Written by G.W. McCann Oct. 2020
*/

#include "CompassFile.h"

namespace EventBuilder {

    // Default constructor initializes class members.
    CompassFile::CompassFile() :
        m_filename(""), m_bufferIter(nullptr), m_bufferEnd(nullptr), m_smap(nullptr), m_hitUsedFlag(true), m_hitsize(0), m_buffersize(0),
        m_file(std::make_shared<std::ifstream>()), m_eofFlag(false)
    {
    }

    // Constructor that opens a file and initializes parameters.
    CompassFile::CompassFile(const std::string& filename) :
        m_filename(""), m_bufferIter(nullptr), m_bufferEnd(nullptr), m_smap(nullptr), m_hitUsedFlag(true), m_hitsize(0), m_buffersize(0),
        m_file(std::make_shared<std::ifstream>()), m_eofFlag(false)
    {
        Open(filename);
    }

    // Constructor that takes a filename and buffer size.
    CompassFile::CompassFile(const std::string& filename, int bsize) :
        m_filename(""), m_bufferIter(nullptr), m_bufferEnd(nullptr), m_smap(nullptr), m_hitUsedFlag(true), m_bufsize(bsize), m_hitsize(0),
        m_buffersize(0), m_file(std::make_shared<std::ifstream>()), m_eofFlag(false)
    {
        Open(filename);
    }

    // Destructor that ensures the file is closed when the object is destroyed.
    CompassFile::~CompassFile() 
    {
        Close();
    }

    // Open the specified Compass file. Reads the header and initializes parameters.
    void CompassFile::Open(const std::string& filename) 
    {
        m_eofFlag = false;
        m_hitUsedFlag = true;
        m_filename = filename;
        m_nHits = 0;
        m_file->open(m_filename, std::ios::binary | std::ios::in); // Open file in binary mode
        m_file->seekg(0, std::ios_base::end); // Seek to the end of the file
        m_size = m_file->tellg(); // Get file size
        
        if (m_size == 2) 
        {
            m_eofFlag = true; // If file size is 2, it indicates EOF
        } 
        else 
        {
            m_file->seekg(0, std::ios_base::beg); // Seek to the beginning of the file
            ReadHeader(); // Read file header to configure hit size and other parameters
            m_nHits = m_size / m_hitsize; // Calculate number of hits
            m_buffersize = m_hitsize * m_bufsize; // Calculate buffer size
            m_hitBuffer.resize(m_buffersize); // Resize the hit buffer
        }
    }

    // Close the file stream if it is open.
    void CompassFile::Close() 
    {
        if (IsOpen()) 
        {
            m_file->close();
        }
    }

    // Read the header from the file to determine hit size and other properties.
    void CompassFile::ReadHeader() 
    {
        if (!IsOpen()) 
        {
            EVB_WARN("Unable to read header from file. State not validated", m_filename);
            return;
        }

        char* header = new char[2];
        m_file->read(header, 2); // Read the first 2 bytes for header
        m_header = *((uint16_t*)header); // Interpret header as 16-bit value
        m_hitsize = 16; // Default hit size is 16 bytes

        // Adjust hit size based on certain flags (energy, calibration, etc.)
        if (IsEnergy()) m_hitsize += 2;
        if (IsEnergyCalibrated()) m_hitsize += 8;
        if (IsEnergyShort()) m_hitsize += 2;
        if (IsWaves())
        {
            EVB_ERROR("Waveforms are not supported by the SPS_SABRE_EventBuilder. The wave data will be skipped.");
            m_hitsize += 5;
            char* firstHit = new char[m_hitsize]; // Allocate memory for a hit
            m_file->read(firstHit, m_hitsize); // Read first hit data
            firstHit += m_hitsize - 4;
            uint32_t nsamples = *((uint32_t*) firstHit); // Get number of waveform samples
            m_hitsize += nsamples * 2; // Adjust hit size for waveform samples
            m_file->seekg(0, std::ios_base::beg); // Seek back to beginning
            m_file->read(header, 2); // Read header again
            delete[] firstHit; // Cleanup memory
        }

        delete[] header; // Clean up header memory
    }

    /*
        GetNextHit() retrieves the next hit from the file.
        It refills the buffer if necessary, and marks the hit as unused for the next level.
        Returns true if EOF is reached, false otherwise.
    */
    bool CompassFile::GetNextHit()
    {
        if (!IsOpen()) return true;

        // If buffer is empty or reached end, fetch next buffer
        if ((m_bufferIter == nullptr || m_bufferIter == m_bufferEnd) && !IsEOF()) 
        {
            GetNextBuffer();
        }

        if (!IsEOF()) 
        {
            ParseNextHit(); // Parse the next hit
            m_hitUsedFlag = false; // Mark hit as used
        }

        return m_eofFlag; // Return EOF flag status
    }

    /*
        GetNextBuffer() reads the next buffer from the file and sets the buffer iterators.
        Signals EOF after the last buffer is read completely.
    */
    void CompassFile::GetNextBuffer() 
    {
        if (m_file->eof()) 
        {
            m_eofFlag = true; // Set EOF flag when end of file is reached
            return;
        }

        m_file->read(m_hitBuffer.data(), m_hitBuffer.size()); // Read next buffer of hits
        m_bufferIter = m_hitBuffer.data(); // Set buffer iterator to the start of the buffer
        m_bufferEnd = m_bufferIter + m_file->gcount(); // Set buffer end iterator (one past the last byte)
    }

    // Parse the next hit from the buffer and extract relevant data
    void CompassFile::ParseNextHit() 
    {
        m_currentHit.board = *((uint16_t*)m_bufferIter); // Read board ID
        m_bufferIter += 2;
        m_currentHit.channel = *((uint16_t*)m_bufferIter); // Read channel ID
        m_bufferIter += 2;
        m_currentHit.timestamp = *((uint64_t*)m_bufferIter); // Read timestamp
        m_bufferIter += 8;

        // Parse additional fields based on flags (Energy, Calibration, etc.)
        if (IsEnergy())
        {
            m_currentHit.energy = *((uint16_t*)m_bufferIter); // Read energy
            m_bufferIter += 2;
        }
        if (IsEnergyCalibrated())
        {
            m_currentHit.energyCalibrated = *((uint64_t*)m_bufferIter); // Read calibrated energy
            m_bufferIter += 8;
        }
        if (IsEnergyShort())
        {
            m_currentHit.energyShort = *((uint16_t*)m_bufferIter); // Read short energy
            m_bufferIter += 2;
        }
        m_currentHit.flags = *((uint32_t*)m_bufferIter); // Read flags (e.g., PSD/PHA settings)
        m_bufferIter += 4;

        // Handle waveform data (if present)
        if (IsWaves())
        {
            m_currentHit.waveCode = *((uint8_t*)m_bufferIter); // Read wave code
            m_bufferIter += 1;
            m_currentHit.Ns = *((uint32_t*)m_bufferIter); // Read number of samples
            m_bufferIter += 4;
            m_bufferIter += 2 * m_currentHit.Ns; // Skip waveform data
        }

        // Apply channel shift if shift map is provided
        if (m_smap != nullptr) 
        { 
			// memory safety
            int gchan = m_currentHit.channel + m_currentHit.board * 16;
            m_currentHit.timestamp += m_smap->GetShift(gchan); // Apply shift based on channel
        }
    }

}
