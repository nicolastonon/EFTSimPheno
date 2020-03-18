from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tllqdim6_FASTSIM2_v1"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.psetName = 'FASTSIM2_cfg.py'
config.JobType.disableAutomaticOutputCollection = False
#settings below should be enough for 1000 events per job. would advice 800-1000 events per job (jobs will fail if you use too many)
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
config.Data.inputDataset = '/tllqdim6_FASTSIM_v2/ntonon-tllqdim6_FASTSIM_v2_AODSIMoutput-1c7e6583a97cb2a86562f09fca71c7fb/USER'
config.Data.inputDBS = 'phys03' #Else, looking in DAS under prod/global
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1 #number of files per jobs
config.Data.totalUnits = 1000 #Nof files
#publication on das under prod/phys03.
config.Data.publication = True
#second part of the sample name
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True

config.section_("Site")
#site where you have your t2 account and grid storage
config.Site.storageSite = 'T2_DE_DESY'
#configure which sites to run on
config.Site.whitelist = ['T2_DE_DESY']

config.section_("User")

## only german users
config.User.voGroup = "dcms"
