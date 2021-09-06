import os
import stat
import sys
from argparse import ArgumentParser
from array import array
import random
import math

"""
example:
python Produce_LHE_condor.py --tag=new --gridpack=./tllqdim6_v3_slc6_amd64_gcc630_CMSSW_9_3_16_tarball.tar.xz --jobflavour=longlunch --neventstotal=10 --neventsperjob=10
"""

#-------------------------------------

parser = ArgumentParser()
parser.add_argument('--tag', default="LHE_production_condor",help='A specific tag name to create log files etc.')
parser.add_argument('--gridpack', default=os.getcwd()+"/test_tarball.tar.xz",help='input tarball from the gridpack production')
parser.add_argument('--neventstotal', type=int, default=1000,help='total number of simulated LHE events')
parser.add_argument('--neventsperjob', type=int, default=100,help='number of events per condor job')
parser.add_argument('--outdir', default=os.getcwd(),help='output directory with enough space for the LHE files and with write priviledges (example: EOS)')
parser.add_argument('--jobflavour', default="microcentury",help='jobFlavour as described in https://batchdocs.web.cern.ch/local/submit.html')
args = parser.parse_args()

# check if jobflavour is valid
"""
https://batchdocs.web.cern.ch/local/submit.html
espresso     = 20 minutes
microcentury = 1 hour
longlunch    = 2 hours
workday      = 8 hours
tomorrow     = 1 day
testmatch    = 3 days
nextweek     = 1 week
"""
if not(args.jobflavour in ["espresso","microcentury","longlunch","workday","tomorrow","testmatch","nextweek"]):
	print("ERROR: unknown jobflavour! should be one of the following: 'espresso','microcentury','longlunch','workday','tomorrow','testmatch','nextweek'")
	print("Exiting...")
	sys.exit(1)
	
#-------------------------------------

# Create directory to store the log files
if not os.path.isdir(os.getcwd()+"/condor_log_"+(args.tag).replace(" ","_")): os.mkdir(os.getcwd()+"/condor_log_"+(args.tag).replace(" ","_"))

# check existence of the output directory
'''
if not os.path.isdir(os.path.abspath(args.outdir)):
	need_answer = True
	while need_answer:
		answer = raw_input("The output directory (%s) is not found, should I try to create it now (y/n)?"%os.path.abspath(args.outdir))
		if answer == "n": 
			print("Exiting...")
			sys.exit(1)
		elif answer == "y": 
			os.mkdir(os.path.abspath(args.outdir))
			if not os.path.isdir(os.path.abspath(args.outdir)):
				print("creating directory failed (do you have proper acces rights?)")
				print("Exiting...")
				sys.exit(1)
			need_answer = False
		else:
			print("please type either 'y' or 'n'")
else:
    outdir_job_path = os.path.abspath(args.outdir)+'/output_' + (args.tag).replace(" ","_")

    if os.path.isdir(outdir_job_path): #Job-specific outdir already exists, warning
        print('WARNING: outdir', outdir_job_path, 'already exists... MAY OVERWRITE !')
    else:
        print("Creating output directory (where output rootfiles will be transfered): ", outdir_job_path)
        os.mkdir(outdir_job_path)
'''

#-------------------------------------

# create a text file with the production parameters
njobs = int(math.ceil(float(args.neventstotal)/float(args.neventsperjob)))
print("preparing %i jobs"%njobs)

''' #Not needed
initial_seed = int(random.uniform(1,1000))
remaining_events = args.neventstotal
f_tmp_ = open(os.getcwd()+"/params_condor.txt", 'w')
for i in  range(njobs):
	#seed = initial_seed + 2*i*args.neventsperjob + int(random.uniform(1,args.neventsperjob))
	#nevents = args.neventsperjob
	#if remaining_events < args.neventsperjob: nevents = remaining_events
	#f_tmp_.write("%i, %i, %i \n"%(i+1, seed, nevents)) #Don't need seed and nevents
	f_tmp_.write("%i \n"%(i+1))
	#remaining_events -= args.neventsperjob	
f_tmp_.close()
'''
#-------------------------------------

# create a submission batch script that untars the tarball
f_tmp_btach_ = open(os.getcwd()+"/LHEproduction_%s.sh"%((args.tag).replace(" ","_")), 'w')

f_tmp_btach_.write("#!/bin/bash\n")
f_tmp_btach_.write("echo $PWD\n\n")

f_tmp_btach_.write("gfal-mkdir \"srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/ntonon/test/\" \n\n") #Create outdir if missing

f_tmp_btach_.write("# Dump actual test code to a file that can be run in Singularity\n")
f_tmp_btach_.write("cat <<'EndOfTestFile' > script.sh\n\n") #Will write into 'script.sh' until 'EndOftestFile' string is printed

f_tmp_btach_.write("#!/bin/bash\n")
f_tmp_btach_.write("export SCRAM_ARCH=slc6_amd64_gcc481\n") #Source correct ARCH
f_tmp_btach_.write("source /cvmfs/cms.cern.ch/cmsset_default.sh\n") #Source scripts
f_tmp_btach_.write("export X509_USER_PROXY=/afs/cern.ch/user/n/ntonon/x509up_u91903\n") #Source proxy file (created first and placed in /afs home)
#f_tmp_btach_.write("voms-proxy-info -all\n")
f_tmp_btach_.write("cd /afs/cern.ch/work/n/ntonon/public/TopEFT_MCSimulation/CMSSW_7_1_46/src/ \n") #Source correct CMSSW distribution (must be created by user)
f_tmp_btach_.write("eval `scram runtime -sh` \n") #cmsenv
f_tmp_btach_.write("cd - \n") #Go back to base jobdir
f_tmp_btach_.write("echo $PWD \n")
f_tmp_btach_.write("cmsRun LHE-GEN-SIM_cfg.py \n\n") #Produce events (script shipped with job)

f_tmp_btach_.write("echo \"gfal-copy ./LHE-GEN-SIM_cfg.py srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/ntonon/tllqdim6_v4_2016_LHEGENSIM/LHE-GEN-SIM_cfg.py\" \n")
f_tmp_btach_.write("gfal-copy ./LHE-GEN-SIM.root \"srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/ntonon/test/LHE-GEN-SIM_$1.root\" \n")

f_tmp_btach_.write("# End of script.sh file \n")
f_tmp_btach_.write("EndOfTestFile\n\n") #NB: no trailing space

f_tmp_btach_.write("# Make file executable \n")
f_tmp_btach_.write("chmod +x script.sh \n")
f_tmp_btach_.write("echo 'ls -lrth' \n")
f_tmp_btach_.write("ls -lrth \n\n")

f_tmp_btach_.write("# Run in SLC6 container #Mount necessary dirs \n")
f_tmp_btach_.write("export SINGULARITY_CACHEDIR=\"/tmp/$(whoami)/singularity\" \n")
f_tmp_btach_.write("echo 'singularity run...' \n")
f_tmp_btach_.write("singularity run -B /afs -B /cvmfs -B /usr/libexec/condor -B /pool --no-home /cvmfs/singularity.opensciencegrid.org/cmssw/cms:rhel6 $(echo $(pwd)/script.sh) \n")

f_tmp_btach_.close()

#-------------------------------------

# create condor submission file
f_tmp_condor_ = open(os.getcwd()+"/ProduceLHE_condor_%s.submit"%((args.tag).replace(" ","_")), 'w')
f_tmp_condor_.write("Universe = vanilla \n")
f_tmp_condor_.write("Executable = LHEproduction_%s.sh \n"%((args.tag).replace(" ","_")))
f_tmp_condor_.write("Arguments = $(Process) \n")
f_tmp_condor_.write(" \n")
f_tmp_condor_.write("Error = condor_log_%s/job.err \n"%((args.tag).replace(" ","_")))
f_tmp_condor_.write("Output = condor_log_%s/job.out \n"%((args.tag).replace(" ","_")))
f_tmp_condor_.write("Log = condor_log_%s/job.log \n"%((args.tag).replace(" ","_")))
f_tmp_condor_.write(" \n")
f_tmp_condor_.write("stream_output = True \n")
f_tmp_condor_.write("should_transfer_files = IF_NEEDED \n")
f_tmp_condor_.write("transfer_input_files = %s, LHE-GEN-SIM_cfg.py \n"%((args.gridpack).split("/")[-1]))
f_tmp_condor_.write("when_to_transfer_output = ON_EXIT \n")
#f_tmp_condor_.write('transfer_output_remaps = "LHE-GEN-SIM.root = %s/LHE-GEN-SIM_$(Process).root" \n'%outdir_job_path)
f_tmp_condor_.write('+JobFlavour = "%s" \n' %args.jobflavour)
f_tmp_condor_.write("queue %s \n"%(njobs))
#f_tmp_condor_.write("queue number from params_condor.txt \n")
f_tmp_condor_.close()

#request_memory = < MB >
#request_disk = < KB >
#request_cpus = < positive integer >

print("The jobs can now be submitted via condor with 'condor_submit ProduceLHE_condor_%s.submit'"%((args.tag).replace(" ","_")))
print("The status can then be checked using 'condor_q'")
