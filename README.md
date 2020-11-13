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

#HOW TO HIDE CONTENTS (which can be viewed by cliking icon) :
<details>
<summary>[NameOfHiddenContent]:</summary>
[theHiddenContent]
</details>

-------------------------------------------->

:telephone: Contact : nicolas.tonon@cern.ch

<!-- > The only real mistake is the one from which we learn nothing. *-- John Powell* -->
_____________________________________________________________________________

> Files and instructions to :
> * generate private MC samples, including EFT reweighting
> * perform basic phenomenology studies

#### Table Of Contents

* [MC simulation](https://github.com/nicolastonon/EFT-Simu-Pheno#MC-simulation)
    * [Setup](https://github.com/nicolastonon/EFT-Simu-Pheno#Setup)
    * [Madgraph cards](https://github.com/nicolastonon/EFT-Simu-Pheno#Madgraph-cards)
    * [Gridpack generation](https://github.com/nicolastonon/EFT-Simu-Pheno#Gridpack-generation)
    * [Generate LHE events interactively](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-LHE-events-interactively)
    * [Generate particle-level events](https://github.com/nicolastonon/EFT-Simu-Pheno#Generate-particle-level-events)
      * [GEN-only](https://github.com/nicolastonon/EFT-Simu-Pheno#GEN-only)
      * [miniAOD (FASTSIM)](https://github.com/nicolastonon/EFT-Simu-Pheno#miniaod-fastsim)
      * [miniAOD (FULLSIM)](https://github.com/nicolastonon/EFT-Simu-Pheno#miniaod-fullsim)
      * [Final ntuples](https://github.com/nicolastonon/EFT-Simu-Pheno#Final-ntuples)

* [Pheno studies](https://github.com/nicolastonon/EFT-Simu-Pheno#Pheno-studies)
    * [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno#GenAnalyzer)
    * [GenPlotter](https://github.com/nicolastonon/EFT-Simu-Pheno#GenPlotter)

_____________________________________________________________________________

#### Useful links

<details>
<summary>Useful links:</summary>

:link: [SMEFT@NLO model](http://feynrules.irmp.ucl.ac.be/wiki/SMEFTatNLO)

:link: [dim6top model](https://feynrules.irmp.ucl.ac.be/wiki/dim6top)

:link: [Instructions for private production](https://docs.google.com/document/d/1YghFcqPGS8lx4OIpHWtpNHD8keQQf1vL5XtAP4TJBuo) *(by Seth Moortgat)*

</details>

_____________________________________________________________________________

# MC simulation

## Setup

Depending on the production step, and the data-taking year you are considering, different CMSSW releases must be used:

<details>
<summary>CMSSW releases</summary>
```
#-- 2016
RELEASE=7_1_46 #LHE-GEN-SIM
RELEASE=8_0_21 #DIGI RECO Step1/2
RELEASE=9_4_9 #MINIAOD

#-- 2017
RELEASE=9_3_6 #LHE-GEN-SIM
RELEASE=9_4_4 #DIGI RECO Step1/2 + MINIAOD

#-- 2018
RELEASE=10_2_3 #LHE-GEN-SIM
RELEASE=10_2_5 #DIGI RECO Step1/2 + MINIAOD
```
</details>

Then, setup your workind directory:

```
mkdir myDir
cd myDir

#-- Setup release
cmsrel CMSSW_$RELEASE
cd CMSSW_X_Y_Z/src
cmsenv

#-- Needed for GEN step only
mkdir -p Configuration/GenProduction/
git clone https://github.com/cms-sw/genproductions.git Configuration/GenProduction/

#-- You can then copy there the custom fragment [Configuration/GenProduction/python/PrivProd.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/Fragments/PrivProd.py) (or use some other fragment found online -- download with `wget`).
```

## Madgraph cards

Templates of Madgraph datacards are provided for some processes in the [GenCards](https://github.com/nicolastonon/EFT-Simu-Pheno/tree/master/GenCards) directory.

## Gridpack generation

Gridpacks may be generated in several ways (e.g. interactively, using screen, via CRAB, HTCondor, etc.). I suggest using the CMSConnect service (easy to use and efficient).

- First time : [General instructions](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCMSConnect) to create an account and use the CMSConnect service.

- Login, setup the datacards properly, and launch the gridpack production :

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
2) generate events in LHE format, shower them, run the detector simulation, digitize the signals in the detector, emulate the trigger response, and reconstruct events in the miniAOD format (for final analysis).

Moreover, you may run these steps either interactively or e.g. via CRAB (HTCondor not tested yet). For more than ~1K events, running on the grid is necessary.

_____________________________________________________________________________

Below you can find example `cmsDriver` commands to create python configuration files for each production step and each year separately.

:information_source: *After you have created a config file, you can run it interactively (testing) with: `cmsRun myCfgfile.py`.*

:information_source: *Type 'cmsDriver --help' to get infos on arguments.*

:information_source: *Template for the python configuration files for 2017 and each production step can be found in the [ProductionScripts](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts) directory. Can take them as examples, but make sure to reproduce your config files by yourself to ensure consistency.*

### GEN-only

These instructions are for running the GEN step alone (i.e. for pheno-level studies, not for a reco-level analysis). For the latter, use the LHE to miniAOD steps described in [miniAOD (Full simulation)](https://github.com/nicolastonon/EFT-Simu-Pheno#miniaod-full-simulation).

:arrow_right_hook: *The output file can be passed to my [GenAnalyzer](https://github.com/nicolastonon/EFT-Simu-Pheno/Pheno/Analyzer) code for generator-level studies.*

- Create the config file 'GEN_cfg.py', read a LHE input file, shower events with a custom fragment. Example for 2017:
```
cmsDriver.py Configuration/GenProduction/python/PrivProd.py \
--filein file:cmsgrid_final_tzq.lhe --fileout file:GEN.root \
--python_filename GEN_cfg.py \
--mc --conditions auto:run2_mc -n 100 --era Run2_25ns \
--eventcontent RAWSIM --step GEN --datatier GEN-SIM \
--beamspot Realistic25ns13TeVEarly2017Collision --no_exec
```

<!-- :clock430: *NB : for 10K events interactively, this step takes ~1h.* -->

### miniAOD [FASTSIM]

*[VALIDATED UNDER CMSSW_9_4_12] for 2017 only*

With FastSim, several production steps can be chained together, and a simplified detector simulation is used.

:information_source: *NB : trigger collections are not used with FastSim and should not be read.*

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
```


### miniAOD [FULLSIM]

Here are the main production steps when using the full GEANT4 detector simulation.

#### GEN-SIM

:heavy_exclamation_mark: **The path to the gridpack is taken from the fragment when a fragment is provided. My example fragment contains a dummy path. If using it, you must then update the gridpack path directly in the LHE-GEN-SIM config file produced with this step !**

Use this command if you want to read an existing LHE file (if you want to create events directly from a gridpack, use LHE-GEN-SIM instead, see below).

- Example for 2017:

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
```

:information_source: *NB: this example reads an existing LHE file as input. If you want to generate events from a gridpack, use a consistent fragment and remove `filein`.*


#### LHE-GEN-SIM

:heavy_exclamation_mark: **The path to the gridpack is taken from the fragment when a fragment is provided. My example fragment contains a dummy path. If using it, you must then update the gridpack path directly in the LHE-GEN-SIM config file produced with this step !**

Use this command if you want to create events from a gridpack (+ shower them and apply detector simulation) :

<details>
<summary>2016</summary>
```
cmsDriver.py Configuration/GenProduction/python/PrivProdFromGridpack.py \
--fileout file:LHE-GEN-SIM.root \
--python_filename LHE-GEN-SIM_cfg.py \
--mc --eventcontent RAWSIM,LHE --datatier GEN-SIM,LHE --step LHE,GEN,SIM \
--conditions MCRUN2_71_V1::All \
--beamspot Realistic50ns13TeVCollision --magField 38T_PostLS1 \
--customise SLHCUpgradeSimulations/Configuration/postLS1Customs.customisePostLS1,Configuration/DataProcessing/Utils.addMonitoring \
--no_exec -n 10
```
</details>

<details>
<summary>2017</summary>
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
```
</details>

<details>
<summary>2018</summary>
```
cmsDriver.py Configuration/GenProduction/python/PrivProdFromGridpack.py \
--fileout file:LHE-GEN-SIM.root \
--python_filename LHE-GEN-SIM_cfg.py \
--mc --eventcontent RAWSIM,LHE --datatier GEN-SIM,LHE --step LHE,GEN,SIM \
--conditions 102X_upgrade2018_realistic_v11 \
--beamspot Realistic25ns13TeVEarly2018Collision \
--geometry DB:Extended --era Run2_2018 \
--customise Configuration/DataProcessing/Utils.addMonitoring \
--no_exec -n 10
```
</details>

<!-- :information_source: *Note that a different custom fragment [PrivProdFromGridpack.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/Fragments/PrivProdFromGridpack.py) is used, which includes a block to read the gridpack. Need to be updated manually.* -->

#### DIGI-RECO

:information_source: *NB: step 1 is probably the longest of all, and the config file is slow to produce, because it reads a lot of PU input files. Hence the number of jobs is increased in the corresponding crab prod file.*

<details>
<summary>2016</summary>
* Step 1 [L1, HLT, Pileup, DIGI] :
```
cmsDriver.py step1 --filein file:GEN-SIM.root --fileout file:DIGI1.root \
--python_filename DIGI1_cfg.py \
--mc --eventcontent PREMIXRAW \
--datatier GEN-SIM-RAW \
--step DIGIPREMIX_S2,DATAMIX,L1,DIGI2RAW,HLT:@frozen2016 \
--datamix PreMix --era Run2_2016 \
--conditions 80X_mcRun2_asymptotic_2016_TrancheIV_v6 \
--customise Configuration/DataProcessing/Utils.addMonitoring --no_exec \
--pileup_input "dbs:/Neutrino_E-10_gun/RunIISpring15PrePremix-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v2-v2/GEN-SIM-DIGI-RAW"
```

* Step 2 [RECO] :
```
cmsDriver.py step2 --filein file:DIGI1.root --fileout file:DIGI2.root \
--python_filename DIGI2_cfg.py \
--mc --eventcontent AODSIM \
--runUnscheduled --datatier AODSIM \
--conditions 80X_mcRun2_asymptotic_2016_TrancheIV_v6 \
--step RAW2DIGI,RECO,EI \
--era Run2_2016 \
--no_exec
```
</details>

<details>
<summary>2017</summary>
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
```
</details>

<details>
<summary>2018</summary>
* Step 1 [L1, HLT, Pileup, DIGI] :
```
cmsDriver.py step1 --filein file:GEN-SIM.root --fileout file:DIGI1.root \
--python_filename DIGI1_cfg.py \
--mc --eventcontent PREMIXRAW \
--datatier GEN-SIM-RAW \
--step DIGI,DATAMIX,L1,DIGI2RAW,HLT:@relval2018 \
--datamix PreMix --era Run2_2018 --procModifiers premix_stage2 \
--conditions 102X_upgrade2018_realistic_v15 \
--geometry DB:Extended \
--customise Configuration/DataProcessing/Utils.addMonitoring --no_exec \
--pileup_input "dbs:/Neutrino_E-10_gun/RunIISummer17PrePremix-PUAutumn18_102X_upgrade2018_realistic_v15-v1/GEN-SIM-DIGI-RAW"
```

* Step 2 [RECO] :
```
cmsDriver.py step2 --filein file:DIGI1.root --fileout file:DIGI2.root \
--python_filename DIGI2_cfg.py \
--mc --eventcontent AODSIM \
--runUnscheduled --datatier AODSIM \
--conditions 102X_upgrade2018_realistic_v15 \
--step RAW2DIGI,L1Reco,RECO,RECOSIM,EI \
--procModifiers premix_stage2 \
--era Run2_2018 \
--no_exec
```
</details>


#### MINIAOD

<details>
<summary>2016</summary>
```
cmsDriver.py --filein file:DIGI2.root --fileout file:miniAOD.root \
--python_filename miniAOD_cfg.py \
--eventcontent MINIAODSIM \
--datatier MINIAODSIM \
--conditions 94X_mcRun2_asymptotic_v3 \
--step PAT \
--mc --runUnscheduled --scenario pp \
--era Run2_2016,run2_miniAOD_80XLegacy --no_exec
```
</details>

<details>
<summary>2017</summary>
```
cmsDriver.py --filein file:DIGI2.root --fileout file:miniAOD.root \
--python_filename miniAOD_cfg.py \
--eventcontent MINIAODSIM \
--datatier MINIAODSIM --conditions 94X_mc2017_realistic_v14 \
--step PAT \
--mc --runUnscheduled --scenario pp \
--era Run2_2017 --no_exec
```
</details>

<details>
<summary>2018</summary>
```
cmsDriver.py step1 --filein file:DIGI2.root --fileout file:miniAOD.root \
--python_filename miniAOD_cfg.py \
--eventcontent MINIAODSIM \
--datatier MINIAODSIM \
--conditions 102X_upgrade2018_realistic_v15 \
--step PAT --geometry DB:Extended \
--mc --runUnscheduled \
--era Run2_2018 --no_exec
```
</details>

#### Production with CRAB

Template for CRAB configuration files for each production step can be found in the [ProductionScripts](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts) directory.
These templates depend on the production step (some instructions differ, e.g. job splitting, etc.) but not on years.
Hence you can directly copy these templates and adapt them to your needs (make sure to link the proper gridpack path, refer to the proper config file names, etc.).

:information_source: *When running CRAB, it is most practical to set `config.Data.publication = True`. This way, a user-dataset is produced at each step and can be passed to the next step, with options `config.Data.inputDataset = '/DSname'` and `config.Data.inputDBS = 'phys03'`.
Alternatively, you can use the [GenerateInputPathFile.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/ConfigFiles/FullSim/GenerateInputPathFile.py) script to generate the filelist to be read (pass it via `config.Data.userInputFiles = open('fullpath.txt').readlines()`).*

### Final ntuples

:heavy_exclamation_mark: **These instructions are slightly outdated ! In particular, some more options should be included to process private SMEFT samples (include PDF, etc.).**

The instructions below are relevant for use within the DESY Top-Z group only, and are kept to a minimum.

Once produced, the private miniAOD samples should then be processed :
- first with the [TopAnalysis](https://gitlab.cern.ch/cms-desy-top/TopAnalysis/tree/Trilepton) code, which produces flat TTrees out of miniAOD samples. Use the `Trilepton` branch and fork your own repository ;
- then with the PoTATo private code. Create your own branch and fork your own repository.

#### TopAnalysis

- Main code : `TopAnalysis/plugins/NTupleWriter.cc` (compile with `scram b`).

- Define the list of private input files e.g. in this way :
```
[Configuration/python/MC/TEST/test_cff.py]

import FWCore.ParameterSet.Config as cms
import FWCore.Utilities.FileUtils as FileUtils # see : https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePythonTips#Running_on_more_than_255_files

mylist = FileUtils.loadListFromFile ('FULLPATH/inputs.txt')
readFiles = cms.untracked.vstring(*mylist)

maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
secFiles = cms.untracked.vstring()

source = cms.Source ("PoolSource",fileNames = readFiles, secondaryFileNames = secFiles)
```

where the list of private input files `inputs.txt` has been e.g. generated with the [GenerateInputPathFile.py](https://github.com/nicolastonon/EFT-Simu-Pheno/ProductionScripts/ConfigFiles/FullSim/GenerateInputPathFile.py) script.


- You can modify options in `Configuration/analysis/common/ntuple_cfg.py`. Also reference the list of input files e.g. in this way :
```
[Configuration/analysis/common/files_tllqdim6.txt]

# Syntax:
# (1) Number of Jobs
# (2) cff file in TopAnalysis.Configuration containing the input ROOT files
# (3) output ROOT file
# (4) options for ntuple_cfg.py, first argument is sampleName

#Example tllq/tzq
50   MC.TEST.tllqdim6_cff   tllqdim6.root   tzq,run=2017,eftweights=True,fastsim=True

#Example ttll/ttz
50   MC.TEST.ttlldim6_cff   ttlldim6.root   ttbarz,run=2017,eftweights=True,fastsim=True
```

- Run the HTCondor jobs :
```
./scripts/runall-condor.pl -d outputdir -f files_tllqdim6.txt -c ntuple_cfg.py -s
```

- Once the jobs are done, run the following script to check resubmit failed jobs or merge all the outputs :
```
nafJobSplitter-condor.pl -j check outputdir/naf_*/
```

#### PoTATo

- Define the list of input files, e.g. in this way :
```
[nicolas/python/test.py]

from common.python.InputFiles import InputFiles
from common.python.SampleMode import Mode, Sample

mytest = InputFiles('/afs/desy.de/user/n/ntonon/tzq.root', mode=Mode.ttHToNonbb_M125, sample=Sample.Fall17, jobs=0,)
```

- From the top directory, you can run the code interactively :
```
//Here : use 'nicolas' sub-dir, call 'Analyzer3l' analyzer, run on input files listed in 'nicolas/python/test.py', run with 2017 config, consider ttH process (naming, xsec, ...)

./run analyze -m nicolas -a Analyzer3l -f test -sm Fall17 ttHToNonbb_M125 --max 100
```

- For large-scale production, run jobs on HTCondor. E.g. to process all the ntuples from the Ntuple_V05 campaign, do :
```
./submit analyze -m nicolas -a Analyzer3l -f AllSamples -v V05_X

//Resubmit failed jobs
./submit check directories nicolas/output/dirname
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
_____________________________________________________________________________


<!-- OBSOLETE, TO VERIFY, ...
