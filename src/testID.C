//cpp dependencies
#include <iostream>
#include <map>
#include <string>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"

int testID(const std::string inFileName)
{
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  const Int_t nEtaPhiID = 1062;
  UShort_t etaPhiID[nEtaPhiID];

  std::map<UShort_t, ULong64_t> idMap;
  
  TTree* hydTree1_p = (TTree*)inFile_p->Get("hydTree1");
  TTree* hydTree2_p = (TTree*)inFile_p->Get("hydTree2");

  hydTree1_p->SetBranchStatus("*", 0);
  hydTree1_p->SetBranchStatus("etaPhiID", 1);

  hydTree1_p->SetBranchAddress("etaPhiID", etaPhiID);

  const ULong64_t nEntries1 = hydTree1_p->GetEntries();
  for(ULong64_t entry = 0; entry < nEntries1; ++entry){
    hydTree1_p->GetEntry(entry);

    for(Int_t eI = 0; eI < nEtaPhiID; ++eI){
      ++(idMap[etaPhiID[eI]]);
    }
  }
  
  hydTree2_p->SetBranchStatus("*", 0);
  hydTree2_p->SetBranchStatus("etaPhiID", 1);

  hydTree2_p->SetBranchAddress("etaPhiID", etaPhiID);

  const ULong64_t nEntries2 = hydTree2_p->GetEntries();
  for(ULong64_t entry = 0; entry < nEntries2; ++entry){
    hydTree2_p->GetEntry(entry);

    for(Int_t eI = 0; eI < nEtaPhiID; ++eI){
      ++(idMap[etaPhiID[eI]]);
    }
  }

  ULong64_t nID = 0;
  for(auto const & val : idMap){
    ++nID;
  }

  std::cout << "nID: " << nID << std::endl;
  
  inFile_p->Close();
  delete inFile_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/testID.exe <inFileName>. return 1" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += testID(argv[1]);
  return retVal;
}
