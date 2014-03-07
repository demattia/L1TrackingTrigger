import os, sys

executableDir = "/uscms_data/d3/demattia/Upgrade/CMSSW_6_1_2_SLHC6_patch1/src/amsimulation/"
filesDir = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/612_SLHC6_MUBANK_low/"
# runDir = "/eos/uscms/store/user/lpcdve/noreplica/Upgrade/"

fileList = os.listdir(filesDir)

for rootFile in fileList:

    print filesDir+rootFile

    # Assuming that the last two characters before .root are the file number
    fileNumber = rootFile.strip(".root")[-2:]

    condorFile = open("condor_"+fileNumber, "w")
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

    scriptFile = open("job_"+fileNumber+".sh", "w")
    scriptFile.write("#!/bin/bash\n")
    scriptFile.write("cd "+executableDir+"\n")
    # scriptFile.write("cp amsimulation.cfg "+runDir+"\n")
    scriptFile.write("export SCRAM_ARCH=slc5_amd64_gcc462\n")
    scriptFile.write("eval `scramv1 runtime -sh`\n")
    # scriptFile.write("cd "+runDir+"\n")
    scriptFile.write("cd -\n")
    scriptFile.write(executableDir+"/Filter "+filesDir+"/"+rootFile+" "+fileNumber+"\n")
    scriptFile.close()

    os.system("condor_submit condor_"+fileNumber)
