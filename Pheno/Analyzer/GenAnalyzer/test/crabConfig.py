from WMCore.Configuration import Configuration
config = Configuration()

prodName = "Analyzer_v4"

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
config.Data.inputDataset = '/tllqdim6_FASTSIM1_v1/ntonon-tllqdim6_FASTSIM1_v1_AODSIMoutput-1c7e6583a97cb2a86562f09fca71c7fb/USER'
config.Data.inputDBS = 'phys03' #Else, looking in DAS under prod/global
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 50 #number of files per jobs
config.Data.totalUnits = 500 #Nof files
#publication on das under prod/phys03.
config.Data.publication = False
#second part of the sample name
config.Data.outputDatasetTag = prodName
#config.Data.ignoreLocality = True #CHANGED -- DO NOT ONLY RUN AT DESY
#config.Data.outLFNDirBase = '/store/user/ntonon/'+prodName #Can specify here output path. Defaults to '/store/user/ntonon/inputDS/prodName'


config.section_("Site")
#site where you have your t2 account and grid storage
config.Site.storageSite = 'T2_DE_DESY'
#configure which sites to run on
#config.Site.whitelist = ['T2_DE_DESY'] #CHANGED -- DO NOT ONLY RUN AT DESY ?

config.section_("User")

## only german users
config.User.voGroup = "dcms"
