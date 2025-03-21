# SPS-CATRiNA Data Analysis Package
Version 4
This is a software package designed to help experimenters analyze data from SPS-SABRE at FSU. 
It can convert CoMPASS data to ROOT, sort the data in time, build events, perform preliminary analysis, and provide basic plots.

## Installation
To build and install the event builder, the CMake build system is used. To build, simply run the following commands from the SPS_SABRE_EventBuilder directory:
```
mkdir build
cd build
cmake ..
make
```

To clone the repository use `git clone --recursive https://github.com/sesps/SPS_SABRE_EventBuilder.git`. The recursive flag is important; this tells github to pull all submodules associated with the repository. 

The binaries are installed to the `bin` directory of the event builder, and should be run from the event builder directory (i.e. `./bin/EventBuilderGui`). THe `bin` directory also contains a shell script named `archivist` for transferring data from a CoMPASS project to the event builder workspace.

In general, one should only build for Release (this is the default), for maximum optimization. However, it can be useful to run in Debug (change the cmake command to `cmake -DCMAKE_BUILD_TYPE=Debug ..`) when testing new features.

## EventBuilder Set-Up 
There are files that must be edited in order to successfully start building events and histograms for analysis. 
1. archivist - found in the 'bin/' directory. give paths to raw binary from CAEN digitizers. ***depending on the version of CoMPASS, could be '.bin' or '.BIN'
2. channel map - mapping of channel/board of detectors.
3. cut list - provides path to root CutG objects used for applying cuts in focal plane histograms.
4. scaler - setting counting parameters.
5. shift map -  
6. input file - modify the paths to all of the evb files listed above. the input file for SPS-CAT has been modified to include additional kinematic correction  inputs for the user. 

## EventBuilder vs. EventBuilderGui
There are two programs provided. They are `EventBuilderGui` and `EventBuilder`. The first is a full GUI version of the event builder. The GUI supports all conversion methods and the plotting tool.

### Building Events
The event building operation is the bulk of the analysis process. As files are being converted to ROOT from the raw CoMPASS binary, events are built using information given by the user. 

#### Types of Event Building
1. Convert: simply sends data from CoMPASS format to ROOT format and time orders the data.
2. Slow Events: This perfoms the event building of slow events and then analyzes the slow data. Note that in this option, if there are unresolved multiplicities in data, the analyzer assumes the earliest datum is relevant one.
3. Fast Events: This performs the event building of fast events, assuming that slow event data has already been created and EXISTS in the proper directory. The fast event data is then analyzed.
4. Analyze Slow Events: This performs analysis of slow event data, without performing any fast sorting.
5. Analyze Fast Events: This performs analysis of fast event data.
 
#### Slow Sorting
The first stage is slow sorting the shifted data by timestamp and orgainizing detector hits into large data events. Events are structures which contain all detector hits that occur within a given coincidence window with physical detector variables. This event data is then written to an output file. The goal of the slow sorting is to be as general as possible; slow sorting should change very little on a data set to data set basis, as this coincidence window is limited mostly be the time difference between an anode hit and the maximum delay line time if the correct shifts are applied to SABRE and the scintillator.

#### Fast Sorting
This is basically a secondary tier of event building, that is more user specific. It breaks down data within the coincidence window into single focal plane events with asscoiated SABRE data. The principle is that the scintillator provides very sharp timing resolution by which we can further refine the built event. Currently, `FastSort` is desinged to take two windows: a coincidence window for SABRE and the scintillator, and a coincidence window for the ion chamber and the scintillator. For the ion chamber, the back anode was chosen to be the representative (it really doesn't matter which part of the ion chamber is chosen). SABRE data is additionally filtered to contain only paired hits (hits that have both a ring and a wedge). Fast sorting is where the user will have to make the most changes to the actual event building. Any new detector or additional changes will require more coincidence definitions and sorting depth.

#### Analyzing
Finally, the sorted event data is then converted into meaningful physical data, and saved to a  final analyzed file. This is where the digitizer parameters (charge/energy, time, etc.) are converted into the actual paramters of interest such as focal plane position, SABRE energy, etc. In this way,  each raw data file gives four output files from the analysis: a shifted file, a slow sorted file, a fast sorted file, and an analyzed file. The rationale behind the repetative writting is that it helps the user isolate at which stage data issues occur at; this is especially useful for the shifting and sorting stages, where the values for the shifts and coincidence window have to be estimated by the user before running. 

All of the user input is handled through an input file in the program directory named `input.txt`. This file is preformated; all the user needs to do is update the names and values. Everything from input and output directories, to shifts and coincidence windows should be specified in this file. Note that directorires should be explicit full paths.

See the Plotter section for advice on which histograms are useful for choosing the correct shifts and window sizes for the data set.

### Merging
The program is capable of merging several root files together using either `hadd` or the ROOT TChain class. Currently, only the TChain version is implemented in the API, however if you want the other method, it does exist in the RunCollector class.

### Plotting
The plotting is intended to be the final leg of the analysis pipeline. The goal of this programis to take a collection of analyzed files and produce a file containing relevant histograms, graphs, and other such data measures. As it is currently built, this program has no ability to save any data of its own, it merely makes data measures. It is a quick and dirty analysis, and is not intended to be increased beyond merely checking some TCutGs and making some histograms. Cuts can be applied using a cut list. The cut list should contain a name for the cut, the name of the file containing the TCutG ROOT object (named CUTG), and then names for the x and y variables. The x and y variables must be initialized in the variable map. By default x1, x2, xavg, scintLeft, anodeBack, and cathode are all initialized. Any other variables will have to be added by the user by modifiying the CutHandler::InitVariableMap() function. 

#### Determining Shifts and Windows
The plotting already provides most of the histograms one would need to determine the shifts and windows for a data set. These, in general, come from plots of the relative time of various components of the detector. The goal of the scintillator and si shifts are to make them occur in coincidence with the anode (pick one of the focal plane anodes, they occur at essentially the same time). Included automatically are plots of the back anode relative to the scintillator (anodeB.Time-scintL.Time, gives scint offset), the is relative to the scint (SABRE fronts and backs... pick higher res one to make offsets and shifts), and maximum delay times relative to scint for both lines.

The method is the following:

Using the anode relative to the scint, one can determine the scint offset (center the peak on 0). Then, by looking at the SABRE relative to scint plots one can determine the shift for si and the fast window size (again center the peak on 0, the width of the peak becomes the fast window). Finally, if everything goes according to plan, now the maximum size of the slow coincidence window will be the relative time of the maximum delay line signal. Look at the plot of this and determine where you want to cut off. Run it again and check the results. You should look for, in general, reduced background and noise along with correct centering of the timing peaks.

### Scaler Support
Currently the pipeline supports declaring individual digitizer channels as scalers. These channels will be used a pure counting measures. To make a channel a scaler, put the CoMPASS formated name of the channel and board (check the given etc/ScalerFile.txt for an example) in a text file along with a parameter name for the scaler to be saved as. These files are then processed outside of the event building loop, which can greatly increase the computational speed. Future versions will include scaler rates as well.

## CATRiNA Implementation


## Details
For more information see the [wiki](https://github.com/sesps/SPS_SABRE_EventBuilder/wiki), which describes the app in much more detail.
Additionally, check the [FAQ](https://github.com/sesps/SPS_SABRE_EventBuilder/wiki/FAQ) if you're having trouble with something.

## System Requirements
- Requires C++17
- Requires ROOT >= 6.22.04 for C++17
- Requires CMake >= 3.16
- This version is for data from CAEN CoMPASS >= 2.0. Data from older CoMPASS versions are not compatible.
