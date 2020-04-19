from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tllqdim6_FASTSIM1_v1"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName = 'FASTSIM1_cfg.py'
config.JobType.inputFiles = ['/afs/cern.ch/work/n/ntonon/public/TopEFT_MCSimulation/CMSSW_9_4_12/src/crabDir/workdir_tllqdim6/tllqdim6_slc6_amd64_gcc630_CMSSW_9_3_16_tarball.tar.xz']
config.JobType.disableAutomaticOutputCollection = False
#settings below should be enough for 1000 events per job. would advice 800-1000 events per job (jobs will fail if you use too many)
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
#Name of the private campaign. It is going to be published with /outputPrimaryDataset/username-outputDatasetTag/
config.Data.outputPrimaryDataset = prodName
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 1000 #number of events per jobs
config.Data.totalUnits = 1000000 #Nof events
#publication on das under prod/phys03.
config.Data.publication = True
#second part of the sample name
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True #Force running jobs at DESY


config.section_("Site")
#site where you have your t2 account and grid storage
config.Site.storageSite = 'T2_DE_DESY'
#configure which sites to run on
config.Site.whitelist = ['T2_DE_DESY'] #NB : should keep this to run at DESY (got crashes at other sites when reading PU files... credentials issue ?)

config.section_("User")

## only german users
config.User.voGroup = "dcms"
