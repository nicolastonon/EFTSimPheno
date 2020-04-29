#Custom for private ttll sample
#Modify for each process : qCut (MG xqcut), nQmatch (FS), nJetMax (extra partons)

import FWCore.ParameterSet.Config as cms

from Configuration.Generator.Pythia8CommonSettings_cfi import *
from Configuration.Generator.MCTunes2017.PythiaCP5Settings_cfi import *
#from Configuration.Generator.PSweightsPythia.PythiaPSweightsSettings_cfi import * #not available in 94X

#Block to first produce LHE events from gridpack
externalLHEProducer = cms.EDProducer("ExternalLHEProducer",
    # args = cms.vstring('/cvmfs/cms.cern.ch/phys_generator/gridpacks/slc6_amd64_gcc481/13TeV/powheg/V2/test_gg_H190_quark-mass-effects_NNPDF3.0/test_gg_H190_quark-mass-effects_NNPDF30_13TeV_tarball.tar.gz'),
    args = cms.vstring('../mygridpack.tar.gz'),
    nEvents = cms.untracked.uint32(5000),
    numberOfParameters = cms.uint32(1),
    outputFile = cms.string('cmsgrid_final.lhe'),
    scriptName = cms.FileInPath('GeneratorInterface/LHEInterface/data/run_generic_tarball_cvmfs.sh')
)

#Block to hadronize generated LHE events with Pythia8
generator = cms.EDFilter("Pythia8HadronizerFilter",
    maxEventsToPrint = cms.untracked.int32(1),
    pythiaPylistVerbosity = cms.untracked.int32(1),
    filterEfficiency = cms.untracked.double(1.0),
    pythiaHepMCVerbosity = cms.untracked.bool(False),
    comEnergy = cms.double(13000.),
    PythiaParameters = cms.PSet(
        pythia8CommonSettingsBlock,
        pythia8CP5SettingsBlock,
        #pythia8PSweightsSettingsBlock, #not available in 94X
        processParameters = cms.vstring(
            'JetMatching:setMad = off',
            'JetMatching:scheme = 1',
            'JetMatching:merge = on',
            'JetMatching:jetAlgorithm = 2',
            'JetMatching:etaJetMax = 5.',
            'JetMatching:coneRadius = 1.',
            'JetMatching:slowJetPower = 1',
            'JetMatching:qCut = 10.', #this is the actual merging scale
            'JetMatching:nQmatch = 5', #4 corresponds to 4-flavour scheme (no matching of b-quarks), 5 for 5-flavour scheme
            'JetMatching:nJetMax = 1', #number of partons in born matrix element for highest multiplicity
            'JetMatching:doShowerKt = off', #off for MLM matching, turn on for shower-kT matching
        ),
        parameterSets = cms.vstring('pythia8CommonSettings',
                                    'pythia8CP5Settings',
                                    #'pythia8PSweightsSettings', #not available in 94X
                                    'processParameters',
                                    )
    )
)
