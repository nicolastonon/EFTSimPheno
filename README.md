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
> * generate private MC samples including EFT reweighting ;
> * perform basic phenomenology studies ;
> * analyse DESY TopZ ntuples ;
> * run COMBINE.

#### Table Of Contents

* [MC simulation](https://github.com/nicolastonon/EFT-Simu-Pheno#MC-simulation)
    * [Setup](https://github.com/nicolastonon/EFT-Simu-Pheno#Setup)
    * [Madgraph cards](https://github.com/nicolastonon/EFT-Simu-Pheno#Madgraph-cards)
    * [Gridpack generation](https://github.com/nicolastonon/EFT-Simu-Pheno#Gridpack-generation)
    * [Generate LHE events interactively](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-LHE-events-interactively)
    * [Generate particle-level events](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-particle-level-events)
      * [GEN-only](https://github.com/nicolastonon/EFT-Simu-Pheno#GEN-only)
      * [miniAOD (Fast simulation)](https://github.com/nicolastonon/EFT-Simu-Pheno#miniaod-fast-simulation)
      * [miniAOD (Full simulation)](https://github.com/nicolastonon/EFT-Simu-Pheno#miniaod-full-simulation)

* [Pheno studies](https://github.com/nicolastonon/EFT-Simu-Pheno#Pheno-studies)
    * [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno#GenAnalyzer)
    * [GenPlotter](https://github.com/nicolastonon/EFT-Simu-Pheno#GenPlotter)

* [Ntuple analysis](https://github.com/nicolastonon/EFT-Simu-Pheno#Ntuple-analysis-and-COMBINE)

_____________________________________________________________________________

#### Useful links

:link: [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

:link: [dim6top model](https://feynrules.irmp.ucl.ac.be/wiki/dim6top)

:link: [Instructions for private production](https://docs.google.com/document/d/1YghFcqPGS8lx4OIpHWtpNHD8keQQf1vL5XtAP4TJBuo) *(by Seth Moortgat)*

:link: [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

_____________________________________________________________________________

# MC simulation

## Setup

Depending on the production step, and the data-taking year you are considering, different CMSSW releases may be used.

Example commands below are valid for 2017 MC production.

:construction: *Add commands for each year.*

```
mkdir MyAnalysis
cd MyAnalysis

# Or other release, as specific below
RELEASE=9_3_6

# Setup release
cmsrel CMSSW_$RELEASE
cd CMSSW_X_Y_Z/src
cmsenv

# Useful for GEN step only
mkdir -p Configuration/GenProduction/
git clone https://github.com/cms-sw/genproductions.git Configuration/GenProduction/

# You can then copy the custom fragment [Configuration/GenProduction/python/PrivProd.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/Fragments/PrivProd.py)
```

## Madgraph cards

Templates of Madgraph datacards are provided for some processes in the [GenCards](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/GenCards) directory.

## Gridpack generation

Gridpacks may be generated in several ways (e.g. interactively, using screen, via CRAB, HTCondor, etc.). I suggest using the CMSConnect service (easy to use and efficient).

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

## Generate LHE events interactively

You can run Madgraph directly, or generate events from the gridpack :

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

:arrow_right: This outputs a file in the LHE format named 'cmsgrid_final.lhe'.

## Generate particle-level events

CMSSW can act as a wrapper around various generators and chain their outputs together.
This makes it quite  easy to run large-scale production from a gridpack to the finished (Mini/Nano)AOD file.

Once you have generated your gridpack, you may either want to :
1) simply generate events in LHE format and shower them (for phenomenology studies) ;
2) generate events in LHE format, shower them, run the detector simulation, digitize the signals in the detector, emulate the trigger response, and reconstruct events in the miniAOD format (for final anlysis).

Moreover, you may run these steps either interactively or e.g. via CRAB (HTConder not tested yet). For more than ~1K events, using CRAB is mandatory.

_____________________________________________________________________________

Below you can find example `cmsDriver` commands to create python configuration files for each production step.
Template for CRAB and python configuration files for each production step can be found in the [ProductionScripts](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts) directory.

:information_source: *Type 'cmsDriver --help' to get infos on arguments.*

:information_source: *Whenever you need to run on a long list of user input files (i.e. for all steps apart for the first one), you can use the [GenerateInputPathFile.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/ConfigFiles/FullSim/GenerateInputPathFile.py) script to generate the filelist to be read.*

### GEN-only

- Create the config file 'GEN_cfg.py', read a LHE input file, shower events with a custom fragment :
```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py --mc --conditions auto:run2_mc -n 100 --era Run2_25ns --eventcontent RAWSIM --step GEN --datatier GEN-SIM --beamspot Realistic25ns13TeVEarly2017Collision --filein file:cmsgrid_final_tzq.lhe --fileout file:GEN.root --python_filename GEN_cfg.py --no_exec

cmsRun GEN_cfg.py
```

:arrow_right_hook: The output file can be passed to my [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno/Pheno/Analyzer) code for generator-level studies.

<!-- :clock430: *NB : for 10K events interactively, this step takes ~1h.* -->

### miniAOD [Fast simulation]

*[VALIDATED UNDER CMSSW_9_4_12]*

With FastSim, several production steps can be chained together, and a simplified detector simulation is used.

:information_source: NB : trigger collections are not used with FastSim and should not be read.*

* Step 1 [GEN,SIM,DIGI]

If you read and existing LHE file, remove the `LHE` keyword and add `--filein file:yourfile.lhe` :
```
cmsDriver.py Configuration/GenProduction/python/PrivProdFromGridpack.py \
--fileout file:FASTSIM1.root \
--python_filename FASTSIM1_cfg.py \
--mc --eventcontent LHE,AODSIM \
--fast \
--datatier LHE,AODSIM \
--step LHE,GEN,SIM,RECOBEFMIX,DIGIPREMIX_S2,DATAMIX,L1,DIGI2RAW,L1Reco,RECO \
--conditions 94X_mc2017_realistic_v15 \
--datamix PreMix --era Run2_2017_FastSim \
--beamspot Realistic25ns13TeVEarly2017Collision \
--no_exec -n 10 --customise Configuration/DataProcessing/Utils.addMonitoring \
--pileup_input "dbs:/Neutrino_E-10_gun/RunIIFall17FSPrePremix-PUMoriond17_94X_mc2017_realistic_v15-v1/GEN-SIM-DIGI-RAW"

cmsRun FASTSIM1_cfg.py
```

* Step 2 [miniAOD]

```
cmsDriver.py --filein file:FASTSIM1.root --fileout file:miniAOD.root \
--python_filename FASTSIM2_cfg.py \
--mc --eventcontent MINIAODSIM --runUnscheduled \
--fast \
--datatier MINIAODSIM \
--conditions 94X_mc2017_realistic_v15 \
--step PAT --scenario pp \
--era Run2_2017_FastSim,run2_miniAOD_94XFall17 \
--no_exec --customise Configuration/DataProcessing/Utils.addMonitoring

cmsRun FASTSIM2_cfg.py
```


### miniAOD [Full simulation]

Here are the main production steps when using the full GEANT4 detector simulation.

#### GEN-SIM

*[VALIDATED UNDER CMSSW_9_3_6]*

Use this command if you want to read an existing LHE file (if you want to create events from a gridpack, see below) :

```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py \
--filein file:xxx.lhe --fileout file:GEN-SIM.root \
--python_filename GEN-SIM_cfg.py \
--mc --eventcontent RAWSIM --datatier GEN-SIM \
--conditions 93X_mc2017_realistic_v3 \
--beamspot Realistic25ns13TeVEarly2017Collision \
--step GEN,SIM \
--geometry DB:Extended --era Run2_2017 \
--customise Configuration/DataProcessing/Utils.addMonitoring \
--no_exec -n 10

cmsRun GEN-SIM_cfg.py
```

#### LHE-GEN-SIM

*[VALIDATED UNDER CMSSW_9_3_6]*

Use this command if you want to create events from a gridpack (+ shower them and apply detector simulation) :

```
cmsDriver.py Configuration/GenProduction/python/PrivProdFromGridpack.py \
--fileout file:LHE-GEN-SIM.root \
--python_filename LHE-GEN-SIM_cfg.py \
--mc --eventcontent RAWSIM,LHE --datatier GEN-SIM,LHE \
--conditions 93X_mc2017_realistic_v3 \
--beamspot Realistic25ns13TeVEarly2017Collision \
--step LHE,GEN,SIM \
--geometry DB:Extended --era Run2_2017 \
--customise Configuration/DataProcessing/Utils.addMonitoring \
--no_exec -n 10

cmsRun LHE-GEN-SIM_cfg.py
```

:heavy_exclamation_mark: Note that a different custom fragment [PrivProdFromGridpack.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/Fragments/PrivProdFromGridpack.py) is used, which includes a block to read the gridpack.

#### DIGI-RECO

*[VALIDATED UNDER CMSSW_9_4_4]*

* Step 1 [L1, HLT, Pileup, DIGI] :
```
cmsDriver.py step1 --filein file:GEN-SIM.root --fileout file:DIGI1.root \
--python_filename DIGI1_cfg.py \
--eventcontent PREMIXRAW \
--datatier GEN-SIM-RAW \
--step DIGIPREMIX_S2,DATAMIX,L1,DIGI2RAW,HLT:2e34v40 \
--datamix PreMix --era Run2_2017 \
--conditions 94X_mc2017_realistic_v11 --mc -n 10 \
--customise Configuration/DataProcessing/Utils.addMonitoring \
--no_exec \
--pileup_input "dbs:/Neutrino_E-10_gun/RunIISummer17PrePremix-MCv2_correctPU_94X_mc2017_realistic_v9-v1/GEN-SIM-DIGI-RAW"

cmsRun DIGI1_cfg.py
```

* Step 2 [RECO] :
```
cmsDriver.py step2 --filein file:DIGI1.root --fileout file:DIGI2.root \
--python_filename DIGI2_cfg.py \
--mc --eventcontent AODSIM \
--runUnscheduled --datatier AODSIM \
--conditions 94X_mc2017_realistic_v11 \
--step RAW2DIGI,RECO,RECOSIM,EI \
--era Run2_2017 \
--no_exec

cmsRun DIGI2_cfg.py
```

#### MINIAOD

*[VALIDATED UNDER CMSSW_9_4_4]*

```
cmsDriver.py --filein file:DIGI2.root --fileout file:miniAOD.root \
--python_filename miniAOD_cfg.py \
--eventcontent MINIAODSIM \
--datatier MINIAODSIM --conditions 94X_mc2017_realistic_v14 \
--step PAT \
--mc --runUnscheduled --scenario pp \
--era Run2_2017 --no_exec

cmsRun miniAOD_cfg.py
```

# Pheno studies

The [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno/Pheno/Analyzer) code must be run under CMSSW (I am using CMSSW_10_2_18).
The [GenPlotter](https://github.com/nicolastonon/EFT-Simu-Pheno/Pheno/Plotter) is a standalone code (I run it locally).

## GenAnalyzer

- The directory [Pheno/Analyzer](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/Pheno/Analyzer) contains examples of cfg/code files necessary to analyze the showered events, and extract some relevant features (top/Z kinematics, ...).

- After making the necessary modifications in the config file, run the code :
```
cd src/
scram b
cmsRun Pheno/Analyzer/test/GenAnalyzer/ConfFile_cfg.py
```

## GenPlotter

- The directory [Pheno/Plotter](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/Pheno/Plotter) contains codes to produce plots from ROOT files created with the GenAnalyzer code, for any available variable / EFT weight / process.

- Compiling and running the code 'GenPlotter.cc' :

```
g++ GenPlotter.cc -o GenPlotter.exe `root-config --cflags --glibs`
./GenPlotter.exe
```

# Ntuple analysis and COMBINE

See the READMEs in the [dedicated directory](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/myAnalysis).

_____________________________________________________________________________


<!-- OBSOLETE, TO VERIFY, ...
