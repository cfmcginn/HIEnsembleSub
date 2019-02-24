//cpp dependencies
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT dependencies
#include "TBranch.h"
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TNamed.h"
#include "TTree.h"

//FASTJET dependencies
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/etaPhiFunc.h"
#include "include/getLinBins.h"
#include "include/hydjetHIHFToCent_20190118.h"
#include "include/plotUtilities.h"
#include "include/pseudoTowerGeometry.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

void getBinsFromString(std::string inStr, UInt_t nBins, UInt_t bins[])
{
  if(inStr.size() == 0) return;
  if(inStr.substr(inStr.size()-1, 1).find(",") == std::string::npos) inStr = inStr + ",";
  while(inStr.find(",,") != std::string::npos){inStr.replace(inStr.find(",,"), 2, ",");}

  unsigned int pos = 0;
  while(inStr.find(",") != std::string::npos){  
    if(nBins+1 < pos) std::cout << "WARNING: bins have exceeded array size in getBinsFromString" << std::endl;
    bins[pos] = (UInt_t)std::stoi(inStr.substr(0, inStr.find(",")));
    inStr.replace(0, inStr.find(",")+1, "");
    ++pos;
  }

  return;
}

void getBinsFromString(std::string inStr, UInt_t nBins, Float_t bins[])
{
  if(inStr.size() == 0) return;
  if(inStr.substr(inStr.size()-1, 1).find(",") == std::string::npos) inStr = inStr + ",";
  while(inStr.find(",,") != std::string::npos){inStr.replace(inStr.find(",,"), 2, ",");}

  unsigned int pos = 0;
  while(inStr.find(",") != std::string::npos){  
    if(nBins+1 < pos) std::cout << "WARNING: bins have exceeded array size in getBinsFromString" << std::endl;
    bins[pos] = std::stof(inStr.substr(0, inStr.find(",")));
    inStr.replace(0, inStr.find(",")+1, "");
    ++pos;
  }

  return;
}


void dispBins(std::string binStr, UInt_t nBins, UInt_t bins[])
{
  std::cout << binStr << ": " << nBins << std::endl;
  for(unsigned int i = 0; i < nBins; ++i){
    std::cout << " " << bins[i] << ",";
  }
  std::cout << " " << bins[nBins] << "." << std::endl;
  return;
}


void dispBins(std::string binStr, UInt_t nBins, Float_t bins[])
{
  std::cout << binStr << ": " << nBins << std::endl;
  for(unsigned int i = 0; i < nBins; ++i){
    std::cout << " " << bins[i] << ",";
  }
  std::cout << " " << bins[nBins] << "." << std::endl;
  return;
}


int testEnsembleMaps(const std::string inEnsembleFileName, const std::string inHYDFileName, std::string inPYTFileNames, std::string inJtPtMins, std::string inJtPtMaxs)
{
  cppWatch timer;
  timer.start();

  if(!checkFile(inEnsembleFileName)){
    std::cout << "inEnsembleFileName \'" << inEnsembleFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }
  else if(inEnsembleFileName.find(".root") == std::string::npos){
    std::cout << "inEnsembleFileName \'" << inEnsembleFileName << "\' is not .root. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(inHYDFileName)){
    std::cout << "inHYDFileName \'" << inHYDFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }
  else if(inHYDFileName.find(".root") == std::string::npos){
    std::cout << "inHYDFileName \'" << inHYDFileName << "\' is not .root. return 1" << std::endl;
    return 1;
  }

  std::vector<std::string> pytFiles;
  std::vector<double> jtPtMins;
  std::vector<double> jtPtMaxs;
  if(inPYTFileNames.size() != 0){
    if(inPYTFileNames.substr(inPYTFileNames.size()-1, 1).find(",") == std::string::npos) inPYTFileNames = inPYTFileNames + ",";

    while(inPYTFileNames.find(",") != std::string::npos){
      pytFiles.push_back(inPYTFileNames.substr(0, inPYTFileNames.find(",")));
      inPYTFileNames.replace(0, inPYTFileNames.find(",")+1, "");
    }
  }
  else{
    std::cout << "inPYTFileNames \'" << inPYTFileNames << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  if(inJtPtMins.size() != 0){
    if(inJtPtMins.substr(inJtPtMins.size()-1, 1).find(",") == std::string::npos) inJtPtMins = inJtPtMins + ",";

    while(inJtPtMins.find(",") != std::string::npos){
      jtPtMins.push_back(std::stod(inJtPtMins.substr(0, inJtPtMins.find(","))));
      inJtPtMins.replace(0, inJtPtMins.find(",")+1, "");
    }
  }
  else{
    std::cout << "inJtPtMins \'" << inJtPtMins << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  if(inJtPtMaxs.size() != 0){
    if(inJtPtMaxs.substr(inJtPtMaxs.size()-1, 1).find(",") == std::string::npos) inJtPtMaxs = inJtPtMaxs + ",";

    while(inJtPtMaxs.find(",") != std::string::npos){
      jtPtMaxs.push_back(std::stod(inJtPtMaxs.substr(0, inJtPtMaxs.find(","))));
      inJtPtMaxs.replace(0, inJtPtMaxs.find(",")+1, "");
    }
  }
  else{
    std::cout << "inJtPtMaxs \'" << inJtPtMaxs << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  const UInt_t nMaxPYTFiles = 5;
  const UInt_t nPYTFiles = pytFiles.size();
  if(nPYTFiles > nMaxPYTFiles){
    std::cout << "Given nPYTFiles \'" << nPYTFiles << "\' is greater nMaxPYTFiles \'" << nMaxPYTFiles << "\'." << std::endl;
  }

  
  if(nPYTFiles != jtPtMins.size()){
    std::cout << "inPYTFileNames does not equal number of jet pt min. return 1" << std::endl;
    return 1;
  }
  else if(jtPtMaxs.size() != jtPtMins.size()){
    std::cout << "jet pt maxes does not equal number of jet pt mins. return 1" << std::endl;
    return 1;
  }

  for(auto const& file : pytFiles){
    if(!checkFile(file)){
      std::cout << "inPYTFile \'" << file << "\' is invalid. return 1" << std::endl;
      return 1;
    }
    else if(file.find(".root") == std::string::npos){
      std::cout << "inPYTFile \'" << file << "\' is not root. return 1" << std::endl;
      return 1;
    }
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  std::string outFileName = "output/" + dateStr + "/testEnsembleMaps_" + dateStr + ".root";

  pseudoTowGeo towGeo;
  std::vector<double> etaTowBounds = towGeo.getEtaTowBounds();
  std::vector<int> nTowInPhi = towGeo.getNTowInPhi();
  const UInt_t nSetsOfNTowInPhi = 4;
  const UInt_t setsOfNTowInPhi[nSetsOfNTowInPhi] = {72, 36, 18, 9};
  std::map<UInt_t, std::vector<Float_t> > setsOfTowInPhi;
  for(UInt_t i = 0; i < nSetsOfNTowInPhi; ++i){
    setsOfTowInPhi[setsOfNTowInPhi[i]] = {};

    Float_t tempBins[setsOfNTowInPhi[i]+1];
    getLinBins(-TMath::Pi(), TMath::Pi(), setsOfNTowInPhi[i], tempBins);
    tempBins[0] -= 0.001;
    tempBins[setsOfNTowInPhi[i]] += 0.001;

    for(UInt_t j = 0; j < setsOfNTowInPhi[i]+1; ++j){
      setsOfTowInPhi[setsOfNTowInPhi[i]].push_back(tempBins[j]);
    }
  }

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* globalTree_p = new TTree("globalTree", "");
  TTree* jetTree_p[nMaxPYTFiles];

  UChar_t genHIBinHYD_, genHIPosHYD_, genV2PosHYD_, genV3PosHYD_;
  Float_t genV2PhiHYD_, genV3PhiHYD_;
  UInt_t genEventIDHYD_;
  
  globalTree_p->Branch("genHIBinHYD", &genHIBinHYD_, "genHIBinHYD/b");
  globalTree_p->Branch("genHIPosHYD", &genHIPosHYD_, "genHIPosHYD/b");
  globalTree_p->Branch("genV2PhiHYD", &genV2PhiHYD_, "genV2PhiHYD/F");
  globalTree_p->Branch("genV2PosHYD", &genV2PosHYD_, "genV2PosHYD/b");
  globalTree_p->Branch("genV3PhiHYD", &genV3PhiHYD_, "genV3PhiHYD/F");
  globalTree_p->Branch("genV3PosHYD", &genV3PosHYD_, "genV3PosHYD/b");
  globalTree_p->Branch("genEventIDHYD", &genEventIDHYD_, "genEventIDHYD/i");

  /*
  TBranch* genHIBinHYD_b = (TBranch*)globalTree_p->Branch("genHIBinHYD", &genHIBinHYD_, "genHIBinHYD/b");
  TBranch* genHIPosHYD_b = (TBranch*)globalTree_p->Branch("genHIPosHYD", &genHIPosHYD_, "genHIPosHYD/b");
  TBranch* genV2PhiHYD_b = (TBranch*)globalTree_p->Branch("genV2PhiHYD", &genV2PhiHYD_, "genV2PhiHYD/F");
  TBranch* genV2PosHYD_b = (TBranch*)globalTree_p->Branch("genV2PosHYD", &genV2PosHYD_, "genV2PosHYD/b");
  TBranch* genV3PhiHYD_b = (TBranch*)globalTree_p->Branch("genV3PhiHYD", &genV3PhiHYD_, "genV3PhiHYD/F");
  TBranch* genV3PosHYD_b = (TBranch*)globalTree_p->Branch("genV3PosHYD", &genV3PosHYD_, "genV3PosHYD/b");
  TBranch* genEventIDHYD_b = (TBranch*)globalTree_p->Branch("genEventIDHYD", &genEventIDHYD_, "genEventIDHYD/i");
  */
  
  UChar_t genHIBinPYT_[nMaxPYTFiles];
  UChar_t genHIPosPYT_[nMaxPYTFiles];
  Float_t genV2PhiPYT_[nMaxPYTFiles];
  UChar_t genV2PosPYT_[nMaxPYTFiles];
  Float_t genV3PhiPYT_[nMaxPYTFiles];
  UChar_t genV3PosPYT_[nMaxPYTFiles];
  UInt_t genEventIDPYT_[nMaxPYTFiles];
  const UShort_t nMaxJets = 500;
  UInt_t nGenJt_[nMaxPYTFiles];
  Float_t genJtPt_[nMaxPYTFiles][nMaxJets];
  Float_t genJtPhi_[nMaxPYTFiles][nMaxJets];
  Float_t genJtEta_[nMaxPYTFiles][nMaxJets];

  Float_t inEvtPUJtPt_[nMaxPYTFiles][nMaxJets];
  Float_t inEvtPUJtPhi_[nMaxPYTFiles][nMaxJets];
  Float_t inEvtPUJtEta_[nMaxPYTFiles][nMaxJets];

  Float_t outEvtPUJtPt_[nMaxPYTFiles][nMaxJets];
  Float_t outEvtPUJtPhi_[nMaxPYTFiles][nMaxJets];
  Float_t outEvtPUJtEta_[nMaxPYTFiles][nMaxJets];

  for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
    jetTree_p[pI] = new TTree(("jetTree_MinPt" + prettyString(jtPtMins[pI],1,true) + "toMaxPt" + prettyString(jtPtMaxs[pI],1,true)).c_str(), "");

    jetTree_p[pI]->Branch("genHIBinPYT", &(genHIBinPYT_[pI]), "genHIBinPYT/b");
    jetTree_p[pI]->Branch("genHIPosPYT", &(genHIPosPYT_[pI]), "genHIPosPYT/b");
    jetTree_p[pI]->Branch("genV2PhiPYT", &(genV2PhiPYT_[pI]), "genV2PhiPYT/F");
    jetTree_p[pI]->Branch("genV2PosPYT", &(genV2PosPYT_[pI]), "genV2PosPYT/b");
    jetTree_p[pI]->Branch("genV3PhiPYT", &(genV3PhiPYT_[pI]), "genV3PhiPYT/F");
    jetTree_p[pI]->Branch("genV3PosPYT", &(genV3PosPYT_[pI]), "genV3PosPYT/b");
    jetTree_p[pI]->Branch("genEventIDPYT", &(genEventIDPYT_[pI]), "genEventIDPYT/i");

    jetTree_p[pI]->Branch("nGenJt", &(nGenJt_[pI]), "nGenJt/i");
    jetTree_p[pI]->Branch("genJtPt", genJtPt_[pI], "genJtPt[nGenJt]/F");
    jetTree_p[pI]->Branch("genJtPhi", genJtPhi_[pI], "genJtPhi[nGenJt]/F");
    jetTree_p[pI]->Branch("genJtEta", genJtEta_[pI], "genJtEta[nGenJt]/F");

    jetTree_p[pI]->Branch("inEvtPUJtPt", inEvtPUJtPt_[pI], "inEvtPUJtPt[nGenJt]/F");
    jetTree_p[pI]->Branch("inEvtPUJtPhi", inEvtPUJtPhi_[pI], "inEvtPUJtPhi[nGenJt]/F");
    jetTree_p[pI]->Branch("inEvtPUJtEta", inEvtPUJtEta_[pI], "inEvtPUJtEta[nGenJt]/F");

    jetTree_p[pI]->Branch("outEvtPUJtPt", outEvtPUJtPt_[pI], "outEvtPUJtPt[nGenJt]/F");
    jetTree_p[pI]->Branch("outEvtPUJtPhi", outEvtPUJtPhi_[pI], "outEvtPUJtPhi[nGenJt]/F");
    jetTree_p[pI]->Branch("outEvtPUJtEta", outEvtPUJtEta_[pI], "outEvtPUJtEta[nGenJt]/F");

    nGenJt_[pI] = 0;
  }
  
  TFile* inEnsFile_p = new TFile(inEnsembleFileName.c_str(), "READ");
  std::vector<std::string> paramNames = returnRootFileContentsList(inEnsFile_p, "TNamed", "");
  inEnsFile_p->Close();
  delete inEnsFile_p;
  inEnsFile_p = new TFile(inEnsembleFileName.c_str(), "READ");
  std::vector<std::string> paramTitles;
  paramTitles.reserve(paramNames.size());
  UInt_t nHiBinsTemp = 0;
  UInt_t nV2BinsTemp = 0;
  UInt_t nV3BinsTemp = 0;
  UInt_t nEtaBinsTemp = 0;

  for(auto const & name : paramNames){
    TNamed* tempName = (TNamed*)inEnsFile_p->Get(name.c_str());
    paramTitles.push_back(tempName->GetTitle());
    delete tempName;
    
    if(isStrSame(name, "paramsDir/nHiBins")) nHiBinsTemp = (UInt_t)std::stoi(paramTitles[paramTitles.size()-1]);
    else if(isStrSame(name, "paramsDir/nV2Bins")) nV2BinsTemp = (UInt_t)std::stoi(paramTitles[paramTitles.size()-1]);
    else if(isStrSame(name, "paramsDir/nV3Bins")) nV3BinsTemp = (UInt_t)std::stoi(paramTitles[paramTitles.size()-1]);
    else if(isStrSame(name, "paramsDir/nEtaBins")) nEtaBinsTemp = (UInt_t)std::stoi(paramTitles[paramTitles.size()-1]);
  }


  const UInt_t nMaxHiBins = 200;
  const UInt_t nMaxV2Bins = 72;
  const UInt_t nMaxV3Bins = 72;
  const UInt_t nMaxEtaBins = 82;
  
  const UInt_t nHiBins = nHiBinsTemp;
  const UInt_t nV2Bins = nV2BinsTemp;
  const UInt_t nV3Bins = nV3BinsTemp;
  const UInt_t nEtaBins = nEtaBinsTemp;

  UInt_t hiBins[nMaxHiBins+1];
  Float_t v2Bins[nMaxV2Bins+1];
  Float_t v3Bins[nMaxV3Bins+1];
  Float_t etaBins[nMaxEtaBins+1];
  UInt_t nPhiBins[nMaxEtaBins];

  std::vector<UInt_t> hiBins2;
  hiBins2.reserve(nHiBins+1);
  
  std::vector<Float_t> v2Bins2;
  v2Bins2.reserve(nV2Bins+1);

  std::vector<Float_t> v3Bins2;
  v3Bins2.reserve(nV3Bins+1);

  std::vector<Float_t> etaBins2;
  etaBins2.reserve(nEtaBins+1);

  for(unsigned int i = 0; i < paramNames.size(); ++i){
    if(isStrSame(paramNames[i], "paramsDir/hiBins")) getBinsFromString(paramTitles[i], nHiBins, hiBins);
    else if(isStrSame(paramNames[i], "paramsDir/v2Bins")) getBinsFromString(paramTitles[i], nV2Bins, v2Bins);
    else if(isStrSame(paramNames[i], "paramsDir/v3Bins")) getBinsFromString(paramTitles[i], nV3Bins, v3Bins);
    else if(isStrSame(paramNames[i], "paramsDir/etaBins")) getBinsFromString(paramTitles[i], nEtaBins, etaBins);
    else if(isStrSame(paramNames[i], "paramsDir/nPhiBins")) getBinsFromString(paramTitles[i], nEtaBins-1, nPhiBins);
  }

  for(UInt_t hI = 0; hI < nHiBins+1; ++hI){hiBins2.push_back(hiBins[hI]);}
  for(UInt_t vI = 0; vI < nV2Bins+1; ++vI){v2Bins2.push_back(v2Bins[vI]);}
  for(UInt_t vI = 0; vI < nV3Bins+1; ++vI){v3Bins2.push_back(v3Bins[vI]);}
  for(UInt_t eI = 0; eI < nEtaBins+1; ++eI){etaBins2.push_back(etaBins[eI]);}

  UInt_t nTowersTemp = 0;
  for(UInt_t tI = 0; tI < nEtaBins; ++tI){
    nTowersTemp += nPhiBins[tI];
  }
  const UInt_t nMaxTowers = 10000;
  const UInt_t nTowers = nTowersTemp;
  
  /*
  dispBins("nHiBins", nHiBins, hiBins);
  dispBins("nV2Bins", nV2Bins, v2Bins);
  dispBins("nV3Bins", nV3Bins, v3Bins);
  dispBins("nEtaBins", nEtaBins, etaBins);
  dispBins("nPhiBins", nEtaBins-1, nPhiBins);
  */

  inEnsFile_p->cd();
  TTree* entryTree1_p = (TTree*)inEnsFile_p->Get("entryTree1");
  TTree* medianTree1_p = (TTree*)inEnsFile_p->Get("medianTree1");
  TTree* entryTree2_p = (TTree*)inEnsFile_p->Get("entryTree2");
  UInt_t origEntry1_, origEntry2_, eventID_;
  Float_t towersMedian_[nMaxTowers];
  entryTree1_p->SetBranchAddress("origEntry", &origEntry1_);

  medianTree1_p->SetBranchAddress("eventID", &eventID_);
  medianTree1_p->SetBranchAddress("towersMedian", towersMedian_);

  entryTree2_p->SetBranchAddress("origEntry", &origEntry2_);
    
  std::map<UInt_t, UChar_t> ensembleEntries1;
  UInt_t nEnsembleEntries1 = 0;
  std::map<UInt_t, UChar_t> ensembleEntries2;
  UInt_t nEnsembleEntries2 = 0;
  std::map<UInt_t, std::vector<Float_t> > eventIDToTowersMap;
  std::map<UShort_t, UShort_t> mapPosToArrayPos;

  unsigned int pos = 0;
  for(UInt_t eI = 0; eI < nEtaBins; ++eI){
    for(UInt_t pI = 0; pI < nPhiBins[eI]; ++pI){
      UShort_t id = ((UShort_t)100*eI) + pI;
      mapPosToArrayPos[id] = pos;
      ++pos;	
    }
  }
  
  const ULong64_t nEntriesEntry1 = (ULong64_t)entryTree1_p->GetEntries();
  const ULong64_t nEntriesMedian1 = (ULong64_t)medianTree1_p->GetEntries();
  const ULong64_t nEntriesEntry2 = (ULong64_t)entryTree2_p->GetEntries();

  for(ULong64_t entry = 0; entry < nEntriesEntry1; ++entry){
    entryTree1_p->GetEntry(entry);
    ++(ensembleEntries1[origEntry1_]);
    ++nEnsembleEntries1;
  }    

  for(ULong64_t entry = 0; entry < nEntriesEntry2; ++entry){
    entryTree2_p->GetEntry(entry);
    ++(ensembleEntries2[origEntry2_]);
    ++nEnsembleEntries2;
  }    

  for(ULong64_t entry = 0; entry < nEntriesMedian1; ++entry){
    medianTree1_p->GetEntry(entry);

    eventIDToTowersMap[eventID_] = {};
    eventIDToTowersMap[eventID_].reserve(nTowers);

    for(UInt_t tI = 0; tI < nTowers; ++tI){
      eventIDToTowersMap[eventID_].push_back(towersMedian_[tI]);
    }
  }    

  inEnsFile_p->Close();
  delete inEnsFile_p;

  TFile* inHYDFile_p = new TFile(inHYDFileName.c_str(), "READ");
  TTree* hydGenTree_p = (TTree*)inHYDFile_p->Get("genTree");
  const UInt_t nHYDEntries = hydGenTree_p->GetEntries();

  const UInt_t nMaxPart = 100000;
  std::vector<float>* genHYDPt_p = new std::vector<float>;
  std::vector<float>* genHYDPhi_p = new std::vector<float>;
  std::vector<float>* genHYDEta_p = new std::vector<float>;

  genHYDPt_p->reserve(nMaxPart);
  genHYDPhi_p->reserve(nMaxPart);
  genHYDEta_p->reserve(nMaxPart);
  
  hydGenTree_p->SetBranchStatus("*", 0);  
  hydGenTree_p->SetBranchStatus("pt", 1);
  hydGenTree_p->SetBranchStatus("phi", 1);
  hydGenTree_p->SetBranchStatus("eta", 1);
  
  hydGenTree_p->SetBranchAddress("pt", &(genHYDPt_p));
  hydGenTree_p->SetBranchAddress("phi", &(genHYDPhi_p));
  hydGenTree_p->SetBranchAddress("eta", &(genHYDEta_p));
  
  TFile* inPYTFiles_p[nMaxPYTFiles];
  TTree* pytGenParticleTrees_p[nMaxPYTFiles];
  TTree* pytAK4GenJetTree_ESchemeWTA_p[nMaxPYTFiles];
  UInt_t nPYTEntries[nMaxPYTFiles];
  UInt_t nGoodPYTEntries[nMaxPYTFiles];
  std::vector< std::vector<UInt_t> > goodPYTEntries;
  goodPYTEntries.reserve(nPYTFiles);
  
  std::vector<std::vector<float>* > genJtPt_p;
  std::vector<std::vector<float>* > genJtPhi_p;
  std::vector<std::vector<float>* > genJtEta_p;
  std::vector<std::vector<float>* > genPYTPt_p;
  std::vector<std::vector<float>* > genPYTPhi_p;
  std::vector<std::vector<float>* > genPYTEta_p;

  genJtPt_p.reserve(nPYTFiles);
  genJtPhi_p.reserve(nPYTFiles);
  genJtEta_p.reserve(nPYTFiles);
  genPYTPt_p.reserve(nPYTFiles);
  genPYTPhi_p.reserve(nPYTFiles);
  genPYTEta_p.reserve(nPYTFiles);
  
  for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
    inPYTFiles_p[pI] = new TFile(pytFiles[pI].c_str(), "READ");
    pytGenParticleTrees_p[pI] = (TTree*)inPYTFiles_p[pI]->Get("genParticleTree");
    pytAK4GenJetTree_ESchemeWTA_p[pI] = (TTree*)inPYTFiles_p[pI]->Get("ak4GenJetTree_ESchemeWTA");
    nPYTEntries[pI] = (UInt_t)pytGenParticleTrees_p[pI]->GetEntries();
    goodPYTEntries.push_back({});

    genJtPt_p.push_back(new std::vector<float>);
    genJtPhi_p.push_back(new std::vector<float>);
    genJtEta_p.push_back(new std::vector<float>);

    genPYTPt_p.push_back(new std::vector<float>);
    genPYTPhi_p.push_back(new std::vector<float>);
    genPYTEta_p.push_back(new std::vector<float>);

    genJtPt_p[pI]->reserve(nMaxJets);
    genJtPhi_p[pI]->reserve(nMaxJets);
    genJtEta_p[pI]->reserve(nMaxJets);    

    genPYTPt_p[pI]->reserve(nMaxPart);
    genPYTPhi_p[pI]->reserve(nMaxPart);
    genPYTEta_p[pI]->reserve(nMaxPart);    

    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchStatus("*", 0);
    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchStatus("genJtPt", 1);
    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchStatus("genJtPhi", 1);
    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchStatus("genJtEta", 1);

    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchAddress("genJtPt", &(genJtPt_p[pI]));
    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchAddress("genJtPhi", &(genJtPhi_p[pI]));
    pytAK4GenJetTree_ESchemeWTA_p[pI]->SetBranchAddress("genJtEta", &(genJtEta_p[pI]));

    pytGenParticleTrees_p[pI]->SetBranchStatus("*", 0);
    pytGenParticleTrees_p[pI]->SetBranchStatus("pt", 1);
    pytGenParticleTrees_p[pI]->SetBranchStatus("phi", 1);
    pytGenParticleTrees_p[pI]->SetBranchStatus("eta", 1);

    pytGenParticleTrees_p[pI]->SetBranchAddress("pt", &(genPYTPt_p[pI]));
    pytGenParticleTrees_p[pI]->SetBranchAddress("phi", &(genPYTPhi_p[pI]));
    pytGenParticleTrees_p[pI]->SetBranchAddress("eta", &(genPYTEta_p[pI]));

    std::cout << "Pre-processing PYTHIA input 1, " << nPYTEntries[pI] << " entries..." << std::endl;
    for(UInt_t entry = 0; entry < nPYTEntries[pI]; ++entry){
      pytAK4GenJetTree_ESchemeWTA_p[pI]->GetEntry(entry);

      for(unsigned int jI = 0; jI < genJtPt_p[pI]->size(); ++jI){
	if(TMath::Abs((*(genJtEta_p[pI]))[jI]) > 2.0) continue;
	if((*(genJtPt_p[pI]))[jI] < jtPtMins[pI]) continue;
	if((*(genJtPt_p[pI]))[jI] >= jtPtMaxs[pI]) continue;

	goodPYTEntries[pI].push_back(entry);
	break;
      }
    }

    nGoodPYTEntries[pI] = goodPYTEntries[pI].size();
    if(nGoodPYTEntries[pI] < nHYDEntries - nEnsembleEntries1) std::cout << "WARNING: nGoodPYTEntries " << nGoodPYTEntries[pI] << " in file \'" << pytFiles[pI] << "\' is less than HYDJET available, \'" << nHYDEntries - nEnsembleEntries1 << "\'. Jets will be reused" << std::endl;
  }

  
  std::cout << "Processing " << nHYDEntries << " (" << nHYDEntries - nEnsembleEntries1 << ") HYDJET..." << std::endl;

  std::vector<fastjet::PseudoJet> particlesIn, particlesOut;
  std::vector<fastjet::PseudoJet> jetsIn, jetsOut;
  fastjet::JetDefinition jetDef(fastjet::antikt_algorithm, 0.4);
  std::vector<std::map<UShort_t, Float_t> > inEventPUMap;
  std::vector<std::map<UShort_t, Float_t> > outEventPUMap;
  std::map<UShort_t, UShort_t> outEventMapToPUMap;  
  std::map<UShort_t, UShort_t> outEventMapToPUMapCounts;  
  
  
  for(UInt_t i = 0; i < nPYTFiles; ++i){
    inEventPUMap.push_back({});
    outEventPUMap.push_back({});
    
    for(UInt_t eI = 0; eI < etaTowBounds.size()-1; ++eI){

      Float_t etaVal = (etaTowBounds[eI] + etaTowBounds[eI+1])/2.;
      UShort_t etaPos = ((UShort_t)(std::lower_bound(etaBins2.begin(), etaBins2.end(), etaVal) -  etaBins2.begin()));
      if(etaPos != 0) --etaPos;

      std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[eI]]);
      std::vector<Float_t>* tempVect2 = &(setsOfTowInPhi[nPhiBins[etaPos]]);
      
      for(Int_t pI = 0; pI < nTowInPhi[eI]; ++pI){
	Float_t phiVal = ((*tempVect)[pI] + (*tempVect)[pI+1])/2.;
	Short_t phiPos = ((UShort_t)(std::lower_bound(tempVect2->begin(), tempVect2->end(), phiVal) - tempVect2->begin()));
	if(phiPos != 0) --phiPos;

	UShort_t mapPos = ((UShort_t)100*eI) + (UShort_t)pI;
	UShort_t mapPos2 = 100*etaPos + phiPos;

	(inEventPUMap[i])[mapPos] = 0.0;
	(outEventPUMap[i])[mapPos] = 0.0;
	if(i == 0){
	  outEventMapToPUMap[mapPos] = mapPos2;
	  ++(outEventMapToPUMapCounts[mapPos2]);
	}
      }
    }
  }

  bool hasDisp = false;
  const UInt_t nHYDDiv = TMath::Max((UInt_t)1, nHYDEntries/40);
  
  UInt_t currentPos = 0;
  for(UInt_t entry = 0; entry < nHYDEntries; ++entry){
    if(entry%nHYDDiv == 0) hasDisp = false;
    if(ensembleEntries2[entry] == 0) continue;

    if(!hasDisp){
      std::cout << " Entry " << entry << "/" << nHYDEntries << std::endl;
      hasDisp = true;
    }    
    
    Float_t genHIHFSumHYD_ = 0.0;
    Float_t genV2SumXHYD_ = 0.0;
    Float_t genV2SumYHYD_ = 0.0;
    Float_t genV3SumXHYD_ = 0.0;
    Float_t genV3SumYHYD_ = 0.0;

    Float_t genHIHFSumPYT_[nMaxPYTFiles];
    Float_t genV2SumXPYT_[nMaxPYTFiles];
    Float_t genV2SumYPYT_[nMaxPYTFiles];
    Float_t genV3SumXPYT_[nMaxPYTFiles];
    Float_t genV3SumYPYT_[nMaxPYTFiles];

    hydGenTree_p->GetEntry(entry);

    for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
      pytAK4GenJetTree_ESchemeWTA_p[pI]->GetEntry((goodPYTEntries[pI])[currentPos%nGoodPYTEntries[pI]]);
      pytGenParticleTrees_p[pI]->GetEntry((goodPYTEntries[pI])[currentPos%nGoodPYTEntries[pI]]);
      genHIHFSumPYT_[pI] = 0.0;
      genV2SumXPYT_[pI] = 0.0;
      genV2SumYPYT_[pI] = 0.0;
      genV3SumXPYT_[pI] = 0.0;
      genV3SumYPYT_[pI] = 0.0;

      for(auto const & val : inEventPUMap[pI]){
	(inEventPUMap[pI])[val.first] = 0.0;
	(outEventPUMap[pI])[val.first] = 0.0;
      }
    }

    for(unsigned int i = 0; i < genHYDEta_p->size(); ++i){
      if((*genHYDPt_p)[i] < 0.5) continue;
      if(TMath::Abs((*genHYDEta_p)[i]) > 5.0) continue;
      if(TMath::Abs((*genHYDEta_p)[i]) > 3.0) genHIHFSumHYD_ += (*genHYDPt_p)[i];

      if((*genHYDPt_p)[i] >= 1.0 && (*genHYDPt_p)[i] < 3.0){
	if(TMath::Abs((*genHYDEta_p)[i]) < 1.){
	  genV2SumXHYD_ += TMath::Cos(2*((*genHYDPhi_p)[i]));
	  genV2SumYHYD_ += TMath::Sin(2*((*genHYDPhi_p)[i]));

	  genV3SumXHYD_ += TMath::Cos(3*((*genHYDPhi_p)[i]));
	  genV3SumYHYD_ += TMath::Sin(3*((*genHYDPhi_p)[i]));
	}
      }

      UShort_t etaPos = ((UShort_t)(std::lower_bound(etaTowBounds.begin(), etaTowBounds.end(), (*genHYDEta_p)[i]) - etaTowBounds.begin()));
      if(etaPos != 0) --etaPos;
      std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[etaPos]]);
      UShort_t phiPos = ((UShort_t)(std::lower_bound(tempVect->begin(), tempVect->end(), (*genHYDPhi_p)[i]) - tempVect->begin()));
      if(phiPos != 0) --phiPos;
      
      UShort_t pos = ((UShort_t)etaPos*100) + (UShort_t)phiPos;
      
      for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
	(inEventPUMap[pI])[pos] += (*genHYDPt_p)[i];	
      }
    }

    genHIBinHYD_ = hiBinFromGenHF(genHIHFSumHYD_);
    genV2PhiHYD_ = TMath::ATan2(genV2SumYHYD_, genV2SumXHYD_);
    if(genV2PhiHYD_ < -TMath::Pi()/2.) genV2PhiHYD_ += TMath::Pi();
    else if(genV2PhiHYD_ > TMath::Pi()/2.) genV2PhiHYD_ -= TMath::Pi();

    genV3PhiHYD_ = TMath::ATan2(genV3SumYHYD_, genV3SumXHYD_);
    if(genV3PhiHYD_ < -TMath::Pi()/3.) genV3PhiHYD_ += TMath::Pi()*2./3.;
    else if(genV3PhiHYD_ > TMath::Pi()/3.) genV3PhiHYD_ -= TMath::Pi()*2./3.;

    genHIPosHYD_ = ((UChar_t)(std::lower_bound(hiBins2.begin(), hiBins2.end(), genHIBinHYD_) - hiBins2.begin()));
    if(genHIPosHYD_ != 0) --genHIPosHYD_;
    
    genV2PosHYD_ = ((UChar_t)(std::lower_bound(v2Bins2.begin(), v2Bins2.end(), genV2PhiHYD_) - v2Bins2.begin()));
    if(genV2PosHYD_ != 0) --genV2PosHYD_;

    genV3PosHYD_ = ((UChar_t)(std::lower_bound(v3Bins2.begin(), v3Bins2.end(), genV3PhiHYD_) - v3Bins2.begin()));
    if(genV3PosHYD_ != 0) --genV3PosHYD_;

    genEventIDHYD_ = ((UInt_t)genHIPosHYD_)*10000 + ((UInt_t)genV3PosHYD_)*100 + ((UInt_t)genV2PosHYD_);    
    
    for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
      for(unsigned int i = 0; i < genPYTEta_p[pI]->size(); ++i){
	if((*(genPYTPt_p[pI]))[i] < 0.5) continue;
	if(TMath::Abs((*(genPYTEta_p[pI]))[i]) > 5.0) continue;
	if(TMath::Abs((*(genPYTEta_p[pI]))[i]) > 3.0) genHIHFSumPYT_[pI] += (*(genPYTPt_p[pI]))[i];

	if((*(genPYTPt_p[pI]))[i] >= 1.0 && (*(genPYTPt_p[pI]))[i] < 3.0){
	  if(TMath::Abs((*(genPYTEta_p[pI]))[i]) < 1.0){
	    genV2SumXPYT_[pI] += TMath::Cos(2*((*(genPYTPhi_p[pI]))[i]));
	    genV2SumYPYT_[pI] += TMath::Sin(2*((*(genPYTPhi_p[pI]))[i]));
	    
	    genV3SumXPYT_[pI] += TMath::Cos(3*((*(genPYTPhi_p[pI]))[i]));
	    genV3SumYPYT_[pI] += TMath::Sin(3*((*(genPYTPhi_p[pI]))[i]));
	  }
	}
	
	UShort_t etaPos = ((UShort_t)(std::lower_bound(etaTowBounds.begin(), etaTowBounds.end(), (*(genPYTEta_p[pI]))[i]) - etaTowBounds.begin()));
	if(etaPos != 0) --etaPos;
	    
	std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[etaPos]]);
	UShort_t phiPos = ((UShort_t)(std::lower_bound(tempVect->begin(), tempVect->end(), (*(genPYTPhi_p[pI]))[i]) - tempVect->begin()));
	if(phiPos != 0) --phiPos;

	UShort_t pos = ((UShort_t)etaPos*100) + (UShort_t)phiPos;
	(inEventPUMap[pI])[pos] += (*(genPYTPt_p[pI]))[i];	
      }    
      
      genHIHFSumPYT_[pI] += genHIHFSumHYD_;
      genV2SumXPYT_[pI] += genV2SumXHYD_;
      genV2SumYPYT_[pI] += genV2SumYHYD_;
      genV3SumXPYT_[pI] += genV3SumXHYD_;
      genV3SumYPYT_[pI] += genV3SumYHYD_;
 
      genHIBinPYT_[pI] = hiBinFromGenHF(genHIHFSumPYT_[pI]);
      genV2PhiPYT_[pI] = TMath::ATan2(genV2SumYPYT_[pI], genV2SumXPYT_[pI]);
      if(genV2PhiPYT_[pI] < -TMath::Pi()/2.) genV2PhiPYT_[pI] += TMath::Pi();
      else if(genV2PhiPYT_[pI] > TMath::Pi()/2.) genV2PhiPYT_[pI] -= TMath::Pi();
      
      genV3PhiPYT_[pI] = TMath::ATan2(genV3SumYPYT_[pI], genV3SumXPYT_[pI]);
      if(genV3PhiPYT_[pI] < -TMath::Pi()/3.) genV3PhiPYT_[pI] += TMath::Pi()*2./3.;
      else if(genV3PhiPYT_[pI] > TMath::Pi()/3.) genV3PhiPYT_[pI] -= TMath::Pi()*2./3.;            

      genHIPosPYT_[pI] = ((UChar_t)(std::lower_bound(hiBins2.begin(), hiBins2.end(), genHIBinPYT_[pI]) - hiBins2.begin()));
      if(genHIPosPYT_[pI] != 0) --genHIPosPYT_[pI];

      genV2PosPYT_[pI] = ((UChar_t)(std::lower_bound(v2Bins2.begin(), v2Bins2.end(), genV2PhiPYT_[pI]) - v2Bins2.begin()));
      if(genV2PosPYT_[pI] != 0) --genV2PosPYT_[pI];

      genV3PosPYT_[pI] = ((UChar_t)(std::lower_bound(v3Bins2.begin(), v3Bins2.end(), genV3PhiPYT_[pI]) - v3Bins2.begin()));
      if(genV3PosPYT_[pI] != 0) --genV3PosPYT_[pI];

      genEventIDPYT_[pI] = ((UInt_t)genHIPosPYT_[pI])*10000 + ((UInt_t)genV3PosPYT_[pI])*100 + ((UInt_t)genV2PosPYT_[pI]);

      for(auto const & val : outEventPUMap[pI]){
	(outEventPUMap[pI])[val.first] = ((eventIDToTowersMap[genEventIDPYT_[pI]])[mapPosToArrayPos[outEventMapToPUMap[val.first]]])/(Float_t)outEventMapToPUMapCounts[outEventMapToPUMap[val.first]];
      }

      const UInt_t nMaxEtaBinsPU = 82;
      const UInt_t nEtaBinsPU = etaTowBounds.size()-1;
      Float_t inEvtMean[nMaxEtaBinsPU];
      Float_t outEvtMean[nMaxEtaBinsPU];
      Float_t inEvtSigma[nMaxEtaBinsPU];
      Float_t outEvtSigma[nMaxEtaBinsPU];
      UInt_t totalTowers[nMaxEtaBinsPU];
      Bool_t isTowUsed[nMaxTowers];

      unsigned int towPos = 0;
      for(UInt_t eI = 0; eI < nEtaBinsPU; ++eI){
	inEvtMean[eI] = 0.0;
	outEvtMean[eI] = 0.0;
	inEvtSigma[eI] = 0.0;
	outEvtSigma[eI] = 0.0;
	totalTowers[eI] = 0;
	
	for(Int_t phiI = 0; phiI < nTowInPhi[eI]; ++phiI){
	  UShort_t pos = ((UShort_t)100*eI) + phiI;
	  inEvtMean[eI] += (inEventPUMap[pI])[pos];
	  outEvtMean[eI] += (outEventPUMap[pI])[pos];
	  inEvtSigma[eI] += ((inEventPUMap[pI])[pos])*((inEventPUMap[pI])[pos]);
	  outEvtSigma[eI] += ((outEventPUMap[pI])[pos])*((outEventPUMap[pI])[pos]);
	  ++(totalTowers[eI]);
	  isTowUsed[towPos] = false;
	  ++towPos;
	}
	
	inEvtMean[eI] /= (Float_t)nTowInPhi[eI];
	outEvtMean[eI] /= (Float_t)nTowInPhi[eI];

      	inEvtSigma[eI] = inEvtSigma[eI]/(Float_t)nTowInPhi[eI] - inEvtMean[eI]*inEvtMean[eI];
	if(inEvtSigma[eI] >= 0) inEvtSigma[eI] = TMath::Sqrt(inEvtSigma[eI]);
	else inEvtSigma[eI] = 0.0;
	
      	outEvtSigma[eI] = outEvtSigma[eI]/(Float_t)nTowInPhi[eI] - outEvtMean[eI]*outEvtMean[eI];
	if(outEvtSigma[eI] >= 0) outEvtSigma[eI] = TMath::Sqrt(outEvtSigma[eI]);
	else outEvtSigma[eI] = 0.0;
      }


      particlesIn.clear();
      particlesOut.clear();
      for(UInt_t eI = 0; eI < nEtaBinsPU; ++eI){
	Float_t etaVal = (etaTowBounds[eI] + etaTowBounds[eI+1])/2.;
	std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[eI]]);

	for(Int_t phiI = 0; phiI < nTowInPhi[eI]; ++phiI){
	  UShort_t pos = ((UShort_t)100*eI) + phiI;
	  Float_t phiVal = ((*tempVect)[pI] + (*tempVect)[pI+1])/2.;

	  Double_t ptIn = (inEventPUMap[pI])[pos] - inEvtMean[eI] - inEvtSigma[eI];
	  Double_t ptOut = (inEventPUMap[pI])[pos] - outEvtMean[eI] - outEvtSigma[eI];
	  TLorentzVector tL;
	  if(ptIn > 0.1){
	    tL.SetPtEtaPhiM(ptIn, etaVal, phiVal, 0.0);	  
	    particlesIn.push_back(fastjet::PseudoJet(tL.Px(), tL.Py(), tL.Pz(), tL.E()));
	  }

	  if(ptOut > 0.1){
	    tL.SetPtEtaPhiM(ptOut, etaVal, phiVal, 0.0);	  
	    particlesOut.push_back(fastjet::PseudoJet(tL.Px(), tL.Py(), tL.Pz(), tL.E()));
	  }
	}
      }

      fastjet::ClusterSequence* cs = new fastjet::ClusterSequence(particlesIn, jetDef);
      jetsIn = fastjet::sorted_by_pt(cs->inclusive_jets(20.));
      delete cs;

      for(unsigned int jI = 0; jI < jetsIn.size(); ++jI){
	towPos = 0;
	for(UInt_t eI = 0; eI < nEtaBinsPU; ++eI){
	  Float_t etaVal = (etaTowBounds[eI] + etaTowBounds[eI+1])/2.;
	  std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[eI]]);

	  for(Int_t phiI = 0; phiI < nTowInPhi[eI]; ++phiI){
	    if(isTowUsed[towPos]){
	      ++towPos;
	      continue;
	    }
	    else if(nTowInPhi[eI]/totalTowers[eI] > 2){
	      ++towPos;
	      continue;
	    }

	    Float_t phiVal = ((*tempVect)[pI] + (*tempVect)[pI+1])/2.;

	    if(getDR(etaVal, phiVal, jetsIn[jI].eta(), jetsIn[jI].phi_std()) < 0.4){
	      isTowUsed[towPos] = true;
	      --(totalTowers[eI]);
	    }

	    ++towPos;
	  }
	}    	
      }

      particlesIn.clear();
      towPos = 0;
      for(UInt_t eI = 0; eI < nEtaBinsPU; ++eI){
	inEvtMean[eI] = 0.0;
	inEvtSigma[eI] = 0.0;

	for(Int_t phiI = 0; phiI < nTowInPhi[eI]; ++phiI){
	  if(isTowUsed[towPos]){
	    ++towPos;
	    continue;
	  }
	  UShort_t pos = ((UShort_t)100*eI) + phiI;
	  inEvtMean[eI] += (inEventPUMap[pI])[pos];
	  inEvtSigma[eI] += ((inEventPUMap[pI])[pos])*((inEventPUMap[pI])[pos]);
	  ++towPos;
	}

	inEvtMean[eI] /= (Float_t)totalTowers[eI];       
      	inEvtSigma[eI] = inEvtSigma[eI]/(Float_t)totalTowers[eI] - inEvtMean[eI]*inEvtMean[eI];
	if(inEvtSigma[eI] >= 0) inEvtSigma[eI] = TMath::Sqrt(inEvtSigma[eI]);
	else inEvtSigma[eI] = 0.0;
      }

      for(UInt_t eI = 0; eI < nEtaBinsPU; ++eI){
	Float_t etaVal = (etaTowBounds[eI] + etaTowBounds[eI+1])/2.;
	std::vector<Float_t>* tempVect = &(setsOfTowInPhi[nTowInPhi[eI]]);

	for(Int_t phiI = 0; phiI < nTowInPhi[eI]; ++phiI){
	  UShort_t pos = ((UShort_t)100*eI) + phiI;
	  Float_t phiVal = ((*tempVect)[pI] + (*tempVect)[pI+1])/2.;

	  Double_t ptIn = (inEventPUMap[pI])[pos] - inEvtMean[eI] - inEvtSigma[eI];
	  TLorentzVector tL;
	  if(ptIn > 0.1){
	    tL.SetPtEtaPhiM(ptIn, etaVal, phiVal, 0.0);	  
	    particlesIn.push_back(fastjet::PseudoJet(tL.Px(), tL.Py(), tL.Pz(), tL.E()));
	  }
	}
      }

      cs = new fastjet::ClusterSequence(particlesIn, jetDef);
      jetsIn = fastjet::sorted_by_pt(cs->inclusive_jets(20.));
      delete cs;

      cs = new fastjet::ClusterSequence(particlesOut, jetDef);
      jetsOut = fastjet::sorted_by_pt(cs->inclusive_jets(20.));
      delete cs;

      std::vector<bool> jetsInIsUsed, jetsOutIsUsed;

      for(unsigned int jI = 0; jI < jetsIn.size(); ++jI){jetsInIsUsed.push_back(false);}
      for(unsigned int jI = 0; jI < jetsOut.size(); ++jI){jetsOutIsUsed.push_back(false);}
      
      nGenJt_[pI] = 0;
           
      for(UInt_t jI = 0; jI < genJtPt_p[pI]->size(); ++jI){
	if(TMath::Abs((*(genJtEta_p[pI]))[jI]) > 2.0) continue;
	if((*(genJtPt_p[pI]))[jI] < jtPtMins[pI]) continue;
	if((*(genJtPt_p[pI]))[jI] >= jtPtMaxs[pI]) continue;

	genJtPt_[pI][nGenJt_[pI]] = (*(genJtPt_p[pI]))[jI];
	genJtPhi_[pI][nGenJt_[pI]] = (*(genJtPhi_p[pI]))[jI];
	genJtEta_[pI][nGenJt_[pI]] = (*(genJtEta_p[pI]))[jI];

	int matchPos = -1;
	for(unsigned int jI2 = 0; jI2 < jetsIn.size(); ++jI2){
	  if(jetsInIsUsed[jI2]) continue;

	  if(getDR(genJtEta_[pI][nGenJt_[pI]], genJtPhi_[pI][nGenJt_[pI]], jetsIn[jI2].eta(), jetsIn[jI2].phi_std()) < 0.4){
	    matchPos = jI2;
	    jetsInIsUsed[jI2] = true;
	    break;
	  }
	}

	if(matchPos >= 0){
	  inEvtPUJtPt_[pI][nGenJt_[pI]] = jetsIn[matchPos].pt();
	  inEvtPUJtPhi_[pI][nGenJt_[pI]] = jetsIn[matchPos].phi_std();
	  inEvtPUJtEta_[pI][nGenJt_[pI]] = jetsIn[matchPos].eta();
	}
	else{
	  inEvtPUJtPt_[pI][nGenJt_[pI]] = -99.;
	  inEvtPUJtPhi_[pI][nGenJt_[pI]] = -99.;
	  inEvtPUJtEta_[pI][nGenJt_[pI]] = -99.;
	}

	matchPos = -1;
	for(unsigned int jI2 = 0; jI2 < jetsOut.size(); ++jI2){
	  if(jetsOutIsUsed[jI2]) continue;

	  if(getDR(genJtEta_[pI][nGenJt_[pI]], genJtPhi_[pI][nGenJt_[pI]], jetsOut[jI2].eta(), jetsOut[jI2].phi_std()) < 0.4){
	    matchPos = jI2;
	    jetsOutIsUsed[jI2] = true;
	    break;
	  }
	}

	if(matchPos >= 0){
	  outEvtPUJtPt_[pI][nGenJt_[pI]] = jetsOut[matchPos].pt();
	  outEvtPUJtPhi_[pI][nGenJt_[pI]] = jetsOut[matchPos].phi_std();
	  outEvtPUJtEta_[pI][nGenJt_[pI]] = jetsOut[matchPos].eta();
	}
	else{
	  outEvtPUJtPt_[pI][nGenJt_[pI]] = -99.;
	  outEvtPUJtPhi_[pI][nGenJt_[pI]] = -99.;
	  outEvtPUJtEta_[pI][nGenJt_[pI]] = -99.;
	}

	++(nGenJt_[pI]);
      }
      
      jetTree_p[pI]->Fill();
    }
    globalTree_p->Fill();
    
    ++currentPos;
  }
  
  for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
    genJtPt_p[pI]->clear();
    genJtPhi_p[pI]->clear();
    genJtEta_p[pI]->clear();

    delete genJtPt_p[pI];
    delete genJtPhi_p[pI];
    delete genJtEta_p[pI];

    inPYTFiles_p[pI]->Close();
    delete inPYTFiles_p[pI];
  }

  genHYDPt_p->clear();
  genHYDPhi_p->clear();
  genHYDEta_p->clear();
  
  delete genHYDPt_p;
  delete genHYDPhi_p;
  delete genHYDEta_p;

  inHYDFile_p->Close();
  delete inHYDFile_p;  

  outFile_p->cd();

  globalTree_p->Write("", TObject::kOverwrite);
  /*
  delete genHIBinHYD_b;
  delete genHIPosHYD_b;
  delete genV2PhiHYD_b;
  delete genV2PosHYD_b;
  delete genV3PhiHYD_b;
  delete genV3PosHYD_b;
  delete genEventIDHYD_b;
  */
  delete globalTree_p;

  for(UInt_t pI = 0; pI < nPYTFiles; ++pI){
    jetTree_p[pI]->Write("", TObject::kOverwrite);
    delete jetTree_p[pI];
  }

  TDirectoryFile* paramsDir_p = (TDirectoryFile*)outFile_p->mkdir("paramsDir");
  paramsDir_p->cd();

  for(unsigned int pI = 0; pI < paramNames.size(); ++pI){
    std::string name = paramNames[pI].substr(paramNames[pI].find("/")+1, paramNames[pI].size());
    TNamed tempName(name.c_str(), paramTitles[pI].c_str());
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
  if(argc != 6){
    std::cout << "Usage: ./bin/testEnsembleMaps.exe <inEnsFileName> <inHYDFileName> <inPYTFileNames> <inJtPtMins> <inJtPtMaxs>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += testEnsembleMaps(argv[1], argv[2], argv[3], argv[4], argv[5]);
  return retVal;
}
