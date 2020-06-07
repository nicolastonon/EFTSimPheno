from WMCore.Configuration import Configuration
config = Configuration()

prodName = "xxx"

config.section_("General")
config.General.requestName = prodName
config.General.transferLogs = True

config.section_("JobType")
config.JobType.psetName = 'DIGI2_cfg.py'
config.JobType.disableAutomaticOutputCollection = False
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2500
config.JobType.maxJobRuntimeMin = 2750

config.section_("Data")
config.Data.inputDataset = '/store/user/ntonon/tllqdim6_v2/tllqdim6_v2/200217_153913/0000/LHE-GEN-SIM*'
config.Data.inputDBS = 'phys03' #Else, looking in DAS under prod/global
# config.Data.userInputFiles = open('/afs/cern.ch/work/n/ntonon/public/TopEFT_MCSimulation/CMSSW_9_4_12/src/crabDir/workdir_ttlldim6/inputs_paths.txt').readlines() #Read list of input files generated using 'GenerateInputPathFile.py' script
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1 #number of events per jobs
config.Data.totalUnits = 100 #Total nof files
config.Data.publication = True
config.Data.outputDatasetTag = prodName
config.Data.ignoreLocality = True


config.section_("Site")
config.Site.storageSite = 'T2_DE_DESY'
config.Site.whitelist = ['T2_DE_*']

config.section_("User")

## only german users
config.User.voGroup = "dcms"
