from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tllqdim6_v2"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName = 'LHE-GEN-SIM_cfg_v2.py'
config.JobType.inputFiles = ['/afs/cern.ch/work/n/ntonon/public/TopEFT_MCSimulation/CMSSW_9_3_6/src/crabDir/tllqdim6_slc6_amd64_gcc630_CMSSW_9_3_16_tarball.tar.xz']
config.JobType.disableAutomaticOutputCollection = False
#settings below should be enough for 1000 events per job. would advice 800-1000 events per job (jobs will fail if you use too many)
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
#Name of the private campaign. It is going to be published with /outputPrimaryDataset/username-outputDatasetTag/
config.Data.outputPrimaryDataset = prodName
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 100 #number of events per jobs
config.Data.totalUnits = 1000 #Nof events
#publication on das under prod/phys03.
config.Data.publication = False
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


