#!/bin/bash

if [[ $ENSEMBLESUBPATH= == *"/"* ]]
then
    echo "Using ENSEMBLESUBPATH= '$ENSEMBLESUBPATH'"
else
    echo "ENSEMBLESUBPATH not set. source setEnv.sh. exit 1"
    exit 1
fi


ensFile="/home/cfmcginn/Projects/EnsembleSub/output/20190127/ensembleMaps_20190123_EnsembleMapHistID_20190127.root"
#ensFile="/home/cfmcginn/Projects/EnsembleSub/output/20190124/ensembleMaps_20190123_EnsembleMapHistID_20190124.root"
hydFile="/home/cfmcginn/Samples/HYDJET/genHydjetSkimFromForest_20181214.root"
pytFilesArr=("/home/cfmcginn/Samples/PYTHIA8/testPthat_MERGED0to9_CUETP8M1_NEvt500000_DoFlatPthatFalse_PthatMin20p0_PthatMax999999p0_20181212.root")
pytPtMinsArr=("30")
pytPtMaxsArr=("60")

pytFiles=""
pytPtMins=""
pytPtMaxs=""

pos=0
for j in "${pytFilesArr[@]}"
do
    pytFiles="$pytFiles$j,"
    pytPtMins="$pytPtMins${pytPtMinsArr[$pos]},"
    pytPtMaxs="$pytPtMaxs${pytPtMaxsArr[$pos]},"
done

./bin/testEnsembleMaps.exe $ensFile $hydFile $pytFiles $pytPtMins $pytPtMaxs
#valgrind --track-origins=yes --show-leak-kinds=all --leak-check=full -v ./bin/testEnsembleMaps.exe $ensFile $hydFile $pytFiles $pytPtMins $pytPtMaxs
