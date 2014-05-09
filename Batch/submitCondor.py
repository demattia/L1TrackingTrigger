import os

# Number of events per job and number of jobs
maxEvents = 50000
numJobs = 400
destinationDir = "/eos/uscms/store/user/lpcdve/noreplica/Upgrade/MuonGun/CentralSector/"

# Configuration of the particle gun. It produces a particle-antiparticle back-to-back pair
particleId = -13 # muon gun
maxOneOverPt = 1./2.
minOneOverPt = 1./200.
maxEta = 0.17
minEta = -1.85
maxPhi = 1.05
minPhi = -0.22

# This paramter decides whether the vertex is prompt or not.
# If false the *FlatSpread parameters are ignored.
spreadVertex = "False"
xFlatSpread = 0.
yFlatSpread = 0.
zFlatSpread = 0.


# Initial random seeds. They will be increased by 1 for each successive job.
genSeed = 1
vtxSmearedSeed = 2
g4SimHitsSeed = 3
mixSeed = 4


for index in range(1, numJobs+1):
    print "index =", index
    newCfgName = "FlatRandomOneOverPtMuonGun_cfg_"+str(index)+".py"
    outputFileName = "MuonGun_"+str(index)+".root"
    newCfg = open(newCfgName, "w")
    for line in open("FlatRandomOneOverPtMuonGun_cfg.py"):
        line = line.replace("MAX_EVENTS", str(maxEvents))
        line = line.replace("MAX_ONE_OVER_PT", str(maxOneOverPt))
        line = line.replace("MIN_ONE_OVER_PT", str(minOneOverPt))
        line = line.replace("MAX_ETA", str(maxEta))
        line = line.replace("MIN_ETA", str(minEta))
        line = line.replace("MAX_PHI", str(maxPhi))
        line = line.replace("MIN_PHI", str(minPhi))
        line = line.replace("SPREAD_VERTEX", spreadVertex)
        line = line.replace("X_FLAT_SPREAD", str(xFlatSpread))
        line = line.replace("Y_FLAT_SPREAD", str(yFlatSpread))
        line = line.replace("Z_FLAT_SPREAD", str(zFlatSpread))
        line = line.replace("PARTICLE_ID", str(particleId))
        line = line.replace("GENERATOR_SEED", str(genSeed+index))
        line = line.replace("VTXSMEARED_SEED", str(vtxSmearedSeed+index))
        line = line.replace("G4SIMHITS_SEED", str(g4SimHitsSeed+index))
        line = line.replace("MIX_SEED", str(mixSeed+index))
        line = line.replace("OUTPUT_FILE", outputFileName)
        newCfg.write(line)
    newCfg.close()

    scriptFileName = "runJob_"+str(index)
    scriptFile = open(scriptFileName, "w")
    scriptFile.write("""
cd /uscms/home/demattia/d3/Upgrade/CMSSW_6_2_0_SLHC12/src/
eval `scram runtime -sh`
cd -
cmsRun """+newCfgName+"""
cp """+outputFileName+" "+destinationDir+""" 
rm """+outputFileName+"""
"""
)
    scriptFile.close()

    condorFileName = "condor_"+str(index)
    condorFile = open(condorFileName, "w")
    condorFile.write("""
universe = vanilla
Executable = """+scriptFileName+"""
Requirements = Memory >= 199 &&OpSys == \"LINUX\"&& (Arch != \"DUMMY\" )&& Disk > 1000000
Should_Transfer_Files = YES
Transfer_Input_Files = """+newCfgName+"""
Output = MuonGun_"""+str(index)+"""_$(Cluster)_$(Process).stdout
Error = MuonGun_"""+str(index)+"""_$(Cluster)_$(Process).stderr
Log = MuonGun_"""+str(index)+"""_$(Cluster)_$(Process).log
notify_user = ${LOGNAME}@FNAL.GOV
Queue 1
"""
)
    condorFile.close()


    print "condor_submit "+condorFileName
    os.system("condor_submit "+condorFileName)
