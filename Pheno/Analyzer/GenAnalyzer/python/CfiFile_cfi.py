import FWCore.ParameterSet.Config as cms

# Default settings
GenAnalyzer = cms.EDAnalyzer("GenAnalyzer",

    min_pt_jet   = cms.double(-1.0),
    min_pt_lep   = cms.double(-1.0),
    max_eta_jet  = cms.double(-1.0),
    max_eta_lep  = cms.double(-1.0),

    genParticlesInput = cms.InputTag("genParticles"),
    genJetsInput = cms.InputTag("ak4GenJets"),
    genEventInfoInput = cms.InputTag("generator"),
    srcInput = cms.InputTag("source")
)
