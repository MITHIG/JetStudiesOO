//=====================================================================-*-C++-*-
// File and Version Information:
//      $Id: RooUnfoldExample.cxx 279 2011-02-11 18:23:44Z T.J.Adye $
//
// Description:
//      Simple example usage of the RooUnfold package using toy MC.
//
// Authors: Tim Adye <T.J.Adye@rl.ac.uk> and Fergus Wilson <fwilson@slac.stanford.edu>
//
//==============================================================================


#if !(defined(__CINT__) || defined(__CLING__)) || defined(__ACLIC__)
#include <iostream>
using std::cout;
using std::endl;

#include "TRandom.h"
#include "TRandom3.h"
#include <vector>
#include <string>

#include "TH1D.h"
#include "TFile.h"
#include "TVectorD.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPostScript.h"
#include "TH2D.h"
#include "TLine.h"
#include "TNtuple.h"
#include "TProfile.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TSystemDirectory.h"
#include "TTreeReaderArray.h"
#include "TMath.h"

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#endif

//==============================================================================
// Global definitions
//==============================================================================

const Double_t cutdummy= -99999.0;

//==============================================================================
// helper functions
//==============================================================================
void FillChain(TChain &chain, std::vector<std::string> &files); 
void GetFiles(char const *input, std::vector<std::string> &files); 
void Normalize2D(TH2* h); 
void RooSimplepTPbPb_Split2D(std::string date); 
   
// fill the tchain
void FillChain(TChain &chain, std::vector<std::string> &files) {
  for (auto file : files) {
    chain.Add(file.c_str());
  }
}

// get the files
void GetFiles(char const *input, std::vector<std::string> &files) {
  TSystemDirectory dir(input, input);
  TList *list = dir.GetListOfFiles();

  if (list) {
    TSystemFile *file;
    std::string fname;
    TIter next(list);
    while ((file = (TSystemFile *)next())) {
      fname = file->GetName();

      if (file->IsDirectory() && (fname.find(".") == std::string::npos)) {
        std::string newDir = std::string(input) + fname + "/";
        GetFiles(newDir.c_str(), files);
      } else if ((fname.find(".root") != std::string::npos)) {
        files.push_back(std::string(input) + fname);
        cout << files.back() << endl;
      }
    }
  }

  return;
}




void Normalize2D(TH2* h)
{
   Int_t nbinsYtmp = h->GetNbinsY();
   const Int_t nbinsY = nbinsYtmp;
   Double_t norm[nbinsY];
   for(Int_t biny=1; biny<=nbinsY; biny++)
     {
       norm[biny-1] = 0;
       for(Int_t binx=1; binx<=h->GetNbinsX(); binx++)
     {
       norm[biny-1] += h->GetBinContent(binx,biny);
     }
     }

   for(Int_t biny=1; biny<=nbinsY; biny++)
     {
       for(Int_t binx=1; binx<=h->GetNbinsX(); binx++)
     {
       if(norm[biny-1]==0)  continue;
       else
         {
  h->SetBinContent(binx,biny,h->GetBinContent(binx,biny)/norm[biny-1]);
  h->SetBinError(binx,biny,h->GetBinError(binx,biny)/norm[biny-1]);
         }
     }
     }
}







TH2D* CorrelationHist (const TMatrixD& cov,const char* name, const char* title,
		       Double_t lo, Double_t hi,Double_t lon,Double_t hin)
{
  Int_t nb= cov.GetNrows();
  Int_t na= cov.GetNcols();
  cout<<nb<<" "<<na<<endl;
  TH2D* h= new TH2D (name, title, nb, 0, nb, na, 0, na);
  h->SetAxisRange (-1.0, 1.0, "Z");
  for(int i=0; i < na; i++)
  for(int j=0; j < nb; j++) {
  Double_t Viijj= cov(i,i)*cov(j,j);
      if (Viijj>0.0) h->SetBinContent (i+1, j+1, cov(i,j)/sqrt(Viijj));
    }
  return h;
}

//==============================================================================
// Example Unfolding
//==============================================================================

void RooSimplepTPbPb_data2D(std::string date)
{
#ifdef __CINT__
  gSystem->Load("libRooUnfold");
#endif
  Int_t difference=1;
  Int_t Ppol=0;
  char const *inputData = "/eos/cms/store/group/phys_heavyions/hbossi/OOJetSubstructure/DataForests/";
  char const *inputMC = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/Dijet_pThat-15to1200_TuneCP5_5p36TeV_pythia8/OO_MC_DijetNoEmbedding_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260318_152047/0000/";
  std::cout << "Unfolding with... " << std::endl; 
  std::cout << "Data Sample: " << inputData << std::endl;
  std::cout << "MC Sample: " << inputMC << std::endl;
  
  cout << "==================================== pick up the response matrix for background==========================" << endl;
  ///////////////////parameter setting
  RooUnfold::ErrorTreatment errorTreatment = RooUnfold::kCovariance;   
  
  TRandom3* rand = new TRandom3();

   //***************************************************
  
   Double_t xbins[6];
  xbins[0] = 80.0;
  xbins[1] = 140.0;
  xbins[2] = 200.0;
  xbins[3] = 300.0;
  xbins[4] = 400.0; 
  xbins[5] = 500.0; 

  int nBinspT = 5; 


  std::vector<Double_t> kBinsMeasured  = {-0.05,0,0.1,0.2, 0.4}; 
  std::vector<Double_t> kBinsUnfolded = {-0.05,0, 0.1,0.2, 0.4}; 

 TH2D *h2raw(0);
   h2raw=new TH2D("r","raw",kBinsMeasured.size()-1,kBinsMeasured.data(), nBinspT ,xbins);
   //detector measure level

   TH2D *h2smeared(0);
   h2smeared=new TH2D("smeared","smeared",kBinsMeasured.size()-1,kBinsMeasured.data(),nBinspT,xbins);

   TH2D* h2resp(0);
   h2resp = new TH2D("h2resp", "h2resp", kBinsMeasured.size()-1,kBinsMeasured.data(), kBinsMeasured.size()-1,kBinsMeasured.data());

     //detector measure level no cuts
   TH2D *h2smearednocuts(0);
   h2smearednocuts=new TH2D("smearednocuts","smearednocuts",kBinsMeasured.size()-1,kBinsMeasured.data(),nBinspT,xbins);
   //true correlations with measured cuts

    TH2D *h2true(0);
    h2true=new TH2D("true","true",kBinsUnfolded.size()-1,kBinsUnfolded.data(),nBinspT ,xbins);
    //full true correlation
    TH2D *h2fulleff(0);
    h2fulleff=new TH2D("truef","truef",kBinsUnfolded.size()-1,kBinsUnfolded.data(),nBinspT ,xbins);
   

    TH2D * h2uf  = new TH2D("h2uf","h2uf",kBinsUnfolded.size()-1,kBinsUnfolded.data(),nBinspT ,xbins);
    TH2D* h2resppT(0);
   h2resppT = new TH2D("h2resppT", "h2resppT", nBinspT ,xbins, nBinspT ,xbins);

  

    TFile* frw;
    TH2D* h2_weight; 
    TH2D* h2_smooth; 

   

 
  
   TH2D *hcovariance(0);
  hcovariance=new TH2D("covariance","covariance",nBinspT,0.,1.,nBinspT,0,1.);

  

  TH2D *effnum=(TH2D*)h2fulleff->Clone("effnum");
  TH2D *effdenom=(TH2D*)h2fulleff->Clone("effdenom");
 
  effnum->Sumw2();
  effdenom->Sumw2();
  h2smeared->Sumw2();
  h2true->Sumw2();
  h2raw->Sumw2();
  h2fulleff->Sumw2();

  // --------------------------------------
  // handle the data
  // --------------------------------------

  std::vector<std::string> filesData;
  GetFiles(inputData, filesData);
  std::cout << "Getting the files from Data " << std::endl;

  TChain hiEventChainData("hiEvtAnalyzer/HiTree");
  FillChain(hiEventChainData, filesData);
  TTreeReader hiEventReaderData(&hiEventChainData);
  TTreeReaderValue<float>   HFpf_data(hiEventReaderData, "hiHF_pf");
  TTreeReaderValue<float>   weight_data(hiEventReaderData, "weight");
  TTreeReaderValue<float>   zVertex_data(hiEventReaderData, "vz");
  TTreeReaderValue<int>   nTrk_data(hiEventReaderData, "hiNtracks");
   
  TChain eventChainData("skimanalysis/HltTree");
  FillChain(eventChainData, filesData);
  TTreeReader eventReaderData(&eventChainData);
  TTreeReaderValue<int>   vertexFilterData(eventReaderData, "pprimaryVertexFilter");
  TTreeReaderValue<int>   clusterFilterData(eventReaderData, "pclusterCompatibilityFilter");

   /* read in jet information */
  TChain jetChainData("akCs4PFJetAnalyzer/t");
  FillChain(jetChainData, filesData);
  TTreeReader jetReaderData(&jetChainData);
  TTreeReaderValue<int>   jetNData(jetReaderData, "nref");
  TTreeReaderArray<float> jetEtaData(jetReaderData, "jteta");
  TTreeReaderArray<float> jetPtData(jetReaderData, "jtpt");
  TTreeReaderArray<float> rawPtData(jetReaderData, "rawpt");
  TTreeReaderArray<float> jetRgData(jetReaderData, "jtrg");
  TTreeReaderArray<float> jtPfCEFData(jetReaderData, "jtPfCEF");
  TTreeReaderArray<float> jtPfMUFData(jetReaderData, "jtPfMUF");
  TTreeReaderArray<float> jtPfNEFData(jetReaderData, "jtPfNEF");
  TTreeReaderArray<int> jtPfCHMData(jetReaderData, "jtPfCHM");

  /* read in trigger information */
  TChain trigChainData("hltanalysis/HltTree");
  FillChain(trigChainData, filesData);
  TTreeReader trigReaderData(&trigChainData);
  TTreeReaderValue<int> mbData(trigReaderData, "HLT_MinimumBiasHF_OR_BptxAND_v1");

  Long64_t totalEventsData = jetReaderData.GetEntries(true);

  for (Long64_t i = 0; i < totalEventsData; i++) {
    jetReaderData.Next(); eventReaderData.Next(); trigReaderData.Next(); hiEventReaderData.Next();

     // ------ step 1: Apply the necessary event filters ----
    if(*mbData != 1) continue; // trigger selection
    if(*zVertex_data< -15.0 || *zVertex_data > 15.0) continue;  // z vertex filter    
    if(*vertexFilterData == 0 || *clusterFilterData == 0) continue; // event filters
  
    float w = *weight_data;
    // loop over jets
    for (int j = 0; j < *jetNData; ++ j) {
      // jet kinematic and quality selections
      if (TMath::Abs(jetEtaData[j]) > 1.6) { continue; }
      if(jtPfCEFData[j] > 0.8) continue;
      if(jtPfMUFData[j] > 0.8) continue;
      if(jtPfNEFData[j] > 0.9) continue;
      if(jtPfCHMData[j] < 0) continue;

      if(jetPtData[j] < 80  || jetPtData[j] > 500) continue; // reco level

       h2raw->Fill(jetRgData[j], jetPtData[j], w);
    }

  }// end loop over data events

  // --------------------------------------
  // handle the monte carlo
  // --------------------------------------

  // Int_t nEv=0;; 
  std::vector<std::string> filesMC;
  GetFiles(inputMC, filesMC);
  std::cout << "Getting the files from MC " << std::endl;


  /* read in event information */
  TChain hiEventChain("hiEvtAnalyzer/HiTree");
  FillChain(hiEventChain, filesMC);
  TTreeReader hiEventReader(&hiEventChain);
  TTreeReaderValue<float>   HFpf(hiEventReader, "hiHF_pf");
  TTreeReaderValue<float>   weight(hiEventReader, "weight");
  TTreeReaderValue<float>   zVertex(hiEventReader, "vz");
  TTreeReaderValue<int>   nTrk(hiEventReader, "hiNtracks");

  /* read in filter information */
  TChain eventChain("skimanalysis/HltTree");
  FillChain(eventChain, filesMC);
  TTreeReader eventReader(&eventChain);
  TTreeReaderValue<int>   vertexFilter(eventReader, "pprimaryVertexFilter");
  TTreeReaderValue<int>   clusterFilter(eventReader, "pclusterCompatibilityFilter");

  /* read in jet information */
  TChain jetChain("akCs4PFJetAnalyzer/t");
  FillChain(jetChain, filesMC);
  TTreeReader jetReader(&jetChain);
  TTreeReaderValue<int>   jetN(jetReader, "nref");
  TTreeReaderArray<float> jetEta(jetReader, "jteta");
  TTreeReaderArray<float> jetPt(jetReader, "jtpt");
  TTreeReaderArray<float> rawPt(jetReader, "rawpt");
  TTreeReaderArray<float> genJetPt(jetReader, "refpt");
  TTreeReaderArray<float> jetRg(jetReader, "jtrg");
  TTreeReaderArray<float> genJetRg(jetReader, "refrg");
  TTreeReaderArray<float> jtPfCEF(jetReader, "jtPfCEF");
  TTreeReaderArray<float> jtPfMUF(jetReader, "jtPfMUF");
  TTreeReaderArray<float> jtPfNEF(jetReader, "jtPfNEF");
  TTreeReaderArray<int> jtPfCHM(jetReader, "jtPfCHM");

  /* read in trigger information */
  TChain trigChain("hltanalysis/HltTree");
  FillChain(trigChain, filesMC);
  TTreeReader trigReader(&trigChain);
  TTreeReaderValue<int> mb(trigReader, "HLT_MinimumBiasHF_OR_BptxAND_v1");

  /* setup the response */

  RooUnfoldResponse response;
  RooUnfoldResponse responsenotrunc;
  response.Setup(h2smeared,h2true);
  responsenotrunc.Setup(h2smearednocuts,h2fulleff);
  
  Long64_t totalEvents = jetReader.GetEntries(true);

  for (Long64_t i = 0; i < totalEvents; i++) {
    jetReader.Next(); eventReader.Next(); trigReader.Next(); hiEventReader.Next();

    // ------ step 1: Apply the necessary event filters ----
    if(*mb != 1) continue; // trigger selection
    if(*zVertex < -15.0 || *zVertex > 15.0) continue;  // z vertex filter    
    if(*vertexFilter == 0 || *clusterFilter == 0) continue; // event filters
    //------------------------------------------------------
    /* get the event weight we wil apply to each event */
    float w = *weight;

    /* now loop over jets and fill the response */
    for (int j = 0; j < *jetN; ++ j) {
      // jet kinematic and quality selections
      if (TMath::Abs(jetEta[j]) > 1.6) { continue; }
      if(jtPfCEF[j] > 0.8) continue;
      if(jtPfMUF[j] > 0.8) continue;
      if(jtPfNEF[j] > 0.9) continue;
      if(jtPfCHM[j] < 0) continue;

      // jet pT cuts at reco and gen level (test)
      if(jetPt[j] < 80  || jetPt[j] > 500) continue; // reco level
      if(genJetPt[j] < 80 || genJetPt[j] > 500) continue; // gen matched level
      if(jetRg[j] < kBinsMeasured.front() || jetRg[j] > kBinsMeasured.back()) continue; // reco level Rg
      if(genJetRg[j] < kBinsMeasured.front() || genJetRg[j] > kBinsMeasured.back()) continue; // gen level Rg
      h2fulleff->Fill(genJetRg[j],genJetPt[j],w);  
      h2smearednocuts->Fill(jetRg[j],jetPt[j],w);  
      responsenotrunc.Fill(jetRg[j],jetPt[j],genJetRg[j],genJetPt[j],w);
      
        h2smeared->Fill(jetRg[j],jetPt[j],w);
        //	h2true->Fill(ngMatch,genJetPt,scalefactor);
        response.Fill(jetRg[j],jetPt[j],genJetRg[j],genJetPt[j],w);
        h2resp->Fill(genJetRg[j], jetRg[j], w);

       
        h2true->Fill(genJetRg[j],genJetPt[j],w);
    
      
    }
  }
 

    
    TH1F *htrueptd=(TH1F*) h2fulleff->ProjectionX("trueptd",1,-1);
    TH1F *htruept=(TH1F*) h2fulleff->ProjectionY( "truept",1,-1); 
    
     


  
    //    TH2D* hfold=(TH2D*)h2raw->Clone("hfold");
    //    hfold->Sumw2();
 
 //////////efficiencies done////////////////////////////////////
//  TH1D * effok=(TH1D *)h2true->ProjectionX("effok",2,2);
//  TH1D * effok1=(TH1D *)h2fulleff->ProjectionX("effok2",2,2);
//  effok->Divide(effok1);
//  effok->SetName("correff20-40");
 
//  TH1D * effok3=(TH1D *)h2true->ProjectionX("effok3",3,3);
//   TH1D * effok4=(TH1D *)h2fulleff->ProjectionX("effok4",3,3);
//  effok3->Divide(effok4);
//   effok3->SetName("correff40-60"); 

//   TH1D * effok5=(TH1D *)h2true->ProjectionX("effok5",4,4);
//   TH1D * effok6=(TH1D *)h2fulleff->ProjectionX("effok6",4,4);
//  effok5->Divide(effok6);
//  effok5->SetName("correff60-80"); 

//    TH1D * effok7=(TH1D *)h2true->ProjectionX("effok7",5,6);
//   TH1D * effok8=(TH1D *)h2fulleff->ProjectionX("effok8",5,6);
//  effok7->Divide(effok8);
//  effok7->SetName("correff80-120"); 
 

  TFile *fout=new TFile (Form("Unfold2D_SplitMCTest_%s.root", date.c_str()),"RECREATE");

  fout->cd();
  // effok->Write(); 
  // effok3->Write();
  // effok5->Write();
  // effok7->Write();
  h2raw->SetName("raw");
  h2raw->Write();
  h2resp->Write();
  h2smeared->SetName("smeared");
  h2smeared->Write();
  htrueptd->Write();
  h2true->SetName("true");
  h2true->Write();
  for(int jar=1;jar<15;jar++){
      Int_t iter=jar;
      cout<<"iteration"<<iter<<endl;
      cout<<"==============Unfold h1====================="<<endl;

      RooUnfoldBayes   unfold(&response, h2raw, iter);    // OR
      TH2D* hunf= (TH2D*) unfold.Hreco(errorTreatment);
      //FOLD BACK
      TH1* hfold = response.ApplyToTruth (hunf, "");
        
      TH2D *htempUnf=(TH2D*)hunf->Clone("htempUnf");          
      htempUnf->SetName(Form("Bayesian_Unfoldediter%d",iter));
        
      TH2D *htempFold=(TH2D*)hfold->Clone("htempFold");          
      htempFold->SetName(Form("Bayesian_Foldediter%d",iter));        

      htempUnf->Write();
      htempFold->Write();

  }
}

#ifndef __CINT__
int main () { RooSimplepTPbPb_data2D("042326"); return 0; }  // Main program when run stand-alone
#endif
