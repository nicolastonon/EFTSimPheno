#NB : To inspect a particular rootfile, can do e.g. : edmDumpEventContent root://xrootd-cms.infn.it//store/user/ntonon/tllqdim6_FASTSIM1_v3/tllqdim6_FASTSIM1_v3/200316_201924/0000/FASTSIM1_1.root

#==================================================================
import FWCore.ParameterSet.Config as cms

# INIT PARAMETERS TO READ FILES
# //--------------------------------------------
myProcess = "tllq" #e.g. tzq

#/store/user/ntonon/tllqdim6_FASTSIM1_v3/tllqdim6_FASTSIM1_v3/200316_201924/0000/FASTSIM1_1.root

#--- Params needed if running on crab outputs
suffix = "dim6_FASTSIM1_v3" #e.g. dim6v1
crabProdDate = "200316_201924" #e.g. "191217_220027"
# crabProdDate = "200320_153657" #e.g. "191217_220027"
nofFiles = 1 #Number of files to read (reads xxx_1.root to xxx_N.root ; will ignore missing files)

# datatier = "GEN"
datatier = "AODSIM"
# //--------------------------------------------

process = cms.Process("GenAnalyzer")
process.GenAnalyzer = cms.EDAnalyzer("GenAnalyzer")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("myAnalyzer.GenAnalyzer.CfiFile_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.GenAnalyzer.myProcessName = cms.string(myProcess)

# process.GenAnalyzer.min_pt_jet  = cms.double(-1)
# process.GenAnalyzer.min_pt_lep  = cms.double(-1)
# process.GenAnalyzer.max_eta_jet = cms.double(-1)
# process.GenAnalyzer.max_eta_lep = cms.double(-1)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

# //--------------------------------------------
# //--------------------------------------------
# fileNamesList = cms.untracked.vstring("file:/afs/cern.ch/work/n/ntonon/public/Madgraph/CMSSW_9_3_4/src/crabDir/GEN_"+myProcess+".root")

# fileNamesList = cms.untracked.vstring("/store/user/ntonon/tllqdim6_v6/tllqdim6_v6/200210_113814/0000/GEN_1.root")

# fileNamesList = ["/store/user/ntonon/"+myProcess+"_"+suffix+"/"+myProcess+"_"+suffix+"/"+crabProdDate+"/0000/test_FASTSIM_"+str(i)+".root" for i in range(1, nofFiles+1)]

fileNamesList = ["/store/user/ntonon/"+myProcess+suffix+"/"+myProcess+suffix+"/"+crabProdDate+"/0000/FASTSIM1_inAODSIM_"+str(i)+".root" for i in range(1, nofFiles+1)]
# //--------------------------------------------
# //--------------------------------------------

# //--------------------------------------------
# //--------------------------------------------
#NB : 'file:' indicates a physical file path (i.e. no need to look in a catalog)
process.source = cms.Source("PoolSource",
    # fileNames = fileNamesList
    fileNames = cms.untracked.vstring(fileNamesList)
)
process.source.skipBadFiles = cms.untracked.bool(True) #Ignore missing files

# process.dump = cms.EDAnalyzer("EventContentAnalyzer")

# process.output = cms.OutputModule("PoolOutputModule",
#     fileName = cms.untracked.string('file:test.root')
# )

process.TFileService = cms.Service("TFileService", fileName=cms.string("output_"+myProcess+".root")) #Declare TFService

# //--------------------------------------------
# //--------------------------------------------

# //--------------------------------------------
# //--------------------------------------------
print "\n---------------------------"
print "\n" + "Datatier = " + datatier + "\n"
print "---------------------------\n"

if datatier == "GEN":
    process.GenAnalyzer.srcInput = cms.InputTag("source")
elif datatier == "AODSIM":
    process.GenAnalyzer.srcInput = cms.InputTag("externalLHEProducer")

if datatier == "GEN" or datatier == "AODSIM":
    process.GenAnalyzer.genParticlesInput = cms.InputTag("genParticles")
    process.GenAnalyzer.genJetsInput      = cms.InputTag("ak4GenJets")
else:
    print "[ERROR] Unknown datatier: {}" + datatier
    raise RuntimeError

# //--------------------------------------------
# //--------------------------------------------

process.p = cms.Path(
                    process.GenAnalyzer
                    )
