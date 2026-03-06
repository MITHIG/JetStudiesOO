# crab config to run over a single file (test.txt)
# if you modify 
from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8"
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "forest_CMSSWConfig_Run3_OO_MC_miniAOD.py"
config.JobType.maxMemoryMB = 3000
config.JobType.maxJobRuntimeMin = 360
# needed for temp jet energy corrections
config.JobType.inputFiles = ['ParallelMC_L2Relative_AK4PF.txt']

config.section_("Data")
config.Data.userInputFiles = open("test.txt").readlines()

config.Data.inputDBS = "global"
config.Data.splitting = "FileBased"
config.Data.unitsPerJob = 2 ## Number of *input* event per job !
config.Data.outputDatasetTag = "OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8"
config.Data.outLFNDirBase = "/store/group/phys_heavyions/hbossi/mc_productions"
config.Data.publication = False
# allow for production based datasets to work
config.Data.allowNonValidInputDataset = True

config.section_("Site")
config.Site.storageSite = "T2_CH_CERN"
config.Site.whitelist = ['T2_CH_CERN']
