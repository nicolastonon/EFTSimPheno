'''
A config file consists (typically) of the following parts as data members of a "cms.Process" object (of your naming):

A source (which might read Events from a file or create new empty events)
A collection of modules (e.g. EDAnalyzer, EDProducer, EDFilter) which you wish to run, along with customised settings for parameters you wish to change from default values
An output module to create a ROOT file which stores all the event data. (When running an Analyzer module, the histograms produced are not event data, so an output module is not needed in that case)
A path which will list in order the modules to be run
'''
#==================================================================
import FWCore.ParameterSet.Config as cms

process = cms.Process("GenAnalyzer")

# process.GenAnalyzer = cms.EDAnalyzer("GenAnalyzer")

datatier = "GEN"

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("myAnalyzer.GenAnalyzer.CfiFile_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.GenAnalyzer.min_pt_jet  = cms.double(-1)
process.GenAnalyzer.min_pt_lep  = cms.double(-1)
process.GenAnalyzer.max_eta_jet = cms.double(-1)
process.GenAnalyzer.max_eta_lep = cms.double(-1)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# -----------------------------
#NB : 'file:' indicates a physical file path (i.e. no need to look in a catalog)
process.source = cms.Source("PoolSource",
    # fileNames = cms.untracked.vstring("file:/afs/cern.ch/user/n/ntonon/tmp/test_GEN-SIM.root")
    fileNames = cms.untracked.vstring("/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_1.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_2.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_3.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_4.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_5.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_6.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_7.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_8.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_9.root",
        # "/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191211_151247/0000/test_GEN-SIM_10.root"
    )
)

# process.dump = cms.EDAnalyzer("EventContentAnalyzer")

print "---------------------------"
print "\n" + "Datatier = " + datatier + "\n"
print "---------------------------"

if datatier == "GEN":
    process.GenAnalyzer.genParticlesInput = cms.InputTag("genParticles")
    process.GenAnalyzer.genJetsInput      = cms.InputTag("ak4GenJets")
elif datatier == "MINIAODSIM":
    process.GenAnalyzer.genParticlesInput = cms.InputTag("prunedGenParticles")
    process.GenAnalyzer.genJetsInput      = cms.InputTag("slimmedGenJets")
else:
    print "[ERROR] Unknown datatier: {}" + datatier
    raise RuntimeError
# ---------------------------

process.p = cms.Path(
process.GenAnalyzer
)
