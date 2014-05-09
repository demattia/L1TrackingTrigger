# Auto generated configuration file
# using: 
# Revision: 1.20 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: FourMuPt_1_200_cfi --conditions auto:upgradePLS3 -n 10 --eventcontent FEVTDEBUG --relval 10000,100 -s GEN,SIM,DIGI:pdigi_valid,L1,L1TrackTrigger --datatier GEN-SIM-DIGI --beamspot HLLHC --customise SLHCUpgradeSimulations/Configuration/combinedCustoms.cust_2023TTI --geometry Extended2023TTI,Extended2023TTIReco --magField 38T_PostLS1 --no_exec --fileout file:output.root --python_filename=FlatRandomOneOverPtMuonGun_cfg.py
import FWCore.ParameterSet.Config as cms

process = cms.Process('L1TrackTrigger')

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.Geometry.GeometryExtended2023TTIReco_cff')
process.load('Configuration.Geometry.GeometryExtended2023TTI_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_PostLS1_cff')
process.load('Configuration.StandardSequences.Generator_cff')
# process.load('IOMC.EventVertexGenerators.VtxSmearedHLLHC_cfi')
process.load('Configuration.StandardSequences.VtxSmearedNoSmear_cff')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.SimL1Emulator_cff')
process.load('Configuration.StandardSequences.L1TrackTrigger_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(MAX_EVENTS)
)

# Input source
process.source = cms.Source("EmptySource")

process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    version = cms.untracked.string('$Revision: 1.20 $'),
    annotation = cms.untracked.string('FourMuPt_1_200_cfi nevts:10'),
    name = cms.untracked.string('Applications')
)

# Output definition

process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    splitLevel = cms.untracked.int32(0),
    eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
    outputCommands = process.FEVTDEBUGEventContent.outputCommands,
    fileName = cms.untracked.string('file:OUTPUT_FILE'),
    dataset = cms.untracked.PSet(
        filterName = cms.untracked.string(''),
        dataTier = cms.untracked.string('GEN-SIM-DIGI')
    ),
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    )
)

# Additional output definition

# Other statements
process.genstepfilter.triggerConditions=cms.vstring("generation_step")
process.mix.digitizers = cms.PSet(process.theDigitizersValid)
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:upgradePLS3', '')

# Random seeds
process.RandomNumberGeneratorService.generator.initialSeed      = GENERATOR_SEED
process.RandomNumberGeneratorService.VtxSmeared.initialSeed     = VTXSMEARED_SEED
process.RandomNumberGeneratorService.g4SimHits.initialSeed      = G4SIMHITS_SEED
process.RandomNumberGeneratorService.mix.initialSeed            = MIX_SEED

process.generator = cms.EDProducer("FlatRandomOneOverPtGunProducer",
				   PGunParameters = cms.PSet(
	MaxOneOverPt = cms.double(MAX_ONE_OVER_PT),
	MinOneOverPt = cms.double(MIN_ONE_OVER_PT),
	PartID = cms.vint32(PARTICLE_ID),
	MaxEta = cms.double(MAX_ETA),
	MaxPhi = cms.double(MAX_PHI),
	MinEta = cms.double(MIN_ETA),
	MinPhi = cms.double(MIN_PHI),
	# This paramter decides whether the vertex is prompt or not.
	# If false the *FlatSpread parameters are ignored.
	SpreadVertex = cms.bool(SPREAD_VERTEX),
	XFlatSpread = cms.double(X_FLAT_SPREAD),
	YFlatSpread = cms.double(Y_FLAT_SPREAD),
	ZFlatSpread = cms.double(Z_FLAT_SPREAD)
	),
	Verbosity = cms.untracked.int32(0),
	psethack = cms.string('Four mu pt 1 to 200'),
	AddAntiParticle = cms.bool(True),
	firstRun = cms.untracked.uint32(1)
)

# Path and EndPath definitions
process.generation_step = cms.Path(process.pgen)
process.simulation_step = cms.Path(process.psim)
process.digitisation_step = cms.Path(process.pdigi_valid)
process.L1simulation_step = cms.Path(process.SimL1Emulator)
process.L1TrackTrigger_step = cms.Path(process.L1TrackTrigger)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

# Schedule definition
process.schedule = cms.Schedule(process.generation_step,process.genfiltersummary_step,process.simulation_step,process.digitisation_step,process.L1simulation_step,process.L1TrackTrigger_step,process.endjob_step,process.FEVTDEBUGoutput_step)
# filter all path with the production filter sequence
for path in process.paths:
	getattr(process,path)._seq = process.generator * getattr(process,path)._seq 

# customisation of the process.

# Automatic addition of the customisation function from SLHCUpgradeSimulations.Configuration.combinedCustoms
from SLHCUpgradeSimulations.Configuration.combinedCustoms import cust_2023TTI 

#call to customisation function cust_2023TTI imported from SLHCUpgradeSimulations.Configuration.combinedCustoms
process = cust_2023TTI(process)

# End of customisation functions
