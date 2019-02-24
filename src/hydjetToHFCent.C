//cpp dependencies
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//ROOT dependencies
#include "TDatime.h"
#include "TFile.h"
#include "TH2D.h"
#include "TMath.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"

int hydjetToHFCent(std::string inFileNames, const bool useVector = true)
{
  cppWatch timer;
  timer.start();
  
  std::vector<std::string> fileNames;
  if(inFileNames.size() == 0){
    if(inFileNames.substr(inFileNames.size()-1, 1).find(",") == std::string::npos) inFileNames = inFileNames + ",";
  }

  ULong64_t totalNEntries = 0;
  
  while(inFileNames.find(",") != std::string::npos){
    std::string fileName = inFileNames.substr(0, inFileNames.find(","));
    if(checkFile(fileName) && fileName.find(".root") != std::string::npos){
      fileNames.push_back(fileName);
      TFile* inFile_p = new TFile(fileName.c_str(), "READ");
      TTree* genTree_p = (TTree*)inFile_p->Get("genTree");

      totalNEntries += (ULong64_t)genTree_p->GetEntries();
      
      inFile_p->Close();
      delete inFile_p;
    }
    else{
      std::cout << "WARNING: \'" << fileName << "\' is not valid. exclude" << std::endl;
    }
    inFileNames.replace(0, inFileNames.find(",")+1, "");
  }

  if(fileNames.size() == 0){
    std::cout << "No valid files. return 1" << std::endl;
    return 1;
  }
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  const std::string outFileName = "output/" + dateStr + "/hydjetToHFCent_" + dateStr + ".root";
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1D* hiHFSum_p = new TH1D("hiHFSum_h", ";hiHF Sum;Counts", 200, -0.5, 9999.5);
  TH1D* hiHFBin_p = new TH1D("hiHFBin_h", ";hiHF Bin;Counts", 200, -0.5, 199.5);
  
  Int_t hiBin_;
  std::vector<float>* pt_p=NULL;
  std::vector<float>* phi_p=NULL;
  std::vector<float>* eta_p=NULL;

  const Int_t nMaxPart = 100000;
  Int_t nGen_;
  Float_t pt_[nMaxPart];
  Float_t phi_[nMaxPart];
  Float_t eta_[nMaxPart];

  std::vector<Double_t> sums;
  std::vector<Double_t> sums2;
  std::vector<Int_t> hiBins;
  sums.reserve(totalNEntries);
  sums2.reserve(totalNEntries);
  hiBins.reserve(totalNEntries);
  
  for(auto const & fileName : fileNames){
    TFile* inFile_p = new TFile(fileName.c_str(), "READ");
    TTree* genTree_p = (TTree*)inFile_p->Get("genTree");
    
    genTree_p->SetBranchStatus("*", 0);
    
    if(useVector){
      genTree_p->SetBranchStatus("hiBin", 1);
      genTree_p->SetBranchStatus("pt", 1);
      genTree_p->SetBranchStatus("phi", 1);
      genTree_p->SetBranchStatus("eta", 1);
      
      genTree_p->SetBranchAddress("hiBin", &hiBin_);
      genTree_p->SetBranchAddress("pt", &pt_p);
      genTree_p->SetBranchAddress("phi", &phi_p);
      genTree_p->SetBranchAddress("eta", &eta_p);
    }
    else{
      genTree_p->SetBranchStatus("nGen", 1);
      genTree_p->SetBranchStatus("genPt", 1);
      genTree_p->SetBranchStatus("genPhi", 1);
      genTree_p->SetBranchStatus("genEta", 1);
      
      genTree_p->SetBranchAddress("nGen", &nGen_);
      genTree_p->SetBranchAddress("genPt", pt_);
      genTree_p->SetBranchAddress("genPhi", phi_);
      genTree_p->SetBranchAddress("genEta", eta_);
    }
    
    const ULong64_t nEntries = genTree_p->GetEntries();
    const ULong64_t nDiv = TMath::Max((ULong64_t)1, (ULong64_t)(nEntries/20));
    
    std::cout << "Processing..." << std::endl;
    for(ULong64_t entry = 0; entry < nEntries; ++entry){
      if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
      genTree_p->GetEntry(entry);
      
      Double_t sum = 0.0;

      if(useVector){
	for(ULong64_t pI = 0; pI < pt_p->size(); ++pI){
	  if(TMath::Abs(eta_p->at(pI)) < 3.) continue;
	  
	  sum += pt_p->at(pI);
	}
      }
      else{
	for(Int_t pI = 0; pI < nGen_; ++pI){
	  if(TMath::Abs(eta_[pI]) < 3.) continue;
	  
	  sum += pt_[pI];
	}
      }

      hiHFSum_p->Fill(sum);
      sums.push_back(sum);
      sums2.push_back(sum);
      if(useVector) hiBins.push_back(hiBin_);
    }
    
    inFile_p->Close();
    delete inFile_p;
  }
  
  std::sort(std::begin(sums), std::end(sums));
  std::reverse(std::begin(sums), std::end(sums));
  const Int_t nHiBin = 200;
  Double_t hiBins_[nHiBin+1];

  for(Int_t hI = 0; hI < nHiBin; ++hI){
    hiBins_[hI] = sums.at((hI*sums.size())/nHiBin);
  }
  hiBins_[0] *= 2;
  hiBins_[nHiBin] = 0;

  for(Int_t hI = 0; hI < nHiBin; ++hI){
    std::cout << hI << ": " << hiBins_[hI+1] << "-" << hiBins_[hI] << std::endl;
  }

  for(auto const & sum : sums){
    for(unsigned int hI = 0; hI < nHiBin; ++hI){
      if(sum < hiBins_[hI] && sum >= hiBins_[hI+1]){
	hiHFBin_p->Fill(hI);
	break;
      }
    }
  }
  
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);

  const std::string headerFileName = "output/" + dateStr + "/hydjetHIHFToCent_" + dateStr + ".h";

  std::ofstream headerFile(headerFileName.c_str());
  headerFile << "#ifndef HYDJETHIHFTOCENT_" << dateStr << "_H" << std::endl;
  headerFile << "#define HYDJETHIHFTOCENT_" << dateStr << "_H" << std::endl;

  headerFile << std::endl;
  headerFile << "const int nHiBin = 200;" << std::endl;
  headerFile << "const double hiBins[nHiBin+1] = {";
  for(int hI = 0; hI < nHiBin; ++hI){
    headerFile << hiBins_[hI] << ", ";
  }
  headerFile << hiBins_[nHiBin] << "};" << std::endl;
  headerFile << std::endl;

  headerFile << "int hiBinFromGenHF(const double genSumHF){" << std::endl;
  headerFile << "  int pos = -1;" << std::endl;
  headerFile << "  for(int hI = 0; hI < nHiBin; ++hI){" << std::endl;
  headerFile << "    if(genSumHF >= hiBins[hI+1] && genSumHF < hiBins[hI]){" << std::endl;
  headerFile << "      pos = hI;" << std::endl;
  headerFile << "      break;" << std::endl;
  headerFile << "    }" << std::endl;
  headerFile << "  }" << std::endl;
  headerFile << std::endl;
  headerFile << "  if(pos == -1){" << std::endl;
  headerFile << "    std::cout << \"WARNING: hydjetHIHFToCent pos is -1.\" << std::endl;" << std::endl;
  headerFile << "  }" << std::endl;
  headerFile << std::endl;
  headerFile << "  return pos;" << std::endl;
  headerFile << "}" << std::endl;
  
  headerFile << std::endl;
  headerFile << "#endif" << std::endl;
  headerFile.close();
  
  outFile_p->cd();
  TH2D* hiBinRecoVsGen_p = new TH2D("hiBinRecoVsGen_p", ";RECO;GEN", 200, -0.5, 199.5, 200, -0.5, 199.5);

  for(ULong64_t eI = 0; eI < hiBins.size(); ++eI){
    Int_t hiBinGen = -1;
    for(Int_t hI = 0; hI < nHiBin; ++hI){
      if(sums2.at(eI) < hiBins_[hI] && sums2.at(eI) >= hiBins_[hI+1]){
	hiBinGen = hI;
	break;
      }
    }
    if(hiBinGen == -1){
      std::cout << "WARNING hiBinGen==-1 for sum == " << sums.at(eI) << std::endl;
    }

    hiBinRecoVsGen_p->Fill(hiBins.at(eI), hiBinGen);
  }

  outFile_p->cd();
  hiHFSum_p->Write("", TObject::kOverwrite);
  delete hiHFSum_p;
  hiHFBin_p->Write("", TObject::kOverwrite);
  delete hiHFBin_p;
  hiBinRecoVsGen_p->Write("", TObject::kOverwrite);
  delete hiBinRecoVsGen_p;
  outFile_p->Close();
  delete outFile_p;

  timer.stop();

  std::cout << "totalNEntries: " << totalNEntries << std::endl;
  std::cout << "Timing WALL: " << timer.totalWall() << std::endl;
  std::cout << "Timing CPU: " << timer.totalCPU() << std::endl;
  std::cout << "hydjetToHFCent.exe Complete!" << std::endl;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc < 2 || argc > 3){
    std::cout << "Usage: ./bin/hydjetToHFCent.exe <inFileNames> <useVector>" << std::endl;
    return 1;
  }

  int retVal = 0;
  if(argc == 2) retVal += hydjetToHFCent(argv[1]);
  else if(argc == 3) retVal += hydjetToHFCent(argv[1], std::stoi(argv[2]));
  return retVal;
}
