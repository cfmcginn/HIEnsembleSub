#ifndef RETURNROOTFILECONTENTSLIST_H
#define RETURNROOTFILECONTENTSLIST_H

//cpp dependencies
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TCollection.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TKey.h"
#include "TList.h"

//Local (Utility) dependencies
#include "include/cppWatch.h"
#include "include/doGlobalDebug.h"
#include "include/stringUtil.h"

const std::string tdirStr = "TDirectoryFile";

std::vector<std::string> returnTDirContentsList(TFile*, const std::string, const std::string, const std::string, const Int_t currentLayers, const Int_t nLayersCap, std::vector<std::string>* classList);

TList* returnTDirContentsList(TFile* inFile_p, const std::string dirName, const Int_t currentLayers, const Int_t nLayersCap);

std::vector<std::string> returnRootFileContentsList(TFile *inFile_p, const std::string classFilter = "", const std::string nameFilter = "", const Int_t nLayersCap = -1, std::vector<std::string>* classList = NULL)
{
  inFile_p->cd();

  std::vector<std::string> returnList;
  
  TList* inListOfKeys = inFile_p->GetListOfKeys();
  TIter next(inListOfKeys);
  TKey* key = NULL;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(isStrSame(className, tdirStr)){
      std::vector<std::string>* tempClassList = new std::vector<std::string>;
      std::vector<std::string> tempReturnList = returnTDirContentsList(inFile_p, name, classFilter, nameFilter, 0, nLayersCap, tempClassList);

      const Int_t nTempKeys = tempReturnList.size();
      for(Int_t tempIter = 0; tempIter < nTempKeys; tempIter++){
	returnList.push_back(tempReturnList.at(tempIter));
	if(classList != NULL) classList->push_back(tempClassList->at(tempIter));
      }

      delete tempClassList;
    }

    if(classFilter.size() != 0){
      if(className.size() != classFilter.size() || className.find(classFilter) == std::string::npos){
	//if(key != NULL) delete key;
	continue;
      }
    }
    
    if(nameFilter.size() != 0){
      if(name.find(nameFilter) == std::string::npos){
	//if(key != NULL) delete key;
	continue;
      }
    }
    
    returnList.push_back(name);
    if(classList != NULL) classList->push_back(className);
  
    //if(key != NULL) delete key;
  }

  //  if(inListOfKeys != NULL) delete inListOfKeys;

  return returnList;
}


std::vector<std::string> returnTDirContentsList(TFile* inFile_p, const std::string dirName, const std::string classFilter, const std::string nameFilter, const Int_t currentLayers, const Int_t nLayersCap = -1, std::vector<std::string>* classList = NULL)
{
  inFile_p->cd();

  std::vector<std::string> returnList;
  if(nLayersCap > 0 && currentLayers+1 >= nLayersCap) return returnList;
    
  TDirectoryFile* dir_p = (TDirectoryFile*)inFile_p->Get(dirName.c_str());
  TList* inListOfKeys = (TList*)dir_p->GetListOfKeys();
  TIter next(inListOfKeys);
  TKey* key = NULL;

  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(isStrSame(className, tdirStr)){
      std::vector<std::string>* tempClassList = new std::vector<std::string>;
      std::vector<std::string> tempReturnList = returnTDirContentsList(inFile_p, dirName + "/" + name, classFilter, nameFilter, currentLayers+1, nLayersCap, tempClassList);
      
      const Int_t nTempKeys = tempReturnList.size();
      for(Int_t tempIter = 0; tempIter < nTempKeys; tempIter++){
        returnList.push_back(tempReturnList.at(tempIter));
	if(classList != NULL) classList->push_back(tempClassList->at(tempIter));
      }

      delete tempClassList;
    }

    if(classFilter.size() != 0){
      if(className.size() != classFilter.size() || className.find(classFilter) == std::string::npos){
	//	//if(key != NULL) delete key;
	continue;
      }
    }
    
    if(nameFilter.size() != 0){     
      if(name.find(nameFilter) == std::string::npos && dirName.find(nameFilter) == std::string::npos){
	//if(key != NULL) delete key;
	continue;
      }
    }

    returnList.push_back(dirName + "/" + name);
    if(classList != NULL) classList->push_back(className);

    //if(key != NULL) delete key;
  }

  //  if(inListOfKeys != NULL) delete inListOfKeys;
  //  if(dir_p != NULL) delete dir_p;
  
  return returnList;
}



//TList versions
TList* returnRootFileContentsList(TFile *inFile_p, const Int_t nLayersCap = -1)
{
  inFile_p->cd();

  TList* returnList = inFile_p->GetListOfKeys();
  TIter next(returnList);
  TKey* key = NULL;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();
    
    if(isStrSame(className, tdirStr)){
      TList* tempReturnList = returnTDirContentsList(inFile_p, name, 0, nLayersCap);
      if(tempReturnList == NULL){
	//if(key != NULL) delete key;
	continue;
      }
      TIter tempNext(tempReturnList);
      TObject* obj = NULL;

      while((obj = tempNext())){
	returnList->AddLast(obj);
      }
    }

    //if(key != NULL) delete key;
  }

  return returnList;
}


TList* returnTDirContentsList(TFile* inFile_p, const std::string dirName, const Int_t currentLayers, const Int_t nLayersCap = -1)
{
  inFile_p->cd();

  TList* returnList = NULL;

  if(nLayersCap > 0 && currentLayers+1 >= nLayersCap) return returnList;
  
  TDirectoryFile* dir_p = (TDirectoryFile*)inFile_p->Get(dirName.c_str());
  returnList = dir_p->GetListOfKeys();
  TIter next(returnList);
  TKey* key = NULL;
  while((key=(TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(isStrSame(className, tdirStr)){
      TList* tempReturnList = returnTDirContentsList(inFile_p, dirName + "/" + name, currentLayers+1, nLayersCap);
      if(tempReturnList == NULL){
	//if(key != NULL) delete key;
	continue;
      }

      TIter tempNext(tempReturnList);
      TObject* obj = NULL;

      while( (obj = tempNext()) ){
	returnList->AddLast(obj);
      }
    }

    //if(key != NULL) delete key;
  }

  //  if(dir_p != NULL) delete dir_p;

  return returnList;
}

#endif
