//cpp dependencies
#include <iostream>
#include <map>
#include <string>

//ROOT dependencies
#include "TFile.h"
#include "TMath.h"
#include "TTree.h"

int compareOldNewMapID(const std::string inFileName1, const std::string inFileName2)
{
  TFile* inFile1_p = new TFile(inFileName1.c_str(), "READ");
  TTree* medianTree1_p = (TTree*)inFile1_p->Get("medianTree1");

  const UInt_t nTower = 1062;
  UInt_t eventID1_;
  Float_t towersMedian1_[nTower];
  std::map<UInt_t, UInt_t> idToEntryMap;

  medianTree1_p->SetBranchAddress("eventID", &eventID1_);

  const UInt_t nEntries1 = medianTree1_p->GetEntries();
  for(UInt_t entry = 0; entry < nEntries1; ++entry){
    medianTree1_p->GetEntry(entry);

    idToEntryMap[eventID1_] = entry;
  }


  TFile* inFile2_p = new TFile(inFileName2.c_str(), "READ");
  TTree* medianTree2_p = (TTree*)inFile2_p->Get("medianTree1");

  UInt_t eventID2_;
  Float_t towersMedian2_[nTower];

  medianTree2_p->SetBranchAddress("eventID", &eventID2_);

  const UInt_t nEntries2 = medianTree2_p->GetEntries();
  std::cout << "NEntries: " << nEntries1 << "==" << nEntries2 << std::endl;
  for(UInt_t entry = 0; entry < nEntries2; ++entry){
    medianTree2_p->GetEntry(entry);

    if(idToEntryMap[eventID2_] != entry){
      std::cout << "WARNING: ENTRIES ID NOT SYNCED" << std::endl;
    }
  }

  medianTree1_p->SetBranchAddress("towersMedian", towersMedian1_);
  medianTree2_p->SetBranchAddress("towersMedian", towersMedian2_);

  for(UInt_t entry = 0; entry < nEntries1; ++entry){
    medianTree1_p->GetEntry(entry);
    medianTree2_p->GetEntry(entry);

    for(UInt_t tI = 0; tI < nTower; ++tI){
      if(TMath::Abs(towersMedian1_[tI] - towersMedian2_[tI]) > 0.001){
	std::cout << "WARNING ON Entry \'" << entry << "\'. Mismatch in tower: " << tI << std::endl;
      }
    }
  }
  
  inFile2_p->Close();
  delete inFile2_p;
  
  inFile1_p->Close();
  delete inFile1_p;

  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/compareOldNewMapID.exe <inFileName1> <inFileName2>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += compareOldNewMapID(argv[1], argv[2]);
  return retVal;
}
