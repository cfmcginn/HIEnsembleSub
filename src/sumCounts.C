//cpp dependencies
#include <fstream>
#include <iostream>
#include <string>

//ROOT dependencies
#include "TFile.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"

int sumCounts(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".txt") == std::string::npos){
    std::cout << "inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  std::ifstream inFile(inFileName.c_str());
  std::string tempStr;

  ULong64_t total = 0;
  
  while(std::getline(inFile, tempStr)){
    if(tempStr.size() == 0) continue;
    if(!checkFile(tempStr)) continue;
    if(tempStr.find(".root") == std::string::npos) continue;
    
    TFile* inFile_p = new TFile(tempStr.c_str(), "READ");    
    TTree* genTree_p = (TTree*)inFile_p->Get("genTree");

    total += genTree_p->GetEntries();   
    
    inFile_p->Close();
    delete inFile_p;
  }

  inFile.close();

  std::cout << total << std::endl;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/sumCounts.exe <inFileName>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += sumCounts(argv[1]);
  return retVal;  
}
