import os, sys

# Important configuration
# -----------------------
executableDir = "/uscms_data/d3/demattia/Upgrade/CMSSW_6_1_2_SLHC6_patch1/src/amsimulation/"
sectorFile = "/eos/uscms/store/user/lpcdve/noreplica/Upgrade/trigger_sector_map.csv"
# -----------------------


# Function to perform the filtering
# ---------------------------------
def filterTracks(sectorId, activeLayers, runDir, filesDir, fileList):

    # Create the amsimulation.cfg
    # ---------------------------
    configFile = open(runDir+"/amsimulation.cfg", "w")
    configFile.write("""
#Number of strips in a superstrip {16,32,64,128,256,512,1024}
ss_size=32
#Number of DC bits to use [0-3]
dc_bits=3
#Minimal PT of tracks used to generate a pattern
pt_min=2
#Maximal PT of tracks used to generate a pattern
pt_max=100
#Minimal ETA of tracks used to generate a pattern
eta_min=-2.5
#Maximal ETA of tracks used to generate a pattern
eta_max=2.5
#Maximal number of fake superstrips per pattern (used for barrel/endcap sectors)
maxFakeSStrips=3
#Directory containing root files with single muon/antimuon events (local or RFIO)
input_directory=/uscms_data/d3/demattia/Upgrade/CMSSW_6_1_2_SLHC6_patch1/src/SmallFiles/
#Output file name
bank_name=testOutput.pbk
#Coverage [0-1]
coverage=0.00001
#Root file containing sectors definitions
""")
    configFile.write("sector_file="+sectorFile+"\n")
    configFile.write("#Index of the sector to be used\n")
    configFile.write("sector_id="+sectorId+"\n")
    configFile.write("#Layers used\n")
    configFile.write("active_layers="+activeLayers)
    configFile.close()

    # Submit filtering for all the files
    # ----------------------------------
    for rootFile in fileList:

        print "Filtering file: "+filesDir+rootFile

        # Assuming that the last two characters before .root are the file number
        fileNumber = rootFile.strip(".root")[-2:]

        condorFile = open(runDir+"/condor_"+fileNumber, "w")
        condorFile.write("universe = vanilla\n")
        condorFile.write("Executable = job_"+fileNumber+".sh\n")
        condorFile.write("Requirements = Memory >= 199 &&OpSys == \"LINUX\"&& (Arch != \"DUMMY\" )&& Disk > 1000000\n")
        condorFile.write("Should_Transfer_Files = YES\n")
        condorFile.write("WhenToTransferOutput = ON_EXIT\n")
        condorFile.write("Transfer_Input_Files = job_"+fileNumber+".sh\n")
        condorFile.write("Output = job_$(Cluster)_$(Process).stdout\n")
        condorFile.write("Error = job_$(Cluster)_$(Process).stderr\n")
        condorFile.write("Log = job_$(Cluster)_$(Process).log\n")
        condorFile.write("Queue 1\n")
        condorFile.close()

        scriptFile = open(runDir+"/job_"+fileNumber+".sh", "w")
        scriptFile.write("#!/bin/bash\n")
        scriptFile.write("cd "+executableDir+"\n")
        scriptFile.write("export SCRAM_ARCH=slc5_amd64_gcc462\n")
        scriptFile.write("eval `scramv1 runtime -sh`\n")
        scriptFile.write("cd "+runDir+"\n")
        scriptFile.write(executableDir+"/Filter "+filesDir+"/"+rootFile+" "+fileNumber+"\n")
        scriptFile.close()

        # Submit to condor
        os.system("cd "+runDir+"; condor_submit condor_"+fileNumber)


# Function to filter all files for all sectors
# --------------------------------------------
def filterAllFiles(sectorIdList, activeLayersList, filesDir):
    fileList = os.listdir(filesDir)
    for sectorId, activeLayers in zip(sectorIdList, activeLayersList):
        fileType = filesDir.split("_")[-1]
        runDir = "/eos/uscms/store/user/lpcdve/noreplica/Upgrade/Sector_"+sectorId+"_"+fileType+"/"
        os.system("mkdir -p "+runDir)
        print ""
        print ""
        print "Filtering "+fileType.strip("/")+" pt tracks for sector Id "+sectorId+" with active layers: "+activeLayers
        print ""
        filterTracks(sectorId, activeLayers, runDir, filesDir, fileList)


# -------------------------
# Define sectors and layers
# -------------------------

sectorIdList = []
activeLayersList = []

sectorIdList.append("0")
activeLayersList.append("5 6 18 19 20 21 22")

sectorIdList.append("8")
activeLayersList.append("6 7 8 9 10 18 19 20")

sectorIdList.append("16")
activeLayersList.append("5 6 7 8 9 10")

sectorIdList.append("24")
activeLayersList.append("5 6 7 8 9 10")

sectorIdList.append("32")
activeLayersList.append("6 7 8 9 10 11 12 13")

sectorIdList.append("40")
activeLayersList.append("5 6 11 12 13 14 15")


# -----------------
# Run the filtering
# -----------------

# Filter low pt tracks
# --------------------
filesDirLowPt = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/612_SLHC6_MUBANK_low/"
filterAllFiles(sectorIdList, activeLayersList, filesDirLowPt)


# Filter mid pt tracks
# --------------------
filesDirMidPt = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/612_SLHC6_MUBANK_mid/"
filterAllFiles(sectorIdList, activeLayersList, filesDirMidPt)


# Filter high pt tracks
# --------------------
filesDirHigPt = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/612_SLHC6_MUBANK_hig/"
filterAllFiles(sectorIdList, activeLayersList, filesDirHigPt)

