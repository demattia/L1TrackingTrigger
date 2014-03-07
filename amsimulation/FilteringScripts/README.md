Scripts to Parallelize Track Filtering
--------------------------------------

The script filterAllCondor.py will create the needed files and submit condor jobs for filtering the tracks.
The jobs will run and produce their output in the local directory.

The script has three parameters that can be changed (at the moment, the script must be edited):
* executableDir: location of the Filter executable. This is the amsimulation/ folder.
* filesDir: location where the original root files are located

The script will find all the root files in the filesDir and submit a Filter condor job for each of them.

**Note**: the amsimulation.cfg must be in the same directory where the script is executed.
