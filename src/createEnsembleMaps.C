//cpp dependencies
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT dependencies
#include "TDatime.h"
#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/getLinBins.h"
#include "include/hydjetHIHFToCent_20190118.h"
#include "include/plotUtilities.h"
#include "include/pseudoTowerGeometry.h"

int createEnsembleMaps(const std::string inFileNameHYD, std::string inFileNamesPYT, std::string inJetPtMins)
{
  cppWatch timer;
  timer.start();

  if(!checkFile(inFileNameHYD) || inFileNameHYD.find(".root") == std::string::npos){
    std::cout << "Given inFileNameHYD \'" << inFileNameHYD << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  std::vector<std::string> inFileNamePYTVect;
  if(inFileNamesPYT.size() != 0){
    if(inFileNamesPYT.substr(inFileNamesPYT.size()-1, 1).find(",") == std::string::npos) inFileNamesPYT = inFileNamesPYT + ",";
    while(inFileNamesPYT.find(",") != std::string::npos){
      inFileNamePYTVect.push_back(inFileNamesPYT.substr(0, inFileNamesPYT.find(",")));
      inFileNamesPYT.replace(0, inFileNamesPYT.find(",")+1, "");
    }
  }
  std::vector<float> inJetPtMinVect;
  if(inJetPtMins.size() != 0){
    if(inJetPtMins.substr(inJetPtMins.size()-1, 1).find(",") == std::string::npos) inJetPtMins = inJetPtMins + ",";
    while(inJetPtMins.find(",") != std::string::npos){
      inJetPtMinVect.push_back(std::stof(inJetPtMins.substr(0, inJetPtMins.find(","))));
      inJetPtMins.replace(0, inJetPtMins.find(",")+1, "");
    }
  }
 
  if(inFileNamePYTVect.size() != inJetPtMinVect.size()){
    std::cout << "Sizes inFileNamePYTVect not equal inJtPtMinVect. return 1" << std::endl;
    return 1;
  }

  for(auto const & name : inFileNamePYTVect){
    if(!checkFile(name)){
      std::cout << "inFileNamePYT \'" << name << "\' is not valid. return 1" << std::endl;
      return 1;
    }
  }
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  TRandom3* randGen_p = new TRandom3(269); //Seed from random coin flips 100,001,101 -> 269

  const Int_t nPYT = inFileNamePYTVect.size();
  TFile* inFilePYT_p[nPYT];
  TTree* genPartTreePYT_p[nPYT];
  
  std::vector<std::vector<ULong64_t> > goodEntries;
  
  std::vector<float>* genJtPt_p = new std::vector<float>;
  std::vector<float>* genJtEta_p = new std::vector<float>;

  std::vector<std::vector<float>* > ptPYT_p;
  std::vector<std::vector<float>* > phiPYT_p;
  std::vector<std::vector<float>* > etaPYT_p;
  
  for(unsigned int i = 0; i < inFileNamePYTVect.size(); ++i){
    inFilePYT_p[i] = new TFile(inFileNamePYTVect[i].c_str(), "READ");
    TTree* genJtTreePYT_p = (TTree*)inFilePYT_p[i]->Get("ak4GenJetTree_ESchemeWTA");

    genJtTreePYT_p->SetBranchStatus("*", 0);
    genJtTreePYT_p->SetBranchStatus("genJtPt", 1);
    genJtTreePYT_p->SetBranchStatus("genJtEta", 1);

    genJtTreePYT_p->SetBranchAddress("genJtPt", &genJtPt_p);
    genJtTreePYT_p->SetBranchAddress("genJtEta", &genJtEta_p);

    goodEntries.push_back({});

    ULong64_t nEntries = genJtTreePYT_p->GetEntries();
    
    for(ULong64_t entry = 0; entry < nEntries; ++entry){
      genJtTreePYT_p->GetEntry(entry);

      for(unsigned int jI = 0; jI < genJtPt_p->size(); ++jI){
	if(TMath::Abs((*genJtEta_p)[jI]) > 2.) continue;
	if((*genJtPt_p)[jI] < inJetPtMinVect[i]) continue;

	goodEntries[i].push_back(entry);
      }
    }

    inFilePYT_p[i]->Close();
    delete inFilePYT_p[i];

    ptPYT_p.push_back(new std::vector<float>);
    phiPYT_p.push_back(new std::vector<float>);
    etaPYT_p.push_back(new std::vector<float>);
  }

  genJtPt_p->clear();
  genJtEta_p->clear();

  delete genJtPt_p;
  delete genJtEta_p;


  for(unsigned int i = 0; i < inFileNamePYTVect.size(); ++i){
    inFilePYT_p[i] = new TFile(inFileNamePYTVect[i].c_str(), "READ");
    genPartTreePYT_p[i] = (TTree*)inFilePYT_p[i]->Get("genParticleTree");
    
    genPartTreePYT_p[i]->SetBranchStatus("*", 0);
    genPartTreePYT_p[i]->SetBranchStatus("pt", 1);
    genPartTreePYT_p[i]->SetBranchStatus("phi", 1);
    genPartTreePYT_p[i]->SetBranchStatus("eta", 1);

    genPartTreePYT_p[i]->SetBranchAddress("pt", &(ptPYT_p[i]));
    genPartTreePYT_p[i]->SetBranchAddress("phi", &(phiPYT_p[i]));
    genPartTreePYT_p[i]->SetBranchAddress("eta", &(etaPYT_p[i]));
  }
  
  
  TFile* inFileHYD_p = new TFile(inFileNameHYD.c_str(), "READ");
  TTree* genTreeHYD_p = (TTree*)inFileHYD_p->Get("genTree");

  std::vector<float>* ptHYD_p = new std::vector<float>;
  std::vector<float>* phiHYD_p = new std::vector<float>;
  std::vector<float>* etaHYD_p = new std::vector<float>;

  genTreeHYD_p->SetBranchStatus("*", 0);
  genTreeHYD_p->SetBranchStatus("pt", 1);
  genTreeHYD_p->SetBranchStatus("phi", 1);
  genTreeHYD_p->SetBranchStatus("eta", 1);

  genTreeHYD_p->SetBranchAddress("pt", &ptHYD_p);
  genTreeHYD_p->SetBranchAddress("phi", &phiHYD_p);
  genTreeHYD_p->SetBranchAddress("eta", &etaHYD_p);

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  std::string outFileName = "output/" + dateStr + "/ensembleMaps_" + dateStr + ".root";
  
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* hydTree1_p = new TTree("hydTree1", "");
  TTree* hydTree2_p = new TTree("hydTree2", "");

  UInt_t origEntry_, eventID_;
  UChar_t genHIBin_, genHIPos_, v2Pos_, v3Pos_;
  Float_t v2Phi_, v3Phi_;
  
  pseudoTowGeo towers;
  std::vector<double> etaTowBounds = towers.getEtaTowBoundsHalf();
  const Int_t nEtaTowBounds = etaTowBounds.size()-1;
  std::vector<int> nPhiInTower = towers.getNTowInPhiHalf();

  const Int_t maxNHiBins_ = 200;
  const Int_t nHiBins_ = 50;
  const Int_t hiBinDiv_ = maxNHiBins_/nHiBins_;
  const Int_t nV2Bins_ = 9; //corresponds to tower granularity
  const Int_t nV3Bins_ = 6; //corresponds to tower granularity

  Int_t hiBins_[nHiBins_+1];
  getLinBins(0, 200, nHiBins_, hiBins_);

  Double_t v2Bins_[nV2Bins_+1];
  getLinBins(-TMath::Pi()/2., TMath::Pi()/2., nV2Bins_, v2Bins_);
  std::vector<Double_t> v2Bins2_;
  v2Bins2_.reserve(nV2Bins_+1);
  for(Int_t v2I = 0; v2I < nV2Bins_ + 1; ++v2I){
    v2Bins2_.push_back(v2Bins_[v2I]);
  }
  
  Double_t v3Bins_[nV3Bins_+1];
  getLinBins(-TMath::Pi()/3., TMath::Pi()/3., nV3Bins_, v3Bins_);
  std::vector<Double_t> v3Bins2_;
  v3Bins2_.reserve(nV3Bins_+1);
  for(Int_t v3I = 0; v3I < nV3Bins_ + 1; ++v3I){
    v3Bins2_.push_back(v3Bins_[v3I]);
  }

  const Int_t nNTowers = 4;
  const Int_t nTowers[nNTowers] = {9, 18, 36, 72};
  std::map<Int_t, std::vector<Double_t> > phiTowBounds;

  for(Int_t tI = 0; tI < nNTowers; ++tI){
    std::vector<Double_t> tempPhi;
    tempPhi.reserve(nTowers[tI]+1);

    Double_t tempPhi2[nTowers[tI]+1];
    getLinBins(-TMath::Pi(), TMath::Pi(), nTowers[tI], tempPhi2);
    tempPhi2[0] -= 0.001;
    tempPhi2[nTowers[tI]] += 0.001;    
    
    for(Int_t tI2 = 0; tI2 < nTowers[tI]+1; ++tI2){
      tempPhi.push_back(tempPhi2[tI2]);
    }
    phiTowBounds[nTowers[tI]] = tempPhi;
  }    

  UShort_t totalTow = 0;
  std::map<UShort_t, Float_t> etaPhiMapToSum;
  std::map<UShort_t, UShort_t> etaPhiMapToArrayPos;
  std::vector<std::map<UShort_t, Float_t> > etaPhiMapToSumJets;
  
  for(Int_t eI = 0; eI < nEtaTowBounds; ++eI){
    for(int pI = 0; pI < nPhiInTower[eI]; ++pI){
      UShort_t id = eI*100 + pI;
      etaPhiMapToSum[id] = 0.0;
      etaPhiMapToArrayPos[id] = totalTow;
      ++totalTow;
    }
  }

  for(Int_t pI = 0; pI < nPYT; ++pI){etaPhiMapToSumJets.push_back(etaPhiMapToSum);}
  
  const UShort_t nTotalTow = totalTow;
  Float_t etaPhiSum_[nTotalTow];
  Float_t etaPhiSumJet_[nPYT][nTotalTow];

  hydTree1_p->Branch("origEntry", &origEntry_, "origEntry/i");
  hydTree1_p->Branch("genHIBin", &genHIBin_, "genHIBin/b");
  hydTree1_p->Branch("genHIPos", &genHIPos_, "genHIPos/b");
  hydTree1_p->Branch("v2Phi", &v2Phi_, "v2Phi/F");
  hydTree1_p->Branch("v2Pos", &v2Pos_, "v2Pos/b");
  hydTree1_p->Branch("v3Phi", &v3Phi_, "v3Phi/F");
  hydTree1_p->Branch("v3Pos", &v3Pos_, "v3Pos/b");
  hydTree1_p->Branch("eventID", &eventID_, "eventID/i");
  hydTree1_p->Branch("etaPhiSum", etaPhiSum_, ("etaPhiSum[" + std::to_string(nTotalTow) + "]/F").c_str());
  for(int pI = 0; pI < nPYT; ++pI){
    hydTree1_p->Branch(("etaPhiSum_JetPtMin" + prettyString(inJetPtMinVect[pI], 1, true)).c_str(), etaPhiSumJet_[pI], ("etaPhiSum_JetPtMin" + prettyString(inJetPtMinVect[pI], 1, true) + "[" + std::to_string(nTotalTow) + "]/F").c_str());    
  }
  
  hydTree2_p->Branch("origEntry", &origEntry_, "origEntry/i");
  hydTree2_p->Branch("genHIBin", &genHIBin_, "genHIBin/b");
  hydTree2_p->Branch("genHIPos", &genHIPos_, "genHIPos/b");
  hydTree2_p->Branch("v2Phi", &v2Phi_, "v2Phi/F");
  hydTree2_p->Branch("v2Pos", &v2Pos_, "v2Pos/b");
  hydTree2_p->Branch("v3Phi", &v3Phi_, "v3Phi/F");
  hydTree2_p->Branch("v3Pos", &v3Pos_, "v3Pos/b");
  hydTree2_p->Branch("eventID", &eventID_, "eventID/i");
  hydTree2_p->Branch("etaPhiSum", etaPhiSum_, ("etaPhiSum[" + std::to_string(nTotalTow) + "]/F").c_str());
  for(int pI = 0; pI < nPYT; ++pI){
    hydTree2_p->Branch(("etaPhiSum_JetPtMin" + prettyString(inJetPtMinVect[pI], 1, true)).c_str(), etaPhiSumJet_[pI], ("etaPhiSum_JetPtMin" + prettyString(inJetPtMinVect[pI], 1, true) + "[" + std::to_string(nTotalTow) + "]/F").c_str());    
  }

  const ULong64_t nEntries = TMath::Min((ULong64_t)100000000, (ULong64_t)genTreeHYD_p->GetEntries());
  const ULong64_t div = TMath::Max((ULong64_t)1, nEntries/20);
  
  std::cout << "Processing..." << std::endl;
  for(ULong64_t entry = 0; entry < nEntries; ++entry){
    if(entry%div == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    genTreeHYD_p->GetEntry(entry);

    Bool_t fillTree1 = (randGen_p->Uniform(0,1) < 0.5);
    Double_t genHIHF_ = 0.0;

    for(auto const & val : etaPhiMapToSum){
      etaPhiMapToSum[val.first] = 0.0;

      for(Int_t pI = 0; pI < nPYT; ++pI){
	etaPhiMapToSumJets[pI][val.first] = 0.0;     
      }
    }

    Double_t v2SumX = 0.0;
    Double_t v2SumY = 0.0;

    Double_t v3SumX = 0.0;
    Double_t v3SumY = 0.0;

    for(unsigned int pI = 0; pI < ptHYD_p->size(); ++pI){
      if((*ptHYD_p)[pI] < 0.5) continue;
      if(TMath::Abs((*etaHYD_p)[pI]) > 5.0) continue;
      if(TMath::Abs((*etaHYD_p)[pI]) > 3.0) genHIHF_ += (*ptHYD_p)[pI];      
      
      if((*ptHYD_p)[pI] >= 1. && (*ptHYD_p)[pI] < 3.){
	if(TMath::Abs((*etaHYD_p)[pI]) < 1.){
	  v2SumX += TMath::Cos(2*((*phiHYD_p)[pI]));
	  v2SumY += TMath::Sin(2*((*phiHYD_p)[pI]));

	  v3SumX += TMath::Cos(3*((*phiHYD_p)[pI]));
	  v3SumY += TMath::Sin(3*((*phiHYD_p)[pI]));
	}
      }      
      
      unsigned int etaPos = ((unsigned int)(std::lower_bound(etaTowBounds.begin(), etaTowBounds.end(), (*etaHYD_p)[pI]) - etaTowBounds.begin())) - 1;
      int nPhiTemp = nPhiInTower[etaPos];
      std::vector<Double_t> phiBoundTemp = phiTowBounds[nPhiTemp];
      unsigned int phiPos = ((unsigned int)(std::lower_bound(phiBoundTemp.begin(), phiBoundTemp.end(), (*phiHYD_p)[pI]) - phiBoundTemp.begin())) - 1;
      
      ULong64_t id = etaPos*100 + phiPos;
      etaPhiMapToSum[id] += (*ptHYD_p)[pI];

      for(Int_t pI = 0; pI < nPYT; ++pI){
	etaPhiMapToSumJets[pI][id] += (*ptHYD_p)[pI];
      }
    }
  
    for(Int_t pI = 0; pI < nPYT; ++pI){
      genPartTreePYT_p[pI]->GetEntry(goodEntries[pI][entry]%goodEntries[pI].size());

      for(unsigned int pJ = 0; pJ < ptPYT_p[pI]->size(); ++pJ){
	if((*(ptPYT_p[pI]))[pJ] < 0.5) continue;
	if(TMath::Abs((*(etaPYT_p[pI]))[pJ]) > 5.0) continue;

	unsigned int etaPos = ((unsigned int)(std::lower_bound(etaTowBounds.begin(), etaTowBounds.end(), (*(etaPYT_p[pI]))[pJ]) - etaTowBounds.begin())) - 1;
	int nPhiTemp = nPhiInTower[etaPos];
	std::vector<Double_t> phiBoundTemp = phiTowBounds[nPhiTemp];
	unsigned int phiPos = ((unsigned int)(std::lower_bound(phiBoundTemp.begin(), phiBoundTemp.end(), (*(phiPYT_p[pI]))[pJ]) - phiBoundTemp.begin())) - 1;
	
	ULong64_t id = etaPos*100 + phiPos;

	etaPhiMapToSumJets[pI][id] += (*(ptPYT_p[pI]))[pJ];
      }
    }

    origEntry_ = (UInt_t)entry;

    genHIBin_ = hiBinFromGenHF(genHIHF_);
    if(genHIBin_ >= 160) continue;

    genHIPos_ = genHIBin_/hiBinDiv_;

    v2Phi_ = TMath::ATan2(v2SumY, v2SumX);
    if(v2Phi_ < -TMath::Pi()/2.) v2Phi_ += TMath::Pi();
    else if(v2Phi_ > TMath::Pi()/2.) v2Phi_ -= TMath::Pi();

    v2Pos_ = ((UChar_t)(std::lower_bound(v2Bins2_.begin(), v2Bins2_.end(), v2Phi_) - v2Bins2_.begin())) - 1;
    
    v3Phi_ = TMath::ATan2(v3SumY, v3SumX);
    if(v3Phi_ < -TMath::Pi()/3.) v3Phi_ += TMath::Pi()*2./3.;
    else if(v3Phi_ > TMath::Pi()/3.) v3Phi_ -= TMath::Pi()*2./3.;

    v3Pos_ = ((UChar_t)(std::lower_bound(v3Bins2_.begin(), v3Bins2_.end(), v3Phi_) - v3Bins2_.begin())) - 1;

    eventID_ = ((UInt_t)genHIPos_)*10000 + ((UInt_t)v3Pos_)*100 + ((UInt_t)v2Pos_);    

    for(auto const & val : etaPhiMapToSum){
      etaPhiSum_[etaPhiMapToArrayPos[val.first]] = val.second;
     
      for(Int_t pI = 0; pI < nPYT; ++pI){
	etaPhiSumJet_[pI][etaPhiMapToArrayPos[val.first]] = etaPhiMapToSumJets[pI][val.first];
      }
    }
    
    if(fillTree1) hydTree1_p->Fill();
    else hydTree2_p->Fill();
  }

  outFile_p->cd();

  hydTree1_p->Write("", TObject::kOverwrite);
  hydTree2_p->Write("", TObject::kOverwrite);

  delete hydTree1_p;
  delete hydTree2_p;

  std::string etaBinsStr = "";
  for(unsigned int eI = 0; eI < etaTowBounds.size(); ++eI){
    etaBinsStr = etaBinsStr + prettyString(etaTowBounds[eI], 3, false) + ",";
  }

  std::string nPhiBinsStr = "";
  for(int eI = 0; eI < nEtaTowBounds; ++eI){
    nPhiBinsStr = nPhiBinsStr + std::to_string(nPhiInTower[eI]) + ",";
  }

  std::string hiBinsStr = "";
  for(Int_t cI = 0; cI < nHiBins_+1; ++cI){
    hiBinsStr = hiBinsStr + std::to_string(hiBins_[cI]) + ",";
  }

  std::string v2BinsStr = "";
  for(Int_t vI = 0; vI < nV2Bins_+1; ++vI){
    v2BinsStr = v2BinsStr + std::to_string(v2Bins_[vI]) + ",";
  }

  std::string v3BinsStr = "";
  for(Int_t vI = 0; vI < nV3Bins_+1; ++vI){
    v3BinsStr = v3BinsStr + std::to_string(v3Bins_[vI]) + ",";
  }

  TDirectoryFile* paramsDir_p = (TDirectoryFile*)outFile_p->mkdir("paramsDir");
  paramsDir_p->cd();

  TNamed nHiBinsName("nHiBins", std::to_string(nHiBins_));
  TNamed hiBinsName("hiBins", hiBinsStr.c_str());

  TNamed nV2BinsName("nV2Bins", std::to_string(nV2Bins_));
  TNamed v2BinsName("v2Bins", v2BinsStr.c_str());

  TNamed nV3BinsName("nV3Bins", std::to_string(nV3Bins_));
  TNamed v3BinsName("v3Bins", v3BinsStr.c_str());

  TNamed nEtaBinsName("nEtaBins", std::to_string(((Int_t)nEtaTowBounds)));
  TNamed etaBinsName("etaBins", etaBinsStr.c_str());
  TNamed nPhiBinsName("nPhiBins", nPhiBinsStr.c_str());

  nHiBinsName.Write("", TObject::kOverwrite);
  hiBinsName.Write("", TObject::kOverwrite);

  nV2BinsName.Write("", TObject::kOverwrite);
  v2BinsName.Write("", TObject::kOverwrite);

  nV3BinsName.Write("", TObject::kOverwrite);
  v3BinsName.Write("", TObject::kOverwrite);

  nEtaBinsName.Write("", TObject::kOverwrite);
  etaBinsName.Write("", TObject::kOverwrite);
  nPhiBinsName.Write("", TObject::kOverwrite);  
  
  outFile_p->Close();
  delete outFile_p;

  ptHYD_p->clear();
  etaHYD_p->clear();
  phiHYD_p->clear();

  delete ptHYD_p;
  delete etaHYD_p;
  delete phiHYD_p;

  inFileHYD_p->Close();
  delete inFileHYD_p;

  for(int i = 0; i < nPYT; ++i){
    ptPYT_p[i]->clear();
    phiPYT_p[i]->clear();
    etaPYT_p[i]->clear();

    delete ptPYT_p[i];
    delete phiPYT_p[i];
    delete etaPYT_p[i];
    
    inFilePYT_p[i]->Close();
    delete inFilePYT_p[i];
  }
  
  ptPYT_p.clear();
  etaPYT_p.clear();
  phiPYT_p.clear();
  
  delete randGen_p;

  timer.stop();

  std::cout << "Total timing CPU: " <<   timer.totalCPU() << std::endl;
  std::cout << "Total timing Wall: " <<   timer.totalWall() << std::endl;
  std::cout << "Job Complete" << std::endl;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "Usage: ./bin/createEnsembleMaps.exe <inFileNameHYD> <inFileNamesPYT> <inJetPtMins>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += createEnsembleMaps(argv[1], argv[2], argv[3]);
  return retVal;
}
