import FWCore.ParameterSet.Config as cms

from HeavyIonsAnalysis.JetAnalysis.inclusiveJetAnalyzer_cff import *

akCs4PFJetAnalyzer = inclusiveJetAnalyzer.clone(
    jetTag = cms.InputTag("slimmedJets"),
    genjetTag = cms.InputTag("slimmedGenJets"),
    matchTag = cms.untracked.InputTag("slimmedGenJets"),
    rParam = 0.4,
    fillGenJets = True,
    isMC = True,
    genParticles = cms.untracked.InputTag("prunedGenParticles"),
    eventInfoTag = cms.InputTag("generator"),
    jetName = cms.untracked.string("akCs4PF"),
    genPtMin = cms.untracked.double(5),
    vtxTag = cms.InputTag("offlineSlimmedPrimaryVertices"),
    #hltTrgResults = cms.untracked.string('TriggerResults::'+'HISIGNAL'),
    hltTrgResults = cms.untracked.string('TriggerResults'),
    genDroppedBranches = cms.InputTag("ak4GenJets","droppedBranches"),
    jetFlavourInfos = cms.InputTag("ak4PFUnsubJetFlavourInfos")
    )

akFlowPuCs4PFJetAnalyzer = akCs4PFJetAnalyzer.clone()
