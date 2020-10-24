from WMCore.Configuration import Configuration
config = Configuration()

prodName = "Analyzer_ttlldim6_training"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.psetName = 'ConfFile_cfg.py'
config.JobType.disableAutomaticOutputCollection = False
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
config.Data.inputDataset = '/ttlldim6_training_LHEGENSIM/ntonon-ttlldim6_training_LHEGENSIM_RAWSIMoutput-119846688901775ed7d2d364de7217d5/USER'
config.Data.inputDBS = 'phys03' #Else, looking in DAS under prod/global
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 200 #number of files per jobs
config.Data.totalUnits = 2500 #Nof files
#publication on das under prod/phys03.
config.Data.publication = False
#second part of the sample name
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True

config.section_("Site")
#site where you have your t2 account and grid storage
config.Site.storageSite = 'T2_DE_DESY'
#configure which sites to run on
config.Site.whitelist = ['T2_DE_*']

config.section_("User")

## only german users
config.User.voGroup = "dcms"
