#!/bin/bash    

if [[ $ENSEMBLESUBPATH= == *"/"* ]]
then
    echo "Using ENSEMBLESUBPATH= '$ENSEMBLESUBPATH'"
else
    echo "ENSEMBLESUBPATH not set. source setEnv.sh. exit 1"
    exit 1
fi

inFileName="output/20190123/ensembleMaps_20190123.root"
ptCut="30,80"

if [[ -f $ENSEMBLESUBPATH/bin/quickMedianCheck.exe ]]
then
    $ENSEMBLESUBPATH/bin/quickMedianCheck.exe $inFileName $ptCut
else
    echo "ERROR: $ENSEMBLESUBPATH/bin/quickMedianCheck.exe not found"
fi

