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

/*
  RooSimplepTPbPb_split.cxx : Script to perform the split MC test.
  Hannah Bossi <hannah.bossi@cern.ch>
  4/16/2026, adapted from Yale code. 
  
  To compile run: make Execute. 
  export LD_LIBRARY_PATH=/afs/cern.ch/user/h/hbossi/RooUnfold/:$LD_LIBRARY_PATH
  ./Execute
 */


#if !(defined(__CINT__) || defined(__CLING__)) || defined(__ACLIC__)
#include <iostream>
using std::cout;
using std::endl;

#include "TRandom.h"
#include "TRandom3.h"

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

#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#endif

//==============================================================================
// Global definitions
//==============================================================================

// sets the jet radius
const double Rjet = 0.4;

//==============================================================================
// helper functions
//==============================================================================
void FillChain(TChain &chain, std::vector<std::string> &files); 
void GetFiles(char const *input, std::vector<std::string> &files); 
void Normalize2D(TH2* h); 
void RooSimplepTPbPb_split(); 
   
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




//==============================================================================
// Example Unfolding
//==============================================================================

void RooSimplepTPbPb_split(){
#ifdef __CINT__
  gSystem->Load("libRooUnfold");
#endif
  Int_t difference=1;
  Int_t Ppol=0;

  char const *input = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/Dijet_pThat-15to1200_TuneCP5_5p36TeV_pythia8/OO_MC_DijetNoEmbedding_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260318_152047/0000/";
  std::cout << "Running over " << input << std::endl;
  cout << "==================================== pick up the response matrix for background==========================" << endl;
  ///////////////////parameter setting
  RooUnfold::ErrorTreatment errorTreatment = RooUnfold::kCovariance;   
  
  TRandom3* rand = new TRandom3();

   //***************************************************

  Double_t xbins[14];
  xbins[0]=80;
  xbins[1]=95;
  xbins[2]=110;
  xbins[3]=125;
  xbins[4]=150;
  xbins[5]=175;
  xbins[6]=200;
  xbins[7]=225;
  xbins[8]=250;
  xbins[9]=275;
  xbins[10]=300;
  xbins[11] = 350; 
  xbins[12] = 400; 
  xbins[13] = 450; 
  //the raw correlation (data or psuedodata)
  TH1F *h1raw(0);
  h1raw=new TH1F("r","raw",13, xbins);
  //detector measure level (reco or hybrid MC)
  TH1F *h1smeared(0);
  h1smeared=new TH1F("smeared","smeared",13, xbins);

  //detector measure level no cuts
  TH1F *h1smearednocuts(0);
  h1smearednocuts=new TH1F("smearednocuts","smearednocuts", 13, xbins);
  //true correlations with measured cuts
  TH1F *h1true(0);
  h1true=new TH1F("true","true", 13, xbins);
  //full true correlation
  TH1F *h1fulleff(0);
  h1fulleff=new TH1F("truef","truef", 13, xbins); 

  
  TH2F *hcovariance(0);
  hcovariance=new TH2F("covariance","covariance",13,0.,1.,13,0,1.);

  TH2F *effnum=(TH2F*)h1fulleff->Clone("effnum");
  TH2F *effdenom=(TH2F*)h1fulleff->Clone("effdenom");
 
  effnum->Sumw2();
  effdenom->Sumw2();
  h1smeared->Sumw2();
  h1true->Sumw2();
  h1raw->Sumw2();
  h1fulleff->Sumw2();

  //branches in the tree that you need in this analysis
  Float_t ptJet,ptJetMatch, ptdet;

  Int_t nEv=0;; 


  std::vector<std::string> files;
  GetFiles(input, files);
  std::cout << "Get the files " << std::endl;


  /* read in event information */
  TChain hiEventChain("hiEvtAnalyzer/HiTree");
  FillChain(hiEventChain, files);
  TTreeReader hiEventReader(&hiEventChain);
  TTreeReaderValue<float>   HFpf(hiEventReader, "hiHF_pf");
  TTreeReaderValue<float>   weight(hiEventReader, "weight");
  TTreeReaderValue<float>   zVertex(hiEventReader, "vz");
  TTreeReaderValue<int>   nTrk(hiEventReader, "hiNtracks");

  /* read in filter information */
  TChain eventChain("skimanalysis/HltTree");
  FillChain(eventChain, files);
  TTreeReader eventReader(&eventChain);
  TTreeReaderValue<int>   vertexFilter(eventReader, "pprimaryVertexFilter");
  TTreeReaderValue<int>   clusterFilter(eventReader, "pclusterCompatibilityFilter");

  /* read in jet information */
  TChain jetChain("akCs4PFJetAnalyzer/t");
  FillChain(jetChain, files);
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
  FillChain(trigChain, files);
  TTreeReader trigReader(&trigChain);
  TTreeReaderValue<int> mb(trigReader, "HLT_MinimumBiasHF_OR_BptxAND_v1");

  /* setup the response */
  RooUnfoldResponse response;
  RooUnfoldResponse responsenotrunc;
  response.Setup(h1smeared,h1true);
  responsenotrunc.Setup(h1smearednocuts,h1fulleff);
  
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
      if(jtPfCHM[j] < 1) continue;

      // jet pT cuts at reco and gen level (test)
      if(jetPt[j] < 80  || jetPt[j] > 450) continue; // reco level
      if(genJetPt[j] < 80 || genJetPt[j] > 450) continue; // gen matched level
      h1fulleff->Fill(jetPt[j], w);  
      h1smearednocuts->Fill(jetPt[j],w);  
      responsenotrunc.Fill(jetPt[j],genJetPt[j],w);
      
      /* do the splitting that corresponds to the split test */
      double split = rand->Rndm();
      if (split < 0.5){
	      h1smeared->Fill(jetPt[j],w);
	      //this is the half split to be the response 
	      response.Fill(jetPt[j],genJetPt[j],w);
	    }
      else {
        //this is the psuedo data!
        h1raw->Fill(jetPt[j], w);
        //this is the generator level distribution for the pseudo data or our answer :)
        h1true->Fill(genJetPt[j],w);
	  }
    


    } // end loop over the number of jets
  } // end loop over the number of events
 

    
    TH1F *htrueptd=(TH1F*) h1fulleff->Clone("trueptd");
    TH1F *htruept=(TH1F*) h1fulleff->Clone( "truept"); 
 
    //////////efficiencies done////////////////////////////////////
 
    TFile *fout=new TFile (Form("UnfoldingSplit_R040_Test.root"),"RECREATE");
    fout->cd();
    h1raw->SetName("raw");
    h1raw->Write();
    h1smeared->SetName("smeared");
    h1smeared->Write();
    htrueptd->Write();
    h1true->SetName("true");
    h1true->Write();
    TH1D* htruth = (TH1D*)response.Htruth();
    htruth->SetName("htruth");
    htruth->Write();


    for(int jar=1;jar<10;jar++){
      Int_t iter=jar;
      cout<<"iteration"<<iter<<endl;
      cout<<"==============Unfold h1====================="<<endl;

      RooUnfoldBayes unfold(&response, h1raw, iter, false);    // OR
      TH1D* hunf= (TH1D*) unfold.Hreco();
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
int main () { RooSimplepTPbPb_split(); return 0; }  // Main program when run stand-alone
#endif
