Scripts to Parallelize Track Filtering
--------------------------------------

The script filterAllCondor.py will create the needed files and submit the condor jobs for filtering the tracks.
The jobs will run and produce their output in subdirectors with names Sector_ID_TYPE, where ID is the sector id number and TYPE is "low", "mid" or "hig" and denotes the type of tracks used.

The script has three parameters that can be changed (at the moment, the script must be edited):
* executableDir: location of the Filter executable. This is the amsimulation/ folder.
* sectorFile: location of the sector definition file.
* filesDirLowPt, filesDirMidPt, filesDirHigPt: are the locations of the low, mid and high pt input files.

The script will find all the root files in the filesDir*Pt directories and submit a Filter condor job for each of them.

The layers associated to each sector are embedded in the script.
The amsimulation.cfg is also embedded in the script and additional parameters can be changed therein.