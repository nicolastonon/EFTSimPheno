<!-- ```
mkdir MyAnalysis
cd MyAnalysis
```
:information_source: xxx
:arrow_right: xxx
:heavy_exclamation_mark: xxx -->

:telephone: Contact : nicolas.tonon@cern.ch
_____________________________________________________________________________

Datacards, codes and instructions for private simulation of Top EFT samples, and for basic pheno studies.


#### Table Of Contents

* [Useful links](https://github.com/nicolastonon/EFT-Simu-Pheno#Useful-links)

* [Setup](https://github.com/nicolastonon/EFT-Simu-Pheno#Setup)

* [MC simulation](https://github.com/nicolastonon/EFT-Simu-Pheno#MC-simulation)
    * [Cards](https://github.com/nicolastonon/EFT-Simu-Pheno#Cards)
    * [Gridpack generation](https://github.com/nicolastonon/EFT-Simu-Pheno#Gridpack-generation)
    * [Generate LHE events](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-LHE-events)
    * [Shower & FastSim](https://github.com/nicolastonon/EFT-Simu-Pheno#shower--fastsim)

* [Pheno studies](https://github.com/nicolastonon/EFT-Simu-Pheno#Pheno-studies)
    * [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno#GenAnalyzer)
    * [GenPlotter](https://github.com/nicolastonon/EFT-Simu-Pheno#GenPlotter)

_____________________________________________________________________________

# Useful links

- [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

- [dim6top model](https://feynrules.irmp.ucl.ac.be/wiki/dim6top)

- [Instructions for private production](https://docs.google.com/document/d/1YghFcqPGS8lx4OIpHWtpNHD8keQQf1vL5XtAP4TJBuo) *(by Seth Moortgat)*

- [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

_____________________________________________________________________________

# Setup

```
mkdir MyAnalysis
cd MyAnalysis

# CMSSW Release
RELEASE=9_3_4

# Setup release
cmsrel CMSSW_$RELEASE
cd CMSSW_X_Y_Z/src
cmsenv
git cms-init
```
_____________________________________________________________________________

# MC simulation

## Cards

* Template datacards for main processes are stored in the [GenCards](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/GenCards) directory.

## Gridpack generation

- [Instructions](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCMSConnect) to use the CMSConnect service.

- Launch the gridpack production :

```
#Renew proxy
voms-proxy-init -voms cms -valid 192:00

#Go to working area, e.g. :
cd /local-scratch/$USER/genproductions/bin/MadGraph5_aMCatNLO/

#Generate your gridpacks, using the cmsConnect script:
nohup ./submit_cmsconnect_gridpack_generation.sh [name of process card without '_proc_card.dat'] [folder containing cards relative to current location] > debugfile 2>&1 &
```

*NB : gridpack generation for the ttZ and tZq processes (including dim6top operators) takes ~30 min.*

:arrow_right: This outputs a gridpack with a name of the form 'PROCNAME_$SCRAM_ARCH_CMSSW_X_Y_Z_tarball.tar.xz'.


## Generate LHE events

- Produce events interactively from the gridpack :

```
mkdir workdir

cd workdir

cp <path to gridpack creation>/gridpackXXX_tarball.tar.xz .

tar -xavf gridpackXXX_tarball.tar.xz

NEVENTS=10000
RANDOMSEED=12345
NCPU=1
./runcmsgrid.sh $NEVENTS $RANDOMSEED $NCPU
```

*NB : generation of 10K events for the ttZ and tZq processes (including dim6top operators) takes ~30 min.*

:arrow_right: This outputs a file in the LHE format named 'cmsgrid_final.lhe'.


## Shower + FastSim

### Generate config file

- Example *cmsDriver* command to produce the config file 'FASTSIM_cfg.py' using a custom fragment :
```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py --conditions auto:run2_mc --fast -n 100 --era Run2_25ns --eventcontent AODSIM -s GEN,SIM,RECOBEFMIX,DIGI:pdigi_valid,RECO --datatier GEN-SIM-DIGI-RECO --beamspot Realistic25ns13TeVEarly2017Collision --filein file:cmsgrid_final_tzq.lhe --fileout file:test_FASTSIM.root --python_filename python_FASTSIM_cfg.py --no-exec
```
:information_source: Remove '--no-exec' to run interactively.

### Run with CRAB

- The directory [ConvertLHEtoX](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/ConvertLHEtoX) contains examples of config files necessary to perform showering/fastSim via CRAB.

- The outputs stored to T2_DE_DESY can then be found e.g. with the command :

```
gfal-ls -l srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/$USER/
```


# Pheno studies

## GenAnalyzer

- The directory [Pheno](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/Pheno) contains examples of cfg/code files necessary to analyze the showered events, and extract some relevant features (top/Z kinematics, ...).

- After making the necessary modifications in the config file, run the code :
```
cd src/
scram b
cmsRun Pheno/Analyzer/test/GenAnalyzer/ConfFile_cfg.py
```

## GenPlotter

- Running the code 'GenPlotter.cc' will produce plots for pheno studies.

_____________________________________________________________________________
