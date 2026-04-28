#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include "RooUnfold.h"
#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include <TStyle.h>
#include <TColor.h>


template <typename T>
void CheckFileExist(T* obj, const std::string& filename = "") {
    if (!obj) {
        cout << "Error: Could not open file" << filename << endl;
        exit(1);
    }
}

int PlotTestDebug(){
    gStyle->SetOptStat(0);

    string MCFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0428DebugTrivial/Response_MC_1.2MEvts_MC_10Files_NewBin_debug_20260428.root";
    TFile *MCFilef = TFile::Open(MCFile.c_str(), "READ");
    CheckFileExist(MCFilef, MCFile);
    TH1F* hGenPt = (TH1F*)MCFilef->Get("hGenPt");
    CheckFileExist(hGenPt, "hGenPt");
    TH1F* hGenNoWeightPt = (TH1F*)MCFilef->Get("hGenNoWeight");
    CheckFileExist(hGenNoWeightPt, "hGenNoWeightPt");
    TH1F* hRecoNoWeightPt = (TH1F*)MCFilef->Get("hRecoNoWeight");
    CheckFileExist(hRecoNoWeightPt, "hRecoNoWeightPt");
    TH1F* hRecoPt = (TH1F*)MCFilef->Get("hRecoPt");
    CheckFileExist(hRecoPt, "hRecoPt");
    TH1F* hFakePt = (TH1F*)MCFilef->Get("hFakePt");
    CheckFileExist(hFakePt, "hFakePt");
    TH1F* hMatchedRecoPt = (TH1F*)MCFilef->Get("hMatchedRecoPt");
    CheckFileExist(hMatchedRecoPt, "hMatchedRecoPt");
    TH1F* hMatchedGenPt = (TH1F*)MCFilef->Get("hMatchedGenPt");
    CheckFileExist(hMatchedGenPt, "hMatchedGenPt");
    TH1F* hGenPt2 = (TH1F*)MCFilef->Get("hGenPt2");
    CheckFileExist(hGenPt2, "hGenPt2");
    TH1F* hGenNoWeightPt2 = (TH1F*)MCFilef->Get("hGenNoWeight2");
    CheckFileExist(hGenNoWeightPt2, "hGenNoWeight2");
    TH1F* hRecoNoWeightPt2 = (TH1F*)MCFilef->Get("hRecoNoWeight2");
    CheckFileExist(hRecoNoWeightPt2, "hRecoNoWeight2");
    TH1F* hRecoPt2 = (TH1F*)MCFilef->Get("hRecoPt2");
    CheckFileExist(hRecoPt2, "hRecoPt2");
    TH1F* hFakePt2 = (TH1F*)MCFilef->Get("hFakePt2");
    CheckFileExist(hFakePt2, "hFakePt2");
    TH1F* hMatchedRecoPt2 = (TH1F*)MCFilef->Get("hMatchedRecoPt2");
    CheckFileExist(hMatchedRecoPt2, "hMatchedRecoPt2");
    TH1F* hMatchedGenPt2 = (TH1F*)MCFilef->Get("hMatchedGenPt2");
    CheckFileExist(hMatchedGenPt2, "hMatchedGenPt2");
    RooUnfoldResponse* response_pt = (RooUnfoldResponse*)MCFilef->Get("response_pt");
    
    TH1F* hGenResponse = (TH1F*)response_pt->Htruth();
    CheckFileExist(hGenResponse, "hGenResponse");
    TH1F* hRecoResponse = (TH1F*)response_pt->Hmeasured();
    CheckFileExist(hRecoResponse, "hRecoResponse");   

    RooUnfoldBayes unfold(response_pt, hRecoResponse, 10);
    TH1F* hUnfolded_trivial = (TH1F*)unfold.Hunfold()->Clone("hUnfolded_trivial");

    cout << "Trivial Unfolding ratio (UnfoldedGen / ResponseGen) values:" << endl;

    for (int i = 1; i <= hUnfolded_trivial->GetNbinsX(); ++i) {
      float unfolded_gen = hUnfolded_trivial->GetBinContent(i);
      float gen_value = hGenResponse->GetBinContent(i);

      cout << "Bin" << i << " Ratio = " << (gen_value > 0 ? unfolded_gen / gen_value : 0) << endl;
    }

    return 0;
}
