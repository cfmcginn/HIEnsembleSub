#!/bin/bash    

if [[ $ENSEMBLESUBPATH= == *"/"* ]]
then
    echo "Using ENSEMBLESUBPATH= '$ENSEMBLESUBPATH'"
else
    echo "ENSEMBLESUBPATH not set. source setEnv.sh. exit 1"
    exit 1
fi


#filesHYD=(/home/cfmcginn/Samples/HYDJET/hydjet_MERGED_Grendel_20190205.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_Grendel_20190210.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_Grendel_20190214.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_SVM_20190205.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_SVM_20190211.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_SVM_20190213.root /home/cfmcginn/Samples/HYDJET/hydjet_MERGED_SVM_20190217.root)

#filesHYDTopPath="/home/cfmcginn/Samples/HYDJET"
filesHYDTopPath="/media/cfmcginn/SeagateExternal/Samples/HYDJET"
filesHYDStr=""

#for i in "${filesHYD[@]}"
for i in $filesHYDTopPath/hyd*.root
do
    filesHYDStr=$filesHYDStr$i,
#    break
done

if [[ -f $ENSEMBLESUBPATH/bin/hydjetToHFCent.exe ]]
then
    $ENSEMBLESUBPATH/bin/hydjetToHFCent.exe $filesHYDStr 0
else
    echo "ERROR: $ENSEMBLESUBPATH/bin/hydjetToHFCent.exe not found"
fi

