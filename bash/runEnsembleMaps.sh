#!/bin/bash    

if [[ $ENSEMBLESUBPATH= == *"/"* ]]
then
    echo "Using ENSEMBLESUBPATH= '$ENSEMBLESUBPATH'"
else
    echo "ENSEMBLESUBPATH not set. source setEnv.sh. exit 1"
    exit 1
fi

fileHYD="/home/cfmcginn/Samples/HYDJET/genHydjetSkimFromForest_20181214.root"
filePYT="/home/cfmcginn/Samples/PYTHIA8/testPthat_MERGED0to9_CUETP8M1_NEvt500000_DoFlatPthatFalse_PthatMin20p0_PthatMax999999p0_20181212.root,/home/cfmcginn/Samples/PYTHIA8/testPthat_MERGED0to9_CUETP8M1_NEvt100000_DoFlatPthatFalse_PthatMin80p0_PthatMax999999p0_20190122.root"
ptCut="30,80"

if [[ -f $ENSEMBLESUBPATH/bin/createEnsembleMaps.exe ]]
then
    $ENSEMBLESUBPATH/bin/createEnsembleMaps.exe $fileHYD $filePYT $ptCut
else
    echo "ERROR: $ENSEMBLESUBPATH/bin/createEnsembleMaps.exe not found"
fi

