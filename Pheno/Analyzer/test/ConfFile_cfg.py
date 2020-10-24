#NB : To inspect a particular rootfile, can do e.g. : edmDumpEventContent root://xrootd-cms.infn.it//store/user/ntonon/tllqdim6_FASTSIM1_v3/tllqdim6_FASTSIM1_v3/200316_201924/0000/FASTSIM1_1.root

#==================================================================
import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils

# INIT PARAMETERS TO READ FILES
# //--------------------------------------------
myProcess = "tllq" #e.g. ttll

#--- Params needed if running on crab outputs
suffix = "test" #e.g. dim6v1
dir_suffix = "LHEGENSIM" #Suffix added to dir. only
crabProdDate = "200923_135421" #e.g. "191217_220027"
nofFiles = 100 #Number of files to read (reads xxx_1.root to xxx_N.root ; will ignore missing files)

datatiername = "LHE-GEN-SIM"
#datatiername = "FASTSIM1_inAODSIM"
#datatiername = "miniAOD"
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

fileNamesList = ["/store/user/ntonon/"+myProcess+suffix+"_"+dir_suffix+"/"+myProcess+suffix+"_"+dir_suffix+"/"+crabProdDate+"/0000/"+datatiername+"_"+str(i)+".root" for i in range(1, nofFiles+1)]

mylist = FileUtils.loadListFromFile('~/files_tZq_central.txt')

# //--------------------------------------------
# //--------------------------------------------

# //--------------------------------------------
# //--------------------------------------------
#NB : 'file:' indicates a physical file path (i.e. no need to look in a catalog)
process.source = cms.Source("PoolSource",
    #fileNames = cms.untracked.vstring(fileNamesList) #List defined above
    fileNames = cms.untracked.vstring(*mylist) #List defined in external txt file
)
process.source.skipBadFiles = cms.untracked.bool(True) #Ignore missing files
# process.dump = cms.EDAnalyzer("EventContentAnalyzer")
# process.output = cms.OutputModule("PoolOutputModule",
#     fileName = cms.untracked.string('file:test.root')
# )

process.TFileService = cms.Service("TFileService", fileName=cms.string("output_"+myProcess+suffix+".root")) #Declare TFService

# //--------------------------------------------
# //--------------------------------------------

# //--------------------------------------------
# //--------------------------------------------
print "\n---------------------------"
print "\n" + "datatiername = " + datatiername + "\n"
print "---------------------------\n"

if datatiername is "miniAOD": #Use RECO collections ?
	process.GenAnalyzer.genParticlesInput = cms.InputTag("prunedGenParticles")
	process.GenAnalyzer.genJetsInput      = cms.InputTag("slimmedGenJets")
else: 
	process.GenAnalyzer.genParticlesInput = cms.InputTag("genParticles")
	process.GenAnalyzer.genJetsInput      = cms.InputTag("ak4GenJets")

# process.GenAnalyzer.srcInput = cms.InputTag("source")
process.GenAnalyzer.srcInput = cms.InputTag("externalLHEProducer")

# print "[ERROR] Unknown datatiername: {}" + datatiername
# raise RuntimeError

# //--------------------------------------------
# //--------------------------------------------

process.p = cms.Path(process.GenAnalyzer)
