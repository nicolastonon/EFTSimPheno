# from https://github.com/cms-sw/cmssw/blob/master/GeneratorInterface/LHEInterface/test/testReader_cfg.py
# input LHE file must be in same dir.

import FWCore.ParameterSet.Config as cms

process = cms.Process("LHE")

process.source = cms.Source("LHESource",
	fileNames = cms.untracked.vstring('file:cmsgrid_final_tllq.lhe')
	# fileNames = cms.untracked.vstring('file:cmsgrid_final_ttll.lhe')
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))
# process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.configurationMetadata = cms.untracked.PSet(
	version = cms.untracked.string('alpha'),
	name = cms.untracked.string('LHEF input'),
	annotation = cms.untracked.string('tllq')
)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cout = cms.untracked.PSet( threshold = cms.untracked.string('INFO') )

process.LHE = cms.OutputModule("PoolOutputModule",
	# dataset = cms.untracked.PSet(dataTier = cms.untracked.string('LHE')),
	dataset = cms.untracked.PSet(dataTier = cms.untracked.string('GEN')),
	fileName = cms.untracked.string('lhe.root')
)

process.outpath = cms.EndPath(process.LHE)
