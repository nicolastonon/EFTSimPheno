<!--
```
CODE EXAMPLE
```

=== Emoji list (see https://gist.github.com/rxaviers/7360908)
:arrow_right:
:heavy_exclamation_mark:
:heavy_check_mark:
:link:
:white_check_mark:
:heavy_multiplication_x:
:x:
:negative_squared_cross_mark:
:bangbang:
:white_check_mark:
:copyright:
:clock430:
:no_entry:
:ok:
:arrow_right_hook:
:paperclip:
:open_file_folder:
:chart_with_upwards_trend:
:lock:
:hourglass:
:warning:
:construction:
:fr:
:one: :two: :hash:
:underage:
:put_litter_in_its_place:
:new:
-------------------------------------------->

:telephone: Contact : nicolas.tonon@cern.ch

<!-- > The only real mistake is the one from which we learn nothing. *-- John Powell* -->
_____________________________________________________________________________

> Files and instructions to :
> * generate private Top EFT samples ;
> * perform basic phenomenology studies ;
> * analyse DESY TopZ ntuples ;
> * run COMBINE.

#### Table Of Contents

* [MC simulation](https://github.com/nicolastonon/EFT-Simu-Pheno#MC-simulation)
    * [Setup](https://github.com/nicolastonon/EFT-Simu-Pheno#Setup)
    * [Cards](https://github.com/nicolastonon/EFT-Simu-Pheno#Cards)
    * [Gridpack generation](https://github.com/nicolastonon/EFT-Simu-Pheno#Gridpack-generation)
    * [Generate parton-level events](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-parton-level-events)
    * [Generate particle-level events](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-particle-level-events)
      * [GEN-only](https://github.com/nicolastonon/EFT-Simu-Pheno#GEN-only)
      * [Shower + FastSim + RECO](https://github.com/nicolastonon/EFT-Simu-Pheno#Shower--FastSim--RECO)

* [Pheno studies](https://github.com/nicolastonon/EFT-Simu-Pheno#Pheno-studies)
    * [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno#GenAnalyzer)
    * [GenPlotter](https://github.com/nicolastonon/EFT-Simu-Pheno#GenPlotter)

* [Ntuple analysis](https://github.com/nicolastonon/EFT-Simu-Pheno#Ntuple-analysis)

* [COMBINE](https://github.com/nicolastonon/EFT-Simu-Pheno#COMBINE)

_____________________________________________________________________________

#### Useful links

:link: [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

:link: [dim6top model](https://feynrules.irmp.ucl.ac.be/wiki/dim6top)

:link: [Instructions for private production](https://docs.google.com/document/d/1YghFcqPGS8lx4OIpHWtpNHD8keQQf1vL5XtAP4TJBuo) *(by Seth Moortgat)*

:link: [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

_____________________________________________________________________________

# MC simulation


## Setup

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

## Cards

* Template datacards for main processes are stored in the [GenCards](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/GenCards) directory.

## Gridpack generation

- First time : [General instructions](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCMSConnect) to create an account and use the CMSConnect service.

- Log in to the service, setup the datacards properly, and launch the gridpack production :

```
#Log in
ssh -X USER@login.uscms.org

#Renew proxy
voms-proxy-init -voms cms -valid 192:00

#Go to working area, e.g. :
cd /local-scratch/$USER/genproductions/bin/MadGraph5_aMCatNLO/

#Place all your datacards in a directory, with the same prefix

#Generate your gridpacks, using the cmsConnect script:
nohup ./submit_cmsconnect_gridpack_generation.sh [name of process card without '_proc_card.dat'] [folder containing cards relative to current location] > debugfile 2>&1 &
```

:arrow_right: This outputs a gridpack with a name of the form 'PROCNAME_$SCRAM_ARCH_CMSSW_X_Y_Z_tarball.tar.xz'.


## Generate parton-level events

- You can run Madgraph directly, or generate events from the gridpack :

```
mkdir workdir

cd workdir

cp <path to gridpack creation>/gridpackXXX_tarball.tar.xz .

tar xavf gridpackXXX_tarball.tar.xz

NEVENTS=10000
RANDOMSEED=12345
NCPU=1
./runcmsgrid.sh $NEVENTS $RANDOMSEED $NCPU
```

:clock430: *NB : generation of 10K events for the ttZ and tZq processes (including dim6top operators) takes ~30 min. Similar times both running Madgraph directly or from the gridpack.*

:arrow_right: This outputs a file in the LHE format named 'cmsgrid_final.lhe'.

## Generate particle-level events

To generate physical collision events, the LHE files we have created need to be showered.
Parton showering accounts for non-perturbative QCD  effects with phenomenological models.
The most used tool for parton showering in CMS is Pythia 8.

### GEN-only

:information_source: Type 'cmsDriver --help' to get infos on arguments.

- Example : produce the config file 'GEN_cfg.py' using a custom fragment and run:
```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py --mc --conditions auto:run2_mc -n 100 --era Run2_25ns --eventcontent RAWSIM --step GEN --datatier GEN-SIM --beamspot Realistic25ns13TeVEarly2017Collision --filein file:cmsgrid_final_tzq.lhe --fileout file:GEN.root --python_filename GEN_cfg.py --no_exec

cmsRun GEN_cfg.py
```

:arrow_right_hook: The output file can be passed to the GenAnalyzer code for generator-level studies.

:clock430: *NB : for 10K events interactively, this step takes ~1h.*

### Shower + FastSim + RECO

CMSSW can act as a wrapper around various generators and chain their outputs together.
This makes it quite  easy to run large-scale production from a gridpack to the finished (Mini/Nano)AOD file.

Here's an example how to chain Shower + FastSim + RECO steps.

- Example : produce the config file 'FASTSIM_cfg.py' using a custom fragment and run:
```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py --conditions auto:run2_mc --fast -n 100 --era Run2_25ns --eventcontent AODSIM -s GEN,SIM,RECOBEFMIX,DIGI:pdigi_valid,RECO --datatier GEN-SIM-DIGI-RECO --beamspot Realistic25ns13TeVEarly2017Collision --filein file:cmsgrid_final_tzq.lhe --fileout file:test_FASTSIM.root --python_filename python_FASTSIM_cfg.py --no_exec

cmsRun python_FASTSIM_cfg.py
```

### Run with CRAB

If you are processing >10K events, or are chaining several processing steps, you may need to run on CRAB.

- The directory [ConvertLHEtoX](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/ConvertLHEtoX) contains examples of config files necessary to perform showering/fastSim via CRAB *(to adapt to your needs)*.

- Once finished, the outputs stored to T2_DE_DESY can then be found e.g. with the command :

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

- Compiling and running the code 'GenPlotter.cc' will produce plots for pheno studies.

```
g++ GenPlotter.cc -o GenPlotter.exe `root-config --cflags --glibs`
./GenPlotter.exe
```


# Ntuple analysis

See the dedicated [README](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/myAnalysis/README.md).

# COMBINE

See the dedicated [README](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/myAnalysis/COMBINE/README.md).


_____________________________________________________________________________


<!-- OBSOLETE, TO VERIFY, ...
