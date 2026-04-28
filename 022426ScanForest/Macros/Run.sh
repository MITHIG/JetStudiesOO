#!/bin/bash

# ===============================
#        USER CONFIG
# ===============================

NEVENTS=0
MC=1

OUTFOLDER="0320MultipleHistMethod"
FOREST="/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/HiForestMiniAOD_5*"
OUTTAG="pthat15_1PD_5_withjetcuts"

L1MinBiasBool=1
HLTMinBiasBool=0
JetTriggerBool=0
CCFilterBool=1
PVFilterBool=1
zvtxCutBool=1
JetPtCutBool=1
JetSelectionsBool=1
HFEFilterBool=1

zvtxCutValue=15.0

jtPfCEFcut=0.8
jtPfNEFcut=0.8
jtPfMUFcut=0.8
jtPfCHMcut=0.0

jtptCut=30.0
etaCut=1.6
HFEMaxCut=14.0

# ===============================
#          RUN
# ===============================

./runSaveHist \
$NEVENTS \
$MC \
"$OUTFOLDER" \
"$FOREST" \
"$OUTTAG" \
$L1MinBiasBool \
$HLTMinBiasBool \
$JetTriggerBool \
$CCFilterBool \
$PVFilterBool \
$zvtxCutBool \
$JetPtCutBool \
$JetSelectionsBool \
$HFEFilterBool \
$zvtxCutValue \
$jtPfCEFcut \
$jtPfNEFcut \
$jtPfMUFcut \
$jtPfCHMcut \
$jtptCut \
$etaCut \
$HFEMaxCut