#==================================================================
import FWCore.ParameterSet.Config as cms

# USER PARAMETERS
# //--------------------------------------------
myProcess = "tllq" #e.g. tzq or ttll

#--- for crab prod only (fastSim, ...)
# crabProdDate = "191217_220027" #e.g. "191217_220027"
# suffix = "dim6_v3" #e.g. dim6v1
# nofFiles = 20 #Number of files to read

datatier = "GEN"
# datatier = "AODSIM"
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
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

fileNamesList = cms.untracked.vstring("file:/afs/cern.ch/work/n/ntonon/public/Madgraph/CMSSW_10_2_18/src/lhe.root") #UPDATE PATH

#fileNamesList = ["/store/user/ntonon/"+myProcess+suffix+"/"+myProcess+suffix+"/"+crabProdDate+"/0000/test_FASTSIM_"+str(i)+".root" #for i in range(1, nofFiles+1)]

# -----------------------------
#NB : 'file:' indicates a physical file path (i.e. no need to look in a catalog)
process.source = cms.Source("PoolSource",
    # fileNames = fileNamesList
    fileNames = cms.untracked.vstring(fileNamesList)
)
# process.dump = cms.EDAnalyzer("EventContentAnalyzer")

print "\n---------------------------"
print "\n" + "Datatier = " + datatier + "\n"
print "---------------------------\n"

if datatier == "GEN":
    process.GenAnalyzer.genParticlesInput = cms.InputTag("genParticles")
    process.GenAnalyzer.genJetsInput      = cms.InputTag("ak4GenJets")
elif datatier == "AODSIM":
    process.GenAnalyzer.genParticlesInput = cms.InputTag("prunedGenParticles")
    process.GenAnalyzer.genJetsInput      = cms.InputTag("slimmedGenJets")
else:
    print "[ERROR] Unknown datatier: {}" + datatier
    raise RuntimeError
# ---------------------------

process.p = cms.Path(
process.GenAnalyzer
)
