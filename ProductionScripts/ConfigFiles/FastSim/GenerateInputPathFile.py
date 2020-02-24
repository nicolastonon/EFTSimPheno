import sys

#List of paths to all UserInputFiles to process with CRAB
#1 file per line, no quotes/commas
#https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile#CRAB_configuration_parameters
def Write_FilePaths(dirpath, filetype, nof_files):
    text_file = open("inputs_paths.txt", "w")

    for i in range(1, nof_files+1):
        text_file.write(dirpath+'/'+filetype+'_'+str(i)+'.root'+'\n')

    text_file.close()



dirpath = "/store/user/ntonon/ttlldim6_FASTSIM_v0/ttlldim6_FASTSIM_v0/200223_164530/0000"
filetype = "FASTSIM1_inAODSIM"
nof_files = 100

Write_FilePaths(dirpath, filetype, nof_files)
