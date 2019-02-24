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
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TNamed.h"
#include "TTree.h"

//Local dependencies
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/getLinBins.h"
#include "include/histDefUtility.h"
#include "include/plotUtilities.h"

int quickMedianCheck(const std::string inFileName, std::string inJtPtMins)
{
  cppWatch timer;
  timer.start();
  
  if(!checkFile(inFileName)){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  std::vector<std::string> jtPtMinStrVect;
  if(inJtPtMins.size() != 0){
    if(inJtPtMins.substr(inJtPtMins.size()-1, 1).find(",") == std::string::npos) inJtPtMins = inJtPtMins + ",";

    while(inJtPtMins.find(",") != std::string::npos){
      int temp = std::stoi(inJtPtMins.substr(0, inJtPtMins.find(",")));
      jtPtMinStrVect.push_back(prettyString(temp, 1, true));
      inJtPtMins.replace(0, inJtPtMins.find(",")+1, "");
    }    
  }
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  std::string outFileName = inFileName.substr(0, inFileName.find(".root"));
  while(outFileName.find("/") != std::string::npos){
    outFileName.replace(0, outFileName.find("/")+1, "");
  }
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  outFileName = "output/" + dateStr + "/" + outFileName + "_QuickMedianCheck_" + dateStr + ".root";

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  const Int_t nEtaPhiSum = 1062;
  const Int_t nJtPtMin = jtPtMinStrVect.size();

  const Int_t nMean = 10;

  Float_t etaPhiSum[nEtaPhiSum];
  Float_t etaPhiSum_JetPtMin[nJtPtMin][nEtaPhiSum];

   TH1F* absDeltaChannelMedian_p = new TH1F("absDeltaChannelMedian_h", ";Tower Channel;|#Delta Median_{1,2}|/Max", nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
  TH1F* absDeltaMedian_p = new TH1F("absDeltaMedian_h", ";|#Delta Median_{1,2}|/Max;Counts", 30, 0, 0.3);
  TH1F* absDeltaChannelMean10_p = new TH1F("absDeltaChannelMean10_h", ";Tower Channel;|#Delta Mean10_{1,2}|/Max", nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
  TH1F* absDeltaMean10_p = new TH1F("absDeltaMean10_h", ";|#Delta Mean10_{1,2}|/Max;Counts", 30, 0, 0.3);

  TH1F* deltaChannelMedian_p = new TH1F("deltaChannelMedian_h", ";Tower Channel;#Delta Median_{1,2}/Max", nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
  TH1F* deltaMedian_p = new TH1F("deltaMedian_h", ";#Delta Median_{1,2}/Max;Counts", 30, -0.3, 0.3);
  TH1F* deltaChannelMean10_p = new TH1F("deltaChannelMean10_h", ";Tower Channel;#Delta Mean10_{1,2}/Max", nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
  TH1F* deltaMean10_p = new TH1F("deltaMean10_h", ";#Delta Mean10_{1,2}/Max;Counts", 30, -0.3, 0.3);
  centerTitles({absDeltaChannelMedian_p, absDeltaChannelMean10_p, absDeltaMedian_p, absDeltaMean10_p, deltaChannelMedian_p, deltaChannelMean10_p, deltaMedian_p, deltaMean10_p});

  TH1F* absDeltaChannelMedianHYDPYT_p[nJtPtMin];
  TH1F* absDeltaMedianHYDPYT_p[nJtPtMin];
  TH1F* absDeltaChannelMean10HYDPYT_p[nJtPtMin];
  TH1F* absDeltaMean10HYDPYT_p[nJtPtMin];
  TH1F* deltaChannelMedianHYDPYT_p[nJtPtMin];
  TH1F* deltaMedianHYDPYT_p[nJtPtMin];
  TH1F* deltaChannelMean10HYDPYT_p[nJtPtMin];
  TH1F* deltaMean10HYDPYT_p[nJtPtMin];
  std::vector<TH1*> tempVect;
  
  for(Int_t jI = 0; jI < nJtPtMin; ++jI){
    absDeltaChannelMedianHYDPYT_p[jI] = new TH1F(("absDeltaChannelMedianHYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";Tower Channel; |#Delta| HYD w/ HYD+PYT" + jtPtMinStrVect[jI]).c_str(), nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
    absDeltaMedianHYDPYT_p[jI] = new TH1F(("absDeltaMedianHYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";|#Delta| HYD w/ HYD+PYT" + jtPtMinStrVect[jI] + ";Counts").c_str(), 30, 0, 0.3);
    absDeltaChannelMean10HYDPYT_p[jI] = new TH1F(("absDeltaChannelMean10HYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";Tower Channel; |#Delta| HYD w/ HYD+PYT" + jtPtMinStrVect[jI]).c_str(), nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
    absDeltaMean10HYDPYT_p[jI] = new TH1F(("absDeltaMean10HYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), ("|#Delta| HYD w/ HYD+PYT" + jtPtMinStrVect[jI] + ";Counts").c_str(), 30, 0, 0.3);

    deltaChannelMedianHYDPYT_p[jI] = new TH1F(("deltaChannelMedianHYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";Tower Channel; #Delta HYD w/ HYD+PYT" + jtPtMinStrVect[jI]).c_str(), nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
    deltaMedianHYDPYT_p[jI] = new TH1F(("deltaMedianHYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";#Delta HYD w/ HYD+PYT" + jtPtMinStrVect[jI] + ";Counts").c_str(), 30, -0.3, 0.3);
    deltaChannelMean10HYDPYT_p[jI] = new TH1F(("deltaChannelMean10HYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), (";Tower Channel; #Delta HYD w/ HYD+PYT" + jtPtMinStrVect[jI]).c_str(), nEtaPhiSum, -0.5, ((Float_t)nEtaPhiSum) - 0.5);
    deltaMean10HYDPYT_p[jI] = new TH1F(("deltaMean10HYDPYT_" + jtPtMinStrVect[jI] + "_h").c_str(), ("#Delta HYD w/ HYD+PYT" + jtPtMinStrVect[jI] + ";Counts").c_str(), 30, -0.3, 0.3);
    tempVect.push_back(absDeltaChannelMedianHYDPYT_p[jI]);
    tempVect.push_back(absDeltaChannelMean10HYDPYT_p[jI]);
    tempVect.push_back(absDeltaMedianHYDPYT_p[jI]);
    tempVect.push_back(absDeltaMean10HYDPYT_p[jI]);
    tempVect.push_back(deltaChannelMedianHYDPYT_p[jI]);
    tempVect.push_back(deltaChannelMean10HYDPYT_p[jI]);
    tempVect.push_back(deltaMedianHYDPYT_p[jI]);
    tempVect.push_back(deltaMean10HYDPYT_p[jI]);
  }
  centerTitles(tempVect);

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  TTree* hydTree1_p = (TTree*)inFile_p->Get("hydTree1");
  TTree* hydTree2_p = (TTree*)inFile_p->Get("hydTree2");

  const Int_t nEtaBins = std::stoi(((TNamed*)inFile_p->Get("paramsDir/nEtaBins"))->GetTitle());
  std::string etaBinsStr = ((TNamed*)inFile_p->Get("paramsDir/etaBins"))->GetTitle();
  std::string nPhiBinsStr = ((TNamed*)inFile_p->Get("paramsDir/nPhiBins"))->GetTitle();

  if(etaBinsStr.size() != 0){
    if(etaBinsStr.substr(etaBinsStr.size()-1, 1).find(",") == std::string::npos) etaBinsStr = etaBinsStr + ",";
  }

  if(nPhiBinsStr.size() != 0){
    if(nPhiBinsStr.substr(nPhiBinsStr.size()-1, 1).find(",") == std::string::npos) nPhiBinsStr = nPhiBinsStr + ",";
  }
  
  Float_t etaBins[nEtaBins+1];
  Int_t nPhiBins[nEtaBins];

  unsigned int pos = 0;
  while(etaBinsStr.find(",") != std::string::npos){
    etaBins[pos] = std::stof(etaBinsStr.substr(0, etaBinsStr.find(",")));
    etaBinsStr.replace(0, etaBinsStr.find(",")+1, "");
    ++pos;
  }

  pos = 0;
  while(nPhiBinsStr.find(",") != std::string::npos){
    nPhiBins[pos] = std::stoi(nPhiBinsStr.substr(0, nPhiBinsStr.find(",")));
    nPhiBinsStr.replace(0, nPhiBinsStr.find(",")+1, "");
    ++pos;
  }

  std::cout << "nEtaBins: " << nEtaBins << std::endl;
  std::cout << "etaBins: ";
  for(int eI = 0; eI < nEtaBins; ++eI){
    std::cout << etaBins[eI] << ", ";
  }
  std::cout << etaBins[nEtaBins] << "." << std::endl;

  std::cout << "nPhiBins: ";
  for(int eI = 0; eI < nEtaBins-1; ++eI){
    std::cout << nPhiBins[eI] << ", ";
  }
  std::cout << nPhiBins[nEtaBins-1] << "." << std::endl;

  std::map<UShort_t, Float_t> towerToPhiCenter;
  std::map<UShort_t, UShort_t> towerToNPhi;
  std::map<UShort_t, Float_t> towerToEtaCenter;
  Int_t maxNPhiBinsTemp = -1;

  pos = 0;
  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    const Int_t nTempPhiBins = nPhiBins[eI];
    Float_t tempPhiBins[nTempPhiBins+1];
    getLinBins(-TMath::Pi(), TMath::Pi(), nTempPhiBins, tempPhiBins);

    if(nTempPhiBins > maxNPhiBinsTemp) maxNPhiBinsTemp = nTempPhiBins;
    
    for(Int_t pI = 0; pI < nTempPhiBins; ++pI){
      towerToPhiCenter[pos] = (tempPhiBins[pI] + tempPhiBins[pI+1])/2;
      towerToNPhi[pos] = nTempPhiBins;
      towerToEtaCenter[pos] = etaBins[eI];
      ++pos;
    }
  }
  const Int_t maxNPhiBins = maxNPhiBinsTemp;
  Float_t maxPhiBins[maxNPhiBins+1];
  getLinBins(-TMath::Pi(), TMath::Pi(), maxNPhiBins, maxPhiBins);  

  outFile_p->cd();
  const Int_t nHydTrees = 2;
  TH2F* etaPhiMedian_p[nHydTrees];
  TH2F* etaPhiMean10_p[nHydTrees];
  for(Int_t hI = 0; hI < nHydTrees; ++hI){
    etaPhiMedian_p[hI] = new TH2F(("etaPhiMedian" + std::to_string(hI+1) + "_h").c_str(), ";#eta;#phi", nEtaBins, etaBins, maxNPhiBins, maxPhiBins);
    etaPhiMean10_p[hI] = new TH2F(("etaPhiMean10" + std::to_string(hI+1) + "_h").c_str(), ";#eta;#phi", nEtaBins, etaBins, maxNPhiBins, maxPhiBins);
    centerTitles({etaPhiMedian_p[hI], etaPhiMean10_p[hI]});
  }
  
  UInt_t eventID;

  std::vector< std::vector<Float_t> > etaPhiSumVects1;
  std::vector< std::vector<Float_t> > etaPhiSumVects2;

  std::vector< std::vector<std::vector<Float_t> > > etaPhiSumJetPtMinVects1;
  std::vector< std::vector<std::vector<Float_t> > > etaPhiSumJetPtMinVects2;

  for(Int_t eI = 0; eI < nEtaPhiSum; ++eI){
    etaPhiSumVects1.push_back({});
    etaPhiSumVects2.push_back({});
    
    etaPhiSumJetPtMinVects1.push_back({});
    etaPhiSumJetPtMinVects2.push_back({});

    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      etaPhiSumJetPtMinVects1[eI].push_back({});
      etaPhiSumJetPtMinVects2[eI].push_back({});
    }
  }
  
  hydTree1_p->SetBranchStatus("*", 0);
  hydTree1_p->SetBranchStatus("eventID", 1);

  hydTree1_p->SetBranchAddress("eventID", &eventID);
  
  const UInt_t idToCheck = 0;

  const ULong64_t nEntries1 = hydTree1_p->GetEntries();
  for(ULong64_t entry = 0; entry < nEntries1; ++entry){
    hydTree1_p->GetEntry(entry);

    if(eventID != idToCheck) continue;

    hydTree1_p->SetBranchStatus("etaPhiSum", 1);
    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      hydTree1_p->SetBranchStatus(("etaPhiSum_JetPtMin" + jtPtMinStrVect[pI]).c_str(), 1);
    }

    hydTree1_p->SetBranchAddress("etaPhiSum", etaPhiSum);
    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      hydTree1_p->SetBranchAddress(("etaPhiSum_JetPtMin" + jtPtMinStrVect[pI]).c_str(), etaPhiSum_JetPtMin[pI]);
    }

    hydTree1_p->GetEntry(entry);

    for(unsigned int eI = 0; eI < nEtaPhiSum; ++eI){
      etaPhiSumVects1[eI].push_back(etaPhiSum[eI]);

      for(Int_t pI = 0; pI < nJtPtMin; ++pI){
	etaPhiSumJetPtMinVects1[eI][pI].push_back(etaPhiSum_JetPtMin[pI][eI]);
      }
    }    
    
    hydTree1_p->SetBranchStatus("*", 0);
    hydTree1_p->SetBranchStatus("eventID", 1);
    hydTree1_p->SetBranchAddress("eventID", &eventID);
  }

  hydTree2_p->SetBranchStatus("*", 0);
  hydTree2_p->SetBranchStatus("eventID", 1);

  hydTree2_p->SetBranchAddress("eventID", &eventID);

  const ULong64_t nEntries2 = hydTree2_p->GetEntries();
  for(ULong64_t entry = 0; entry < nEntries2; ++entry){
    hydTree2_p->GetEntry(entry);
    if(eventID != idToCheck) continue;

    hydTree2_p->SetBranchStatus("etaPhiSum", 1);
    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      hydTree2_p->SetBranchStatus(("etaPhiSum_JetPtMin" + jtPtMinStrVect[pI]).c_str(), 1);
    }

    hydTree2_p->SetBranchAddress("etaPhiSum", etaPhiSum);
    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      hydTree2_p->SetBranchAddress(("etaPhiSum_JetPtMin" + jtPtMinStrVect[pI]).c_str(), etaPhiSum_JetPtMin[pI]);
    }
    hydTree2_p->GetEntry(entry);

    for(unsigned int eI = 0; eI < nEtaPhiSum; ++eI){
      etaPhiSumVects2[eI].push_back(etaPhiSum[eI]);
      for(Int_t pI = 0; pI < nJtPtMin; ++pI){
	etaPhiSumJetPtMinVects1[eI][pI].push_back(etaPhiSum_JetPtMin[pI][eI]);
      }
    }    
    
    hydTree2_p->SetBranchStatus("*", 0);
    hydTree2_p->SetBranchStatus("eventID", 1);
    hydTree2_p->SetBranchAddress("eventID", &eventID);
  }
  
  inFile_p->Close();
  delete inFile_p;

  for(Int_t eI = 0; eI < nEtaPhiSum; ++eI){
    std::vector<Float_t> temp_1 = etaPhiSumVects1[eI];
    std::vector<Float_t> temp_2 = etaPhiSumVects2[eI];

    std::sort(std::begin(temp_1), std::end(temp_1));
    std::sort(std::begin(temp_2), std::end(temp_2));

    etaPhiSumVects1[eI] = temp_1;
    etaPhiSumVects2[eI] = temp_2;
  }

  for(Int_t eI = 0; eI < nEtaPhiSum; ++eI){
    Int_t startVal1 = etaPhiSumVects1[eI].size()/2;
    Float_t val1 = (etaPhiSumVects1[eI])[startVal1];
    if(etaPhiSumVects1[eI].size()%2 == 0) val1 = (val1 + (etaPhiSumVects1[eI])[startVal1-1])/2.;

    Float_t val1Mean10 = 0.0;
    for(Int_t vI = 0; vI < nMean; ++vI){
      val1Mean10 += (etaPhiSumVects1[eI])[startVal1 - nMean/2 + vI];
    }
    val1Mean10 /= (Double_t)nMean;   
    
    Int_t startVal2 = etaPhiSumVects2[eI].size()/2;
    Float_t val2 = (etaPhiSumVects2[eI])[startVal2];
    if(etaPhiSumVects2[eI].size()%2 == 0) val2 = (val2 + (etaPhiSumVects2[eI])[startVal2-1])/2.;

    Float_t val2Mean10 = 0.0;
    for(Int_t vI = 0; vI < nMean; ++vI){
      val2Mean10 += (etaPhiSumVects2[eI])[startVal2 - nMean/2 + vI];
    }
    val2Mean10 /= (Double_t)nMean;   

    etaPhiMedian_p[0]->Fill(towerToEtaCenter[eI], towerToPhiCenter[eI], val1);
    etaPhiMedian_p[1]->Fill(towerToEtaCenter[eI], towerToPhiCenter[eI], val2);

    etaPhiMean10_p[0]->Fill(towerToEtaCenter[eI], towerToPhiCenter[eI], val1Mean10);
    etaPhiMean10_p[1]->Fill(towerToEtaCenter[eI], towerToPhiCenter[eI], val2Mean10);

    Float_t fillVal = val1 - val2;
    if(TMath::Min(val1, val2) > 0) fillVal /= TMath::Min(val1, val2);
    else fillVal = 1;

    Float_t fillValMean10 = val1Mean10 - val2Mean10;
    if(TMath::Min(val1Mean10, val2Mean10) > 0) fillValMean10 /= TMath::Min(val1Mean10, val2Mean10);
    else fillValMean10 = 1;

    absDeltaChannelMedian_p->SetBinContent(eI+1, TMath::Abs(fillVal));
    absDeltaChannelMean10_p->SetBinContent(eI+1, TMath::Abs(fillValMean10));
    absDeltaMedian_p->Fill(TMath::Abs(fillVal));
    absDeltaMean10_p->Fill(TMath::Abs(fillValMean10));

    deltaChannelMedian_p->SetBinContent(eI+1, fillVal);
    deltaChannelMean10_p->SetBinContent(eI+1, fillValMean10);
    deltaMedian_p->Fill(fillVal);
    deltaMean10_p->Fill(fillValMean10);
  }

  for(Int_t eI = 0; eI < nEtaPhiSum; ++eI){
    etaPhiSumVects1[eI].insert(etaPhiSumVects1[eI].end(), etaPhiSumVects2[eI].begin(), etaPhiSumVects2[eI].end());
    std::vector<Float_t> temp = etaPhiSumVects1[eI];
    std::sort(std::begin(temp), std::end(temp));
    etaPhiSumVects1[eI] = temp;

    for(Int_t pI = 0; pI < nJtPtMin; ++pI){
      etaPhiSumJetPtMinVects1[eI][pI].insert(etaPhiSumJetPtMinVects1[eI][pI].end(), etaPhiSumJetPtMinVects2[eI][pI].begin(), etaPhiSumJetPtMinVects2[eI][pI].end());
      std::vector<Float_t> tempJetPtMin = etaPhiSumJetPtMinVects1[eI][pI];
      std::sort(std::begin(tempJetPtMin), std::end(tempJetPtMin));
      etaPhiSumJetPtMinVects1[eI][pI] = tempJetPtMin;
    }
  }
  
  for(Int_t eI = 0; eI < nEtaPhiSum; ++eI){
    Int_t startVal1 = etaPhiSumVects1[eI].size()/2;
    Float_t val1 = (etaPhiSumVects1[eI])[startVal1];
    if(etaPhiSumVects1[eI].size()%2 == 0) val1 = (val1 + (etaPhiSumVects1[eI])[startVal1-1])/2.;

    Float_t val1Mean10 = 0.0;
    for(Int_t vI = 0; vI < nMean; ++vI){
      val1Mean10 += (etaPhiSumVects1[eI])[startVal1 - nMean/2 + vI];
    }
    val1Mean10 /= (Double_t)nMean;   

    for(Int_t jI = 0; jI < nJtPtMin; ++jI){
      Int_t startVal2 = etaPhiSumJetPtMinVects1[eI][jI].size()/2;
      Float_t val2 = (etaPhiSumJetPtMinVects1[eI][jI])[startVal2];
      if(etaPhiSumJetPtMinVects1[eI][jI].size()%2 == 0) val2 = (val2 + (etaPhiSumJetPtMinVects1[eI][jI])[startVal2-1])/2.;

      Float_t val2Mean10 = 0.0;
      for(Int_t vI = 0; vI < nMean; ++vI){
	val2Mean10 += (etaPhiSumJetPtMinVects1[eI][jI])[startVal2 - nMean/2 + vI];
      }
      val2Mean10 /= (Double_t)nMean;   

      Float_t fillVal = val1 - val2;
      if(TMath::Min(val1, val2) > 0) fillVal /= TMath::Min(val1, val2);
      else fillVal = 1;

      Float_t fillValMean10 = val1Mean10 - val2Mean10;
      if(TMath::Min(val1Mean10, val2Mean10) > 0) fillValMean10 /= TMath::Min(val1Mean10, val2Mean10);
      else fillValMean10 = 1;

      absDeltaChannelMedianHYDPYT_p[jI]->SetBinContent(eI+1, TMath::Abs(fillVal));
      absDeltaChannelMean10HYDPYT_p[jI]->SetBinContent(eI+1, TMath::Abs(fillValMean10));
      absDeltaMedianHYDPYT_p[jI]->Fill(TMath::Abs(fillVal));
      absDeltaMean10HYDPYT_p[jI]->Fill(TMath::Abs(fillValMean10));

      deltaChannelMedianHYDPYT_p[jI]->SetBinContent(eI+1, fillVal);
      deltaChannelMean10HYDPYT_p[jI]->SetBinContent(eI+1, fillValMean10);
      deltaMedianHYDPYT_p[jI]->Fill(fillVal);
      deltaMean10HYDPYT_p[jI]->Fill(fillValMean10);
    }
  }
 
  outFile_p->cd();

  absDeltaChannelMedian_p->Write("", TObject::kOverwrite);
  delete absDeltaChannelMedian_p;

  absDeltaMedian_p->Write("", TObject::kOverwrite);
  delete absDeltaMedian_p;

  absDeltaChannelMean10_p->Write("", TObject::kOverwrite);
  delete absDeltaChannelMean10_p;

  absDeltaMean10_p->Write("", TObject::kOverwrite);
  delete absDeltaMean10_p;

  deltaChannelMedian_p->Write("", TObject::kOverwrite);
  delete deltaChannelMedian_p;

  deltaMedian_p->Write("", TObject::kOverwrite);
  delete deltaMedian_p;

  deltaChannelMean10_p->Write("", TObject::kOverwrite);
  delete deltaChannelMean10_p;

  deltaMean10_p->Write("", TObject::kOverwrite);
  delete deltaMean10_p;

  for(Int_t hI = 0; hI < nHydTrees; ++hI){
    etaPhiMedian_p[hI]->Write("", TObject::kOverwrite);
    delete etaPhiMedian_p[hI];

    etaPhiMean10_p[hI]->Write("", TObject::kOverwrite);
    delete etaPhiMean10_p[hI];
  }
  
  for(Int_t jI = 0; jI < nJtPtMin; ++jI){
    absDeltaChannelMedianHYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete absDeltaChannelMedianHYDPYT_p[jI];

    absDeltaChannelMean10HYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete absDeltaChannelMean10HYDPYT_p[jI];

    absDeltaMedianHYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete absDeltaMedianHYDPYT_p[jI];

    absDeltaMean10HYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete absDeltaMean10HYDPYT_p[jI];

    deltaChannelMedianHYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete deltaChannelMedianHYDPYT_p[jI];
    
    deltaChannelMean10HYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete deltaChannelMean10HYDPYT_p[jI];

    deltaMedianHYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete deltaMedianHYDPYT_p[jI];

    deltaMean10HYDPYT_p[jI]->Write("", TObject::kOverwrite);
    delete deltaMean10HYDPYT_p[jI];
  }
  
  outFile_p->Close();
  delete outFile_p;

  timer.stop();

  std::cout << "Total timing CPU: " <<   timer.totalCPU() << std::endl;
  std::cout << "Total timing Wall: " <<   timer.totalWall() << std::endl;
  std::cout << "Job Complete" << std::endl;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/quickMedianCheck.exe <inFileName> <inJtPtMins>" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += quickMedianCheck(argv[1], argv[2]);
  return retVal;
}
