from WMCore.Configuration import Configuration
config = Configuration()

prodName = "tllqdim6_DIGI2_v0"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.psetName = 'DIGI2_crabCfg.py'
config.JobType.disableAutomaticOutputCollection = False
#settings below should be enough for 1000 events per job. would advice 800-1000 events per job (jobs will fail if you use too many)
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
#Name of the private campaign. It is going to be published with /outputPrimaryDataset/username-outputDatasetTag/
config.Data.outputPrimaryDataset = prodName
#config.Data.inputDataset = '/store/user/ntonon/tllqdim6_v2/tllqdim6_v2/200217_153913/0000/LHE-GEN-SIM*'
config.Data.userInputFiles = ['/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_1.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_2.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_3.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_4.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_5.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_6.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_7.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_8.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_9.root', '/store/user/ntonon/tllqdim6_DIGI1_v0/tllqdim6_DIGI1_v0/200217_225510/0000/DIGI1_10.root']	
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1 #number of events per jobs
config.Data.totalUnits = 10 #Total nof events
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


