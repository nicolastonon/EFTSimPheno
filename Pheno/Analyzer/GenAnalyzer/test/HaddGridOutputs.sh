#Name of the subdir containing the rootfiles to merge #e.g. : tllqdim6_FASTSIM1_v1/Analyzer_v2/200325_160027/0000
DIR=tllqdim6_FASTSIM1_v1/Analyzer_v2/200325_160027/0000

echo ""
echo "...Start copy from "$DIR
echo ""

mkdir tmp #tmp dir, download rootfiles
gfal-copy -r srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/ntonon/$DIR ./tmp

echo ""
echo "...Start merge"
echo ""
hadd -f merged.root ./tmp/*root #merge
rm -rf ./tmp #delete tmp files

echo ""
echo "...Done"
