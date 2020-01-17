#Nicolas -- This script merges the ntuples from all 3 years ; these merged files will be used when processing the full run 2 dataset

#New repo, store merged files
outDir="Run2"
mkdir $outDir

#Get list of samples from 2018 repository
dirCheck="./2018"
sampleList=$(ls $dirCheck | cut -d"." -f1)
echo $sampleList

#For each sample, merge all 3 years
for sample in $sampleList
do
    # echo $sample
    hadd -f $outDir/$sample.root 2016/$sample.root 2017/$sample.root 2018/$sample.root
done
