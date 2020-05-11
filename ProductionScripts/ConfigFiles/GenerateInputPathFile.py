import sys

#List of paths to all UserInputFiles to process with CRAB
#1 file per line, no quotes/commas
#https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile#CRAB_configuration_parameters
def Write_FilePaths(dirpath, filetype, nof_files):
    text_file = open("inputs_paths.txt", "w")

    for i in range(1, nof_files+1):
        text_file.write('/store/user/ntonon/'+dirpath+'/'+filetype+'_'+str(i)+'.root'+'\n')

    text_file.close()



dirpath = "tllqdim6_top19001_LHEGENSIM_v2/tllqdim6_top19001_DIGI2_v2/200502_214035/0000"
# filetype = "FASTSIM1"
filetype = "DIGI2"
nof_files = 500

Write_FilePaths(dirpath, filetype, nof_files)
