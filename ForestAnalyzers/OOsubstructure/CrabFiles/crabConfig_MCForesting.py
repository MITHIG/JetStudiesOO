from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "DijetEmbeddedFull_pThat-15to1200_TuneCP5_5p36TeV_pythia8"
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "forest_CMSSWConfig_Run3_OO_MC_miniAOD.py"
config.JobType.maxMemoryMB = 3000
config.JobType.maxJobRuntimeMin = 360
config.JobType.inputFiles = ['ParallelMC_L2Relative_AK4PF.txt']

config.section_("Data")
config.Data.inputDataset = "/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/HINOOSpring25MiniAOD-CustomTrack_150X_mcRun3_2025_forOO_realistic_v9-v2/MINIAODSIM"
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
