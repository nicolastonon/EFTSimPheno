from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tzqdim6_v3"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'PrivateMC'
#name of the python cmsRun config to run
config.JobType.psetName = 'FASTSIM_tzq_cfg.py'#NT
config.JobType.inputFiles = ['cmsgrid_final_tzq.lhe'] #NT -- local files [NB : need to be placed in crab work dir. ?]
config.JobType.disableAutomaticOutputCollection = False
#settings below should be enough for 1000 events per job. would advice 800-1000 events per job (jobs will fail if you use too many)
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
#Name of the private campaign. It is going to be published with /outputPrimaryDataset/username-outputDatasetTag/
config.Data.outputPrimaryDataset = prodName
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 500 #number of files per jobs
config.Data.totalUnits = 10000 #Nof events
#publication on das under prod/phys03.
config.Data.publication = False #NT
#second part of the sample name
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True #NT


config.section_("Site")
#site where you have your t2 account and grid storage
config.Site.storageSite = 'T2_DE_DESY'
#configure which sites to run on
config.Site.whitelist = ['T2_DE_DESY']

config.section_("User")

## only german users
config.User.voGroup = "dcms"


