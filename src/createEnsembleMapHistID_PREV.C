//cpp dependencies
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT dependencies
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TH2F.h"
#include "TNamed.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/returnRootFileContentsList.h"

int createEnsembleMapHistID(const std::string inFileName)
{
  cppWatch timer;
  timer.start();

  if(!checkFile(inFileName)){
    std::cout << "inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }
  else if(inFileName.find(".root") == std::string::npos){
    std::cout << "inFileName \'" << inFileName << "\' is not root file. return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  
  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".root") != std::string::npos) outFileName.replace(outFileName.find(".root"), 5, "");
  outFileName = "output/" + dateStr + "/" + outFileName + "_EnsembleMapHistID_" + dateStr + ".root";
  
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* entryTree1_p = new TTree("entryTree1", "");
  TTree* medianTree1_p = new TTree("medianTree1", "");

  TTree* entryTree2_p = new TTree("entryTree2", "");
  TTree* medianTree2_p = new TTree("medianTree2", "");
  
  UInt_t origEntry;
  const UInt_t nTowers = 1062;
  UInt_t eventID;
  Float_t towersMedian[nTowers];
  
  entryTree1_p->Branch("origEntry", &origEntry, "origEntry/i");

  medianTree1_p->Branch("eventID", &eventID, "eventID/i");
  medianTree1_p->Branch("towersMedian", towersMedian, ("towersMedian[" + std::to_string(nTowers) + "]/F").c_str());
  
  entryTree2_p->Branch("origEntry", &origEntry, "origEntry/i");

  medianTree2_p->Branch("eventID", &eventID, "eventID/i");
  medianTree2_p->Branch("towersMedian", towersMedian, ("towersMedian[" + std::to_string(nTowers) + "]/F").c_str());

  std::map<UInt_t, UInt_t> mapEventIDToCounts;
  std::map<UInt_t, std::vector<ULong64_t> > mapEventIDToEntries1;
  std::map<UInt_t, std::vector<ULong64_t> > mapEventIDToEntries2;

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> paramNames = returnRootFileContentsList(inFile_p, "TNamed", "");
  std::vector<std::string> paramTitles;
  
  for(unsigned int pI = 0; pI < paramNames.size(); ++pI){
    if(paramNames[pI].find("paramsD") == std::string::npos) continue;
    paramTitles.push_back(((TNamed*)inFile_p->Get(paramNames[pI].c_str()))->GetTitle());
    paramNames[pI].replace(0, paramNames[pI].find("/")+1, "");
  }

  for(unsigned int pI = 0; pI < paramNames.size(); ++pI){
    std::cout << paramNames[pI] << ", " << paramTitles[pI] << std::endl;
  }
  
  TTree* hydTree1_p = (TTree*)inFile_p->Get("hydTree1");
  TTree* hydTree2_p = (TTree*)inFile_p->Get("hydTree2");  
  
  Float_t etaPhiSum[nTowers];
  
  hydTree1_p->SetBranchStatus("*", 0);
  hydTree1_p->SetBranchStatus("origEntry", 1);
  hydTree1_p->SetBranchStatus("eventID", 1);

  hydTree1_p->SetBranchAddress("origEntry", &origEntry);
  hydTree1_p->SetBranchAddress("eventID", &eventID);

  hydTree2_p->SetBranchStatus("*", 0);
  hydTree2_p->SetBranchStatus("origEntry", 1);
  hydTree2_p->SetBranchStatus("eventID", 1);

  hydTree2_p->SetBranchAddress("origEntry", &origEntry);
  hydTree2_p->SetBranchAddress("eventID", &eventID);
      
  const ULong64_t nEntries1 = (ULong64_t)hydTree1_p->GetEntries();

  for(ULong64_t entry = 0; entry < nEntries1; ++entry){
    hydTree1_p->GetEntry(entry);

    ++(mapEventIDToCounts[eventID]);
    if(mapEventIDToCounts[eventID] == 1){
      mapEventIDToEntries1[eventID] = {};
      mapEventIDToEntries2[eventID] = {};
    }
    mapEventIDToEntries1[eventID].push_back(entry);
    
    entryTree1_p->Fill();
  }  

  const ULong64_t nEntries2 = (ULong64_t)hydTree2_p->GetEntries();

  for(ULong64_t entry = 0; entry < nEntries2; ++entry){
    hydTree2_p->GetEntry(entry);

    mapEventIDToEntries2[eventID].push_back(entry);

    entryTree2_p->Fill();
  }

  UInt_t idCounts = 0;
  for(auto const & val : mapEventIDToCounts){
    ++idCounts;
  }
  std::cout << "IDCounts: " << idCounts << std::endl;

  UInt_t nDiv = TMath::Max(idCounts/40, (UInt_t)1);
  UInt_t pos = 0;

  std::map<UInt_t, std::map<UInt_t, std::vector<Float_t> > > fullTowerPosToSumVect1;
  std::map<UInt_t, std::map<UInt_t, std::vector<Float_t> > > fullTowerPosToSumVect2;

  std::map<UInt_t, std::vector<Float_t> > towerPosToSumVect1;
  std::map<UInt_t, std::vector<Float_t> > towerPosToSumVect2;
  for(UInt_t i = 0; i < nTowers; ++i){
    towerPosToSumVect1[i] = {};
    towerPosToSumVect2[i] = {};

    for(auto const & val : mapEventIDToCounts){
      (fullTowerPosToSumVect1[i])[val.first] = {};
      (fullTowerPosToSumVect2[i])[val.first] = {};
    }
  }

  hydTree1_p->SetBranchStatus("etaPhiSum", 1);
  hydTree1_p->SetBranchAddress("etaPhiSum", etaPhiSum);

  hydTree2_p->SetBranchStatus("etaPhiSum", 1);
  hydTree2_p->SetBranchAddress("etaPhiSum", etaPhiSum);

  cppWatch timerBuild;
  cppWatch timerSort;
  
  for(auto const & val : mapEventIDToCounts){    
    if(pos%nDiv == 0) std::cout << "Processing id " << pos << "/" << idCounts << std::endl;
    ++pos;

    timerBuild.start();  

    for(UInt_t i = 0; i < nTowers; ++i){
      towerPosToSumVect1[i].clear();
      towerPosToSumVect2[i].clear();     
    }

    for(auto const & entry : mapEventIDToEntries1[val.first]){
      hydTree1_p->GetEntry(entry);
      for(UInt_t i = 0; i < nTowers; ++i){
	towerPosToSumVect1[i].push_back(etaPhiSum[i]);
	(fullTowerPosToSumVect1[i])[val.first].push_back(etaPhiSum[i]);
      }
    }

    timerBuild.stop();  
    timerSort.start();  

    for(auto const & val : towerPosToSumVect1){
      std::vector<Float_t> temp = val.second;
      std::sort(std::begin(temp), std::end(temp));

      UInt_t medPos = temp.size()/2;
      Float_t medianVal = temp[medPos];
      if(temp.size()%2 == 0) medianVal = (medianVal + temp[medPos-1])/2.;

      towersMedian[val.first] = medianVal;      
    }

    eventID = val.first;
    medianTree1_p->Fill();

    timerSort.stop();  
    timerBuild.start();  
    
    for(auto const & entry : mapEventIDToEntries2[val.first]){
      hydTree2_p->GetEntry(entry);
      for(UInt_t i = 0; i < nTowers; ++i){
	towerPosToSumVect2[i].push_back(etaPhiSum[i]);
	(fullTowerPosToSumVect2[i])[val.first].push_back(etaPhiSum[i]);
      }
    }
    
    timerBuild.stop();  
    timerSort.start();  

    for(auto const & val : towerPosToSumVect2){
      std::vector<Float_t> temp = val.second;
      std::sort(std::begin(temp), std::end(temp));

      UInt_t medPos = temp.size()/2;
      Float_t medianVal = temp[medPos];
      if(temp.size()%2 == 0) medianVal = (medianVal + temp[medPos-1])/2.;

      towersMedian[val.first] = medianVal;      
    }

    eventID = val.first;
    medianTree2_p->Fill();

    timerSort.stop();  

    if(pos >= 10) break;
  }

  std::cout << "Total Build timeCPU: " << timerBuild.totalCPU() << std::endl;
  std::cout << "Total Build timeWall: " << timerBuild.totalWall() << std::endl;
  std::cout << std::endl;
  std::cout << "Total Sort timeCPU: " << timerSort.totalCPU() << std::endl;
  std::cout << "Total Sort timeWall: " << timerSort.totalWall() << std::endl;
  
  
  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();

  entryTree1_p->Write("", TObject::kOverwrite);
  delete entryTree1_p;

  medianTree1_p->Write("", TObject::kOverwrite);
  delete medianTree1_p;

  entryTree2_p->Write("", TObject::kOverwrite);
  delete entryTree2_p;

  medianTree2_p->Write("", TObject::kOverwrite);
  delete medianTree2_p;

  TDirectoryFile* paramsDir_p = (TDirectoryFile*)outFile_p->mkdir("paramsDir");
  paramsDir_p->cd();
  for(unsigned int pI = 0; pI < paramNames.size(); ++pI){
    TNamed tempName(paramNames[pI].c_str(), paramTitles[pI].c_str());
    tempName.Write("", TObject::kOverwrite);
  }
  
  outFile_p->Close();
  delete outFile_p;

  timer.stop();
  std::cout << "Total timeCPU: " << timer.totalCPU() << std::endl;
  std::cout << "Total timeWall: " << timer.totalWall() << std::endl;
  std::cout << "Job Complete!" << std::endl;  
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/createEnsembleMapHistID.exe <inFileName>. return1" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += createEnsembleMapHistID(argv[1]);
  return retVal;
}
