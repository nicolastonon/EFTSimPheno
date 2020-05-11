from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tllqdim6_xxx"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName = 'LHE-GEN-SIM_cfg.py'
config.JobType.inputFiles = ['/afs/cern.ch/work/n/ntonon/public/TopEFT_MCSimulation/CMSSW_9_3_6/src/xxx.tar.xz']
config.JobType.disableAutomaticOutputCollection = False
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
config.Data.outputPrimaryDataset = prodName
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 100 #Number of events per jobs
config.Data.totalUnits = 100000 #Total nof events
config.Data.publication = True
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True

config.section_("Site")
config.Site.storageSite = 'T2_DE_DESY'
config.Site.whitelist = ['T2_DE_*']

config.section_("User")

## only german users
config.User.voGroup = "dcms"
