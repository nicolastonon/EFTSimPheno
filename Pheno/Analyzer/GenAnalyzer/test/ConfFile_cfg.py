#==================================================================
import FWCore.ParameterSet.Config as cms

process = cms.Process("GenAnalyzer")

# process.GenAnalyzer = cms.EDAnalyzer("GenAnalyzer")

datatier = "GEN"
# datatier = "AODSIM"

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("myAnalyzer.GenAnalyzer.CfiFile_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.GenAnalyzer.min_pt_jet  = cms.double(-1)
process.GenAnalyzer.min_pt_lep  = cms.double(-1)
process.GenAnalyzer.max_eta_jet = cms.double(-1)
process.GenAnalyzer.max_eta_lep = cms.double(-1)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

# -----------------------------
#NB : 'file:' indicates a physical file path (i.e. no need to look in a catalog)
process.source = cms.Source("PoolSource",
    # fileNames = cms.untracked.vstring("file:/afs/cern.ch/work/n/ntonon/public/Madgraph/CMSSW_9_3_4/src/test_FASTSIM.root")
    fileNames = cms.untracked.vstring("/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_1.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_2.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_3.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_4.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_5.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_6.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_7.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_8.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_9.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_10.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_11.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_12.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_13.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_14.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_15.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_16.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_17.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_18.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_19.root",
    "/store/user/ntonon/tzqdim6_v3/tzqdim6_v3/191217_093028/0000/test_FASTSIM_20.root"
    )
)

'''

fileNames = cms.untracked.vstring("/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000//test_FASTSIM_1.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_2.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_3.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_4.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_5.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_6.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_7.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_8.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_9.root",
"/store/user/ntonon/ttzdim6_v1/ttzdim6_v1/191213_192006/0000/test_FASTSIM_10.root"
)
'''


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
