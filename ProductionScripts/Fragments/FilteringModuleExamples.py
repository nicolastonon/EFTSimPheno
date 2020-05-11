#Example: https://gist.githubusercontent.com/AndreasAlbert/78514bc42cae9fc051113f8ae6e86d9d/raw/6ee0957e1dc13ad4c94c8ae99a7c9be937fea6ee/ggHinv_LHEHpt140_genHpt200_cfg.py
#Example: https://cms-pdmv.cern.ch/mcm/public/restapi/requests/get_fragment/B2G-RunIIFall17wmLHEGS-00002
#Example: https://cms-pdmv.cern.ch/mcm/public/restapi/requests/get_fragment/TOP-RunIIFall18wmLHEGS-00203
#Example: https://cms-pdmv.cern.ch/mcm/public/restapi/requests/get_fragment/SMP-PhaseIISummer17wmLHEGENOnly-00049/0

'''
import FWCore.ParameterSet.Config as cms

externalLHEProducer = cms.EDProducer("ExternalLHEProducer",
    args = cms.vstring('/cvmfs/cms.cern.ch/phys_generator/gridpacks/2017/13TeV/powheg/V2/gg_H_quark-mass-effects_NNPDF31_13TeV_M125/v1/gg_H_quark-mass-effects_NNPDF31_13TeV_M125_slc6_amd64_gcc630_CMSSW_9_3_0.tgz'),
    nEvents = cms.untracked.uint32(5000),
    numberOfParameters = cms.uint32(1),
    outputFile = cms.string('cmsgrid_final.lhe'),
    scriptName = cms.FileInPath('GeneratorInterface/LHEInterface/data/run_generic_tarball_cvmfs.sh')
    )

from Configuration.Generator.Pythia8CommonSettings_cfi import *
from Configuration.Generator.MCTunes2017.PythiaCP5Settings_cfi import *
from Configuration.Generator.Pythia8PowhegEmissionVetoSettings_cfi import *

generator = cms.EDFilter("Pythia8HadronizerFilter",
                         maxEventsToPrint = cms.untracked.int32(1),
                         pythiaPylistVerbosity = cms.untracked.int32(1),
                         filterEfficiency = cms.untracked.double(1.0),
                         pythiaHepMCVerbosity = cms.untracked.bool(False),
                         comEnergy = cms.double(13000.),
                         PythiaParameters = cms.PSet(
        pythia8CommonSettingsBlock,
        pythia8CP5SettingsBlock,
        pythia8PowhegEmissionVetoSettingsBlock,
        processParameters = cms.vstring(
            'POWHEG:nFinal = 1',   ## Number of final state particles
                                   ## (BEFORE THE DECAYS) in the LHE
                                   ## other than emitted extra parton
            '25:m0 = 125.0',
            '25:onMode = off',
            '25:onIfMatch = 23 23', ## H -> ZZ
            '23:onMode = off',      # turn OFF all Z decays
            '23:onIfAny = 12 14 16',# turn ON Z->vv
          ),
        parameterSets = cms.vstring('pythia8CommonSettings',
                                    'pythia8CP5Settings',
                                    'pythia8PowhegEmissionVetoSettings',
                                    'processParameters'
                                    )
        )
)
'''

# -------------------------------

#Filter out events which don't contain a higgs satisfying pT cuts
LHEHiggsPtFilter = cms.EDFilter("LHEPtFilter",
  selectedPdgIds = cms.vint32(25),
  ptMin=cms.double(140.),
  ptMax=cms.double(1e10),
  src=cms.InputTag("externalLHEProducer")
)

#Specifies which particles must be fed to filters ?
genParticlesForFilter = cms.EDProducer("GenParticleProducer",
  abortOnUnknownPDGCode = cms.untracked.bool(False),
  saveBarCodes = cms.untracked.bool(True),
  src = cms.InputTag("generator", "unsmeared")
)

#Filter out events which don't contain a higgs with status=62 satisfying pT cuts
GenHiggsPtFilter = cms.EDFilter("MCSingleParticleFilter",
  ParticleId = cms.vint32(25),
  MinPt = cms.untracked.vdouble(200.),
  Status = cms.untracked.vint32(62),
  src=cms.InputTag("genParticlesForFilter")
)

ProductionFilterSequence = cms.Sequence(LHEHiggsPtFilter * generator * genParticlesForFilter * GenHiggsPtFilter)

# -------------------------------

#Specifies which particles must be fed to filters ?
genParticlesForFilter = cms.EDProducer("GenParticleProducer",
    abortOnUnknownPDGCode = cms.untracked.bool(False),
    saveBarCodes = cms.untracked.bool(True),
    src = cms.InputTag("generator", "unsmeared")
)

#Similar for jets ?
genParticlesForjetsForFilter = cms.EDProducer("InputGenJetsParticleSelector",
    excludeFromResonancePids = cms.vuint32(12, 13, 14, 16),
    excludeResonances = cms.bool(False),
    ignoreParticleIDs = cms.vuint32(1000022, 1000012, 1000014, 1000016, 2000012,
        2000014, 2000016, 1000039, 5100039, 4000012,
        4000014, 4000016, 9900012, 9900014, 9900016,
        39),
    partonicFinalState = cms.bool(False),
    src = cms.InputTag("genParticlesForFilter"),
    tausAsJets = cms.bool(False)
)

ak8GenJetsForFilter = cms.EDProducer("FastjetJetProducer",
    Active_Area_Repeats = cms.int32(5),
    GhostArea = cms.double(0.01),
    Ghost_EtaMax = cms.double(6.0),
    Rho_EtaMax = cms.double(4.5),
    doAreaFastjet = cms.bool(False),
    doPUOffsetCorr = cms.bool(False),
    doPVCorrection = cms.bool(False),
    doRhoFastjet = cms.bool(False),
    inputEMin = cms.double(0.0),
    inputEtMin = cms.double(0.0),
    jetAlgorithm = cms.string('AntiKt'),
    jetPtMin = cms.double(3.0),
    jetType = cms.string('GenJet'),
    maxBadEcalCells = cms.uint32(9999999),
    maxBadHcalCells = cms.uint32(9999999),
    maxProblematicEcalCells = cms.uint32(9999999),
    maxProblematicHcalCells = cms.uint32(9999999),
    maxRecoveredEcalCells = cms.uint32(9999999),
    maxRecoveredHcalCells = cms.uint32(9999999),
    minSeed = cms.uint32(14327),
    nSigmaPU = cms.double(1.0),
    rParam = cms.double(0.8),
    radiusPU = cms.double(0.5),
    src = cms.InputTag("genParticlesForjetsForFilter"),
    srcPVs = cms.InputTag(""),
    useDeterministicSeed = cms.bool(True)
)

#Apply pt cuts
genHTFilter = cms.EDFilter("GenHTFilter",
    genHTcut = cms.double(649.0),
    jetEtaCut = cms.double(1000.0),
    jetPtCut = cms.double(650.0),
    src = cms.InputTag("ak8GenJetsForFilter")
)

#Apply jet cuts
LHEJetFilter = cms.EDFilter("LHEJetFilter",
    jetPtMin = cms.double(350.0),
    jetR = cms.double(0.8),
    src = cms.InputTag("externalLHEProducer")
)

ProductionFilterSequence = cms.Sequence(LHEJetFilter*generator*genParticlesForFilter*genParticlesForjetsForFilter*ak8GenJetsForFilter*genHTFilter)

# -------------------------------

#Filter out events which don't include a ttbar pair + status, invmll cuts
ttgenfilter = cms.EDFilter("MCParticlePairFilter",
                           Status = cms.untracked.vint32(22, 22),
                           MinPt = cms.untracked.vdouble(0., 0.),
                           ParticleCharge = cms.untracked.int32(0),
                           ParticleID1 = cms.untracked.vint32(6),
                           ParticleID2 = cms.untracked.vint32(6),
                           MinInvMass = cms.untracked.double(700.),
                           MaxInvMass = cms.untracked.double(1000.),
)

ProductionFilterSequence = cms.Sequence(generator*ttgenfilter)
-----------------
