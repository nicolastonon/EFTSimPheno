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

#### Useful links

:link: Combine page (browse the *Running combine* tab): http://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/

:link: CMS Combine Hypernews : https://hypernews.cern.ch/HyperNews/CMS/get/higgs-combination.html

:link: CombineHarvester page (external lib) : http://cms-analysis.github.io/CombineHarvester/

_____________________________________________________________________________
#### Table Of Contents

* [Installation](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#installation)

* [Datacards](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#datacards)
  * [Details and setup](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#details-and-setup)
  * [Example](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#example)
  * [MC statistical uncertainty](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#mc-statistical-uncertainty)

* [Combine commands](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#combine-commands)
  * [Expected significance](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#expected-significance-profile-likelihood)
  * [Observed significance](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#observed-significance-profile-likelihood)
  * [Asymptotic frequentist limits](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#asymptotic-frequentist-limits)
  * [Postfit templates, nuisances, etc.](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#postfit-templates-nuisances-etc-mlf)
  * [Nuisance parameters](https://github.com/nicolastonon/tHq-analysis/tree/master/COMBINE#nuisance-parameters)


_____________________________________________________________________________


# Installation

- To install the analysis code & COMBINE code, follow instructions from the main [README](../README.md).

- Move to COMBINE dir.

```
cd tHq-analysis/COMBINE
```

**The 'templates' dir. must contain a file named `Combine_Input.root`, to which the datacards are pointing.**

:information_source: *Make sure that you're using the right input file (read the main [README](../README.md) to know how to create the templates)*

:information_source: *For Template files, we use the convention : `Combine_Input_SUFFIX.root` (tbc)*


# Datacards

The logic to create the datacards is the following : first, the user should configure all (3) codes depending on his needs. More details below.
Once this is done, all you have to do is to execute a code, which will interactively ask you to select options, and will in turn call a python code to parse a template file. This way, it is easy to create as many datacards as needed, based on the same template/conventions.

## Details and setup
- Move to *datacards* dir. This directory contains codes for generating/combining all needed datacards. Compile the codes :

  ```
  cd tHq-analysis/COMBINE/datacards
  make
  ```

- [Generate_Datacard_Template.cxx](Generate_Datacard_Template.cxx) : code creating the datacard template that will be used to produce the final datacards. It is there that you can modify the skeleton of the datacard itself (e.g. the names of processes, the systematics, etc.).
This code should be configured so that all the needed ingredients for the final datacards are there : e.g. all the processes, all the systematics, etc. /!\ It is better to put more than too little in the template (as it is easy to simply comment out the un-desired lines in the next steps).

  Executing it will generate the template file 'Template_Datacard.txt' :

  ```
  ./Generate_Datacard_Template.exe
  ```

- [Parser_Datacard_Template.py](Parser_Datacard_Template.py)* : code parsing the datacard template, and replacing some key-words by the desired values (e.g. : name of channel, value of systematic, etc.).
This makes it easy to adapt the template to your needs (e.g. choose in which region/channel you want the datacard, with/without shape systematics, etc.).
You can either call the script manually by giving it the required arguments, or - as recommended -, call it via 'ScriptProducer_MakeAllDatacards.cxx'.


- [ScriptProducer_MakeAllDatacards.cxx](ScriptProducer_MakeAllDatacards.cxx) : at execution, the user can choose here the main options for the generation of datacard. This is the last, higher level of "configurability", where you should configure everything that needs to be modified often (e.g. region, value of a given systematic, etc.).
  ```
  ./ScriptProducer_MakeAllDatacards.exe
  ```

- This should have a produced a final script, named either 'Script_Make_Datacards_[SUFFIX].sh' depending on what you chose (template fit or postfit input variables). Run this script to generate the final, combined datacard (as well as all the un-combined datacards, placed in a dedicated dir.).
  ```
  ./Script_Make_Datacards_[SUFFIX].sh
  ```



## Example

Let's say you want to make some study on a systematic, and so as a user you will need to change it's value very often. Then you should probably :

1) Modify [Generate_Datacard_Template.cxx](Generate_Datacard_Template.cxx) (or the template datacard itself) so that the template datacard will contain a special key-word at the place where the value for this systematic should be

2) Modify [Parser_Datacard_Template.py](Parser_Datacard_Template.py) so that it accepts this syst value as an argument, and so that it replaces the special key-word by the value in the template

3) Modify [ScriptProducer_MakeAllDatacards.cxx](ScriptProducer_MakeAllDatacards.cxx) so that it makes it easy for the user to choose the value (either interactively, or in the main for example), and so that this value is given as argument when the python script is called.

:arrow_right: This way, you will only need to configure and compile the codes once. Then, all the user has to do is change the value when he executes 'ScriptProducer_MakeAllDatacards.exe', and the final datacards will contain this value.
This might need a bit troublesome (modifying 3 codes while you could 'just' mess with a template by youself), but it is meant to ensure scalability and automation of the datacard production. Hopefully, once you are satisfied with your datacards, there is no need to modify these codes anymore.


## MC statistical uncertainty

In the past, you had to use the additional CombineHarvester package in order to add bin-per-bin MC stat. uncertainty.

This functionnality was included in recent Combine versions. All you need to do is to add this line at the end of your datacard *(done by default)* :
```
[channel] autoMCStats [threshold] [include-signal = 0] [hist-mode = 1]
```
See the [doc](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part2/bin-wise-stats/#automatic-statistical-uncertainties) for details.




# Combine commands

*(Make sure you're using the right datacard name in the commands, `COMBINED_datacard.txt` is a dummy name)*

:information_source: *If you will use Combine a lot, you'll find it hepful to set-up a few aliases (for all commands)...*

- To derive limits and significance, we use the **Asymptotic likelihood** methods :
  - `AsymptoticLimits`: limits calculated according to the asymptotic formulas in [arxiv:1007.1727](https://arxiv.org/abs/1007.1727)
  - `Significance`: simple profile likelihood approximation, for calculating significances.

- To access postfit informations (shapes, uncertainties, etc.), we also use the **Fitting** methods :
  - `FitDiagnostics` [[doc](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/nonstandard/#fitting-diagnostics)] : performs maximum likelihood fits to extract the signal yield and provide diagnostic tools such as pre and post-fit models and correlations
  - `MultiDimFit` [[doc](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/commonstatsmethods/#likelihood-fits-and-scans)] :  perform multi-dimensional fits and likelihood based scans/contours using models with several parameters of interest.



## Expected significance (Profile Likelihood)

The expected significance can be computed from an Asimov dataset of signal+background. There are two options for this :

- *a-posteriori* expected: will depend on the observed dataset.
- *a-priori* expected (the default behavior): does not depend on the observed dataset, and so is a good metric for optimizing an analysis when still blinded.

The a-priori expected significance from the Asimov dataset is calculated as :
```
combine -M Significance -t -1 --expectSignal=1 COMBINED_datacard.txt
```

:information_source: `-t -1` <-> Use Asimov Dataset. To use toys instead, use `-t N`, with N the number of toys

In order to produced the a-posteriori expected significance, just generate a post-fit Asimov (i.e add the option `--toysFreq` in the command above).
The output format is the same as for observed signifiances: the variable limit in the tree will be filled with the significance (or with the p-value if you put also the option `--pvalue`)


## Observed significance (Profile Likelihood)
The observed significance is calculated using the `Significance` method, as :

```
combine -M Significance COMBINED_datacard.txt
```

In case the fit has difficulties to converge, consider adding the following options :

```
combine -M Significance COMBINED_datacard.txt --rMin=0 --rMax=20 --X-rtd ADDNLL_RECURSIVE=0 --cminDefaultMinimizerStrategy 0 --cminDefaultMinimizerTolerance 0.01 --cminPreScan
```

:information_source: *Use option `-v X` (with X>=2) for verbose mode. With X=4, can access postfit POI values & get warning/error messages*

:information_source: *The output root file will contain the significance value in the branch limit. To store the p-value instead, include the option --pval*


## Asymptotic Frequentist Limits

### Un-blinded limits

:cl: The `AsymptoticLimits` method allows to compute quickly an estimate of the observed and expected limits, which is fairly accurate when the event yields are not too small and the systematic uncertainties don't play a major role in the result. The limit calculation relies on an asymptotic approximation of the distributions of the LHC test-statistic, which is based on a profile likelihood ratio, under signal and background hypotheses to compute CLs+b, CLb and therefore CLs=CLs+b/CLb - i.e it is the asymptotic approximation of computing limits with frequentist toys. This method is so commonly used that it is the default method.

The method can be run using :
```
combine -M AsymptoticLimits COMBINED_datacard.txt
```
The program will print out the limit on the signal strength r (number of signal events / number of expected signal events) e .g. `Observed Limit: r < 1.6297 @ 95% CL` , the median expected limit `Expected 50.0%: r < 2.3111` and edges of the 68% and 95% ranges for the expected limits.

:information_source: *By default, the limits are calculated using the CLs prescription, as noted in the output, which takes the ratio of p-values under the signal plus background and background only hypothesis. This can be altered to using the strict p-value by using the option `--rule CLsplusb` You can also change the confidence level (default is 95%) to 90% using the option `--cl 0.9` or any other confidence level. You can find the full list of options for `AsymptoticLimits` using `--help -M AsymptoticLimits`*



### Blinded limits

The `AsymptoticLimits` calculation follows the frequentist paradigm for calculating expected limits. This means that the routine will first fit the observed data, conditionally for a fixed value of r and set the nuisance parameters (NPs) to the values obtained in the fit for generating the Asimov data, i.e it calculates the post-fit or a-posteriori expected limit. In order to use the pre-fit nuisance parameters (to calculate an a-priori limit), you must add the option --noFitAsimov or `--bypassFrequentistFit`.

For blinding the results completely (i.e not using the data) you must include the option `--run blind` :
```
combine -M AsymptoticLimits --noFitAsimov --run blind COMBINED_datacard.txt
```

:no_entry_sign: From the instructions :
> **You should never use -t -1 to get blind limits!**

*(Otherwise, the "nominal" nuisance parameter values are taken from fits to the data and are therefore not "blind" to the observed data by default (following the fully frequentist paradigm) ).*



## Postfit templates, nuisances, etc. (MLF)

:heavy_exclamation_mark: You need to create output dir. first, to avoid segfault.

For details on recipes and outputs, it is advised to refer to the documentation page ([FitDiagnostics](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/nonstandard/#plotting)).

Run the following command :
```
combine -M FitDiagnostics -t -1 --expectSignal 1 --saveShapes --saveWithUncertainties --plots --out outputs COMBINED_datacard.txt #expected
```
or if you want to unblind the data :
```
combine -M FitDiagnostics --saveShapes --saveWithUncertainties --plots --out outputs COMBINED_datacard.txt #observed
```
This will produce plots and a `fitDiagnostics.root` file in the specified output directory. This file contains many informations which can be later used to produce postfit plots, retrieve postfit uncertainties, postfit NPs, etc.

- `--plots` : will produce pre/post-fit plots in the output directory (however these plots are not really satisfying : backgrounds are not split, etc., so you probably want to produce them by yourself).

- `--saveShapes` : will produce additional folders in `fitDiagnostics.root` containing the pre/post-fit distributions for each category/process. The folders are named `shapes_prefit`/`shapes_fit_sb`/`shapes_fit_b` (corresponding to prefit/fit with null signal/fit with floating signal). The distributions are :
  - `data` : TGraphAsymmErrors containing the observed data (or toy data if using `-t`). The vertical error bars correspond to the 68% interval for a Poisson distribution centered on the observed count.
  - `$PROCESS` : TH1F for each process in channel, named as in the datacard.
  - `total_signal`, `total_background`, `total`.

  The above distributions are provided for each channel included in the datacard, in separate sub-folders, named as in the datacard: there will be one sub-folder per channel.

  :information_source: *The pre-fit signal is by default for r=1 but this can be modified using the option --prefitValue.*

  :information_source: *The histograms/data points will have the bin number as the x-axis and the content of each bin will be a number of events.*

- `--saveWithUncertainties` : add uncertainties on the shapes. These uncertainties are generated by re-sampling of the fit covariance matrix, thereby accounting for the full correlation between the parameters of the fit.

  :information_source: *It may be tempting to sum up the uncertainties in each bin (in quadrature) to get the total uncertainty on a process however, this is (usually) incorrect as doing so would not account for correlations between the bins. Instead you can refer to the uncertainties which will be added to the post-fit normalizations described above.*

  :information_source: *This [HN answer](https://hypernews.cern.ch/HyperNews/CMS/get/higgs-combination/991/1.html) further explains how the prefit error bars are obtained : "random sets of nuisance parameters are generated from their respective pdfs and the shapes are reevaluated each time. The RMS of the deviation with respect to the nominal bin content is used to define the bin error."*


## Nuisance parameters

### Pre/postfit NPs and pulls [doc](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/nonstandard/#pre-and-post-fit-nuisance-parameters-and-pulls)
*It is possible to compare pre-fit and post-fit nuisance parameters with the script `diffNuisances.py` (copied locally in the Plotting/ dir. for conveninence).
Taking as input a `fitDiagnostics.root` file, the script will by default print out the parameters which have changed significantly w.r.t. their initial estimate. For each of those parameters, it will print out the shift in value and the post-fit uncertainty, both normalized to the input values, and the linear correlation between the parameter and the signal strength.
The output by default will be the changes in the nuisance parameter values and uncertainties, relative to their initial (pre-fit) values (usually relative to initial values of 0 and 1 for most nuisance types).*

- Run the Combine script from the `Plotting/` dir. :
  ```
  cd Plotting
  python diffNuisances.py -g nuisances.root ../datacards/outputs/fitDiagnostics.root  --all --abs
  ```
  :arrow_right: This will create a `nuisances.root` file containing prefit/postfit infos on the NPs.

- You can then produce a plot out of it with :
  ```
  root -l Draw_Canvas.C
  ```

  :arrow_right: This creates 2 png plots.


### Impact of NPs [[doc](https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/part3/nonstandard/#nuisance-parameter-impacts)]

*It is possible to estimate the impact of the nuisance parameters.
This is effectively a measure of the correlation between the NP and the POI, and is useful for determining which NPs have the largest effect on the POI uncertainty.*

:information_source: This can be quite long and un-readable if all the NPs are included.
You might want to remove the `autoMCStats` lines from the datacard in order to ignore the statistical uncert. for all bins...

:information_source: You can rename parameters by editing rename.json, for example :
```
{
  "r" : "#mu",
  "lumi" : "luminosity"
}
```


- Run the bash script :
  ```
  cd Plotting
  ./Plot_Syst_Impacts.sh [DATACARD_NAME_NOEXT] [EXP_OBS]
  ```
  :information_source: The 1rst argument is the name of the datacard without its file extension. The 2nd argument ie either "exp" or "obs", depending if you want to blind the data or not.
  :arrow_right: This creates a pdf plot representing the pulls of the NPs.


<!--
# OBSOLETE (needs update)

:heavy_exclamation_mark: **Here are outdated commands, for the record only**

### Breakdown of systematics

- To do the break-down of systematics : [Instructions](https://indico.cern.ch/event/577649/contributions/2388797/attachments/1380376/2098158/HComb-Tutorial-Nov16-Impacts.pdf)

- To estimate stat. uncertainty contribution : [Instructions](https://indico.cern.ch/event/577649/contributions/2388797/attachments/1380376/2098158/HComb-Tutorial-Nov16-Impacts.pdf)

- Used to be these commands (**needs update**) :
  ```
  text2workspace.py datacard.txt
  combine -M MultiDimFit --algo grid --points 50 --rMin -1 --rMax 4  datacard.root -m 125 -n nominal --expectSignal=1 -t -1
  combine -M MultiDimFit --algo none --rMin -1 --rMax 4  datacard.root -m 125 -n bestfit  --saveWorkspace --expectSignal=1 -t -1
  combine -M MultiDimFit --algo grid --points 50 --rMin -1 --rMax 4 -m 125 -n stat higgsCombinebestfit.MultiDimFit.mH125.root --snapshotName MultiDimFit --freezeParameters all --expectSignal=1 -t -1
  plot1DScan.py --others 'higgsCombinestat.MultiDimFit.mH125.root:Freeze all:2' --breakdown syst,stat higgsCombinenominal.MultiDimFit.mH125.root
  ```


### Multiple signals

- To compute EXPECTED (cf. '-t -1' options) significance SIMULTANEOUSLY for ttZ & tZq :
```
text2workspace.py -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel  --PO verbose --PO 'map=.*/tZqmcNLO:r_tZqmcNLO[1,0,10]' --PO 'map=.*/ttZ:r_ttZ[1,0,20]' COMBINED_datacard_TemplateFit_tZqANDttZ.txt -o toy-2d.root
combine toy-2d.root  -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZqmcNLO=0 --redefineSignalPOIs r_tZqmcNLO --expectSignal=1 -t -1
combine toy-2d.root -M ProfileLikelihood --signif --redefineSignalPOIs r_tZqmcNLO --setPhysicsModelParameters r_rZqmcNLO=1,r_ttZ=1 --expectSignal=1 -t -1
combine workspace.root -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZq=1,r_ttZ=1
combine workspace.root -M HybridNew --onlyTestStat --testStat=PL --singlePoint r_tZq=1,r_ttZ=1 --setPhysicsModelParameters r_rZqmcNLO=1,r_ttZ=1 --expectSignal=1 -t -1
```
-------------------------------------------->
