# JetStudiesOO

# Setting up the environment


cmsrel CMSSW_15_0_18
cd CMSSW_15_0_18/src
cmsenv
git cms-init

git cms-merge-topic CmsHI:forest_CMSSW_15_0_X
git remote add cmshi git@github.com:CmsHI/cmssw.git
git cms-checkdeps -A -a

scram b -j 8

# Adding in the changes

Replace the files in ./src/HeavyIonsAnalysis/JetAnalysis with the files from this repo. Then recompile and run the forest

`cmsRun forest_CMSSWConfig_Run3_OO_MC_miniAOD.py`