import sys

#List of paths to all UserInputFiles to process with CRAB
#1 file per line, no quotes/commas
#https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile#CRAB_configuration_parameters
def Write_FilePaths(dirpath, filetype, nof_files):
    text_file = open("inputs_paths.txt", "w")

    for i in range(1, nof_files+1):
        text_file.write(dirpath+'/'+filetype+'_'+str(i)+'.root'+'\n')

    text_file.close()



dirpath = "/store/user/ntonon/tllqdim6_FASTSIM_v2/tllqdim6_FASTSIM_v2/200315_011949/0000/"
filetype = "FASTSIM1"
nof_files = 1000

Write_FilePaths(dirpath, filetype, nof_files)
