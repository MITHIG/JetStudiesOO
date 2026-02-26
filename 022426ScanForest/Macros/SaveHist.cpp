
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
#include <TH1F.h>

struct HistVar1D{
	std::string histTitle = "Hist Title";
	std::string xLabel = "xLabel";
    std::string yLabel = "yLabel";
    std::string outFolderName = "Plots/";
    std::string outFileName = "Unknown.png";
    int nbin = 50;
    float xmin = 0;
    float xmax = TMath::Pi()+0.0001;
    float ymin = 0;
    float ymax = 100;
};

static void StyleHist(TH1* h, int color, int marker){
  h->SetLineColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(marker);
  h->SetLineWidth(2);
}


using namespace std;


int SaveHist() {
    string ForestFolder = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/Dijet_pThat-15to1200_TuneCP5_5p36TeV_pythia8/OO_MC_Dijet_pThat-15to1200_TuneCP5_5p36TeV_pythia8//";
    string ForestSubfolder[] = {"260215_030015/0000/"};
    string JetAnalyserTreeString = "ak0PFJetAnalyzer/t";
    string HFAdcanalyserTreeString = "HFAdcana/adc";
    string SkimTreeString = "skimanalysis/HltTree";
    string inclusiveJetSubstructureString = "inclusiveJetSubstructure/t";

    int run, evt, lumi, nref;
    Float_t jtpt[50], jteta[50], jtphi[50], jtsym[50], jtrg[50], jtdynkt[50], jtangu[50];
    Float_t refpt[50], refeta[50], refphi[50], refsym[50], refrg[50], refdynkt[50], refangu[50];

    float jtpt1 = -1;
    float jtpt2 = -1;
    float refpt1 = -1;
    float refpt2 = -1;
    float A_J = 0;
    float X_J = 0;
    float dPhi = 0;
    float A_J_ref = 0;
    float X_J_ref = 0;
    float dPhi_ref = 0;
    float pi = TMath::Pi();

    
    TH1F* hjtpt1 = new TH1F("hjtpt1", "Leading Jet pT Distribution; Leading Jet pT (GeV/c); Number of Jets", 100, 0, 2000);
    TH1F* hjtpt2 = new TH1F("hjtpt2", "Subleading Jet pT Distribution; Subleading Jet pT (GeV/c); Number of Jets", 100, 0, 2000);    
    TH1F* hAj = new TH1F("hAj", "Jet Asymmetry Distribution; A_{J}; Number of Jets", 100, -2, 2);    
    TH1F* hXj = new TH1F("hXj", "Jet Momentum Balance Distribution; X_{J}; Number of Jets", 100, -2, 2);
    TH1F* hAjref = new TH1F("hAjref", "Reference Jet Asymmetry Distribution; A_{J} (Reference); Number of Jets", 100, -2, 2);
    TH1F* hXjref = new TH1F("hXjref", "Reference Jet Momentum Balance Distribution; X_{J} (Reference); Number of Jets", 100, -2, 2);
    TH1F* hdPhiref = new  TH1F("hdPhiref", "Reference Delta Phi Distribution; #Delta#phi (Reference); Number of Jets", 100, 0, TMath::Pi());

    TH1F* hdPhi = new TH1F("hdPhi", "Delta Phi Distribution; #Delta#phi; Number of Jets", 100, 0, TMath::Pi());
    TH1F* hjtpt = new TH1F("hjtpt", "Jet pT Distribution; Jet pT (GeV/c); Number of Jets", 100, 0, 2000);
    TH1F* hjteta = new TH1F("hjteta", "Jet eta Distribution; Jet eta; Number of Jets", 100, -5, 5);
    TH1F* hjtphi = new TH1F("hjtphi", "Jet phi Distribution; Jet phi; Number of Jets", 100, -TMath::Pi(), TMath::Pi());
    TH1F* hjtsym = new TH1F("hjtsym", "Jet Symmetry Distribution; Jet Symmetry; Number of Jets", 100, 0, 2);
    TH1F* hjtrg = new TH1F("hjtrg", "Jet RG Distribution; Jet RG; Number of Jets", 200, 0, 2);
    TH1F* hjtdynkt = new TH1F("hjtdynkt", "Jet DynKt Distribution; Jet DynKt (GeV/c); Number of Jets", 100, 0, 500);
    TH1F* hjtangu = new TH1F("hjtangu", "Jet Angularity Distribution; Jet Angularity; Number of Jets", 400, -2, 2);

    TH1F* hrefpt = new TH1F("hrefpt", "Reference Jet pT Distribution; Reference Jet pT (GeV/c); Number of Jets", 100, 0, 2000);
    TH1F* hrefeta = new TH1F("hrefeta", "Reference Jet eta Distribution; Reference Jet eta; Number of Jets", 100, -5, 5);
    TH1F* hrefphi = new TH1F("hrefphi", "Reference Jet phi Distribution; Reference Jet phi; Number of Jets", 100, -TMath::Pi(), TMath::Pi());
    TH1F* hrefsym = new TH1F("hrefsym", "Reference Jet Symmetry Distribution; Reference Jet Symmetry; Number of Jets", 200, 0, 2);
    TH1F* hrefrg = new TH1F("hrefrg", "Reference Jet RG Distribution; Reference Jet RG; Number of Jets", 200, 0, 2);
    TH1F* hrefdynkt = new TH1F("hrefdynkt", "Reference Jet DynKt Distribution; Reference Jet DynKt (GeV/c); Number of Jets", 100, 0, 500);
    TH1F* hrefangu = new TH1F("hrefangu", "Reference Jet Angularity Distribution; Reference Jet Angularity; Number of Jets", 400, -2, 2);    

    gStyle->SetOptStat(0); // Disable statistics box
    for(string subfolder : ForestSubfolder) {
        string inputFileFolder = ForestFolder + subfolder;
        cout << "Processing subfolder: " << subfolder << endl;
        for(int i : {2,3,26,27,28,32}){
            string inputFileName = inputFileFolder + Form("HiForestMiniAOD_%i.root",i);
            cout << inputFileName << endl;
            cout << "Processing file: " << i << endl;

            TFile* inFile = new TFile(inputFileName.c_str(),"READ");
            if (!inFile || inFile->IsZombie()) {
                cout << "Error: Could not open the file!" << endl;
            }

            TTree* inclusiveJetSubstructureTree = (TTree*)inFile->Get(inclusiveJetSubstructureString.c_str());

            inclusiveJetSubstructureTree->SetBranchAddress("run",&run);
            inclusiveJetSubstructureTree->SetBranchAddress("evt",&evt);
            inclusiveJetSubstructureTree->SetBranchAddress("nref",&nref);
            inclusiveJetSubstructureTree->SetBranchAddress("lumi",&lumi);
            inclusiveJetSubstructureTree->SetBranchAddress("jtpt",jtpt);
            inclusiveJetSubstructureTree->SetBranchAddress("jteta",jteta);
            inclusiveJetSubstructureTree->SetBranchAddress("jtphi",jtphi);
            inclusiveJetSubstructureTree->SetBranchAddress("jtsym",jtsym);
            inclusiveJetSubstructureTree->SetBranchAddress("jtrg",jtrg);
            inclusiveJetSubstructureTree->SetBranchAddress("jtdynkt",jtdynkt);
            inclusiveJetSubstructureTree->SetBranchAddress("jtangu",jtangu);
            inclusiveJetSubstructureTree->SetBranchAddress("refpt",refpt);
            inclusiveJetSubstructureTree->SetBranchAddress("refeta",refeta);
            inclusiveJetSubstructureTree->SetBranchAddress("refphi",refphi);
            inclusiveJetSubstructureTree->SetBranchAddress("refsym",refsym);
            inclusiveJetSubstructureTree->SetBranchAddress("refrg",refrg);
            inclusiveJetSubstructureTree->SetBranchAddress("refdynkt",refdynkt);
            inclusiveJetSubstructureTree->SetBranchAddress("refangu",refangu);

            Long64_t nEntries = inclusiveJetSubstructureTree->GetEntries();

            for (Long64_t entrynum = 0; entrynum < nEntries; entrynum++){
                inclusiveJetSubstructureTree->GetEntry(entrynum);
                A_J = 0;
                X_J = 0;
                A_J_ref = 0;
                X_J_ref = 0;   
                dPhi_ref = 0;
                dPhi = 0;

                if (nref <= 2) continue;

                jtpt1 = jtpt[0];
                jtpt2 = jtpt[1];

		sort(refpt,refpt+nref,greater<float>());

                refpt1 = refpt[0];
                refpt2 = refpt[1];

                if (jtpt2 > jtpt1){
                    cout << "Warning: jtpt2 is greater than jtpt1 for entry " << entrynum << ". Swapping values." << endl;
                    swap(jtpt1, jtpt2);
                }

		if (refpt2 > refpt1){
			cout << "Warning: refpt2 is greater than refpt1" << endl;
			swap(refpt1,refpt2);
		}

                A_J = (jtpt1-jtpt2)/(jtpt1+jtpt2);
                A_J_ref = (refpt1-refpt2)/(refpt1+refpt2);
                X_J = jtpt2/jtpt1;
                X_J_ref = refpt2/refpt1;

                float phi1 = jtphi[0];
                float phi2 = jtphi[1];
                float phi1_ref = refphi[0];
                float phi2_ref = refphi[1];
                dPhi = phi1 - phi2;
                dPhi_ref = phi1_ref - phi2_ref;

                if (pi <= dPhi && dPhi <= 2 * pi){
                    dPhi = (dPhi - 2*pi);
                }
                else if (-2*pi <= dPhi && dPhi <= -pi){
                    dPhi = dPhi + 2*pi;
                }

                if (pi <= dPhi_ref && dPhi_ref <= 2 * pi){
                    dPhi_ref = (dPhi_ref - 2*pi);
                }
                else if (-2*pi <= dPhi_ref && dPhi_ref <= -pi){
                    dPhi_ref = dPhi_ref + 2*pi;
                }
                dPhi_ref = TMath::Abs(dPhi_ref);
                dPhi = TMath::Abs(dPhi);

                hjtpt1->Fill(jtpt1);
                hjtpt2->Fill(jtpt2);
                hAj->Fill(A_J);
                hXj->Fill(X_J);
                hAjref->Fill(A_J_ref);
                hXjref->Fill(X_J_ref);
                hdPhiref->Fill(dPhi_ref);
                hdPhi->Fill(dPhi);

                for (int j = 0; j < nref; j++){
                    hjtpt->Fill(jtpt[j]);
                    hjteta->Fill(jteta[j]);
                    hjtphi->Fill(jtphi[j]);
                    hjtsym->Fill(jtsym[j]);
                    hjtrg->Fill(jtrg[j]);
                    hjtdynkt->Fill(jtdynkt[j]);
                    hjtangu->Fill(jtangu[j]);

                    hrefpt->Fill(refpt[j]);
                    hrefeta->Fill(refeta[j]);
                    hrefphi->Fill(refphi[j]);
                    hrefsym->Fill(refsym[j]);
                    hrefrg->Fill(refrg[j]);
                    hrefdynkt->Fill(refdynkt[j]);
                    hrefangu->Fill(refangu[j]);
                }
                
            }
           /* delete JetAnalyserTree;
            delete HFAdcanaTree;
            delete SkimTree;
            delete PPTracksTree;*/
        }
    }

    TFile* outFile = new TFile("../RootFiles/HistTFiles/JetSubstructureHistogramNocuts.root", "RECREATE");
    outFile->mkdir("JetLevelHistRaw");
    outFile->mkdir("EventLevelHistRaw");

    outFile->cd("JetLevelHistRaw");
    hjtpt->Write();
    hjteta->Write();
    hjtphi->Write();
    hjtsym->Write();
    hjtrg->Write();
    hjtdynkt->Write();
    hjtangu->Write();
    hrefpt->Write();
    hrefeta->Write();
    hrefphi->Write();
    hrefsym->Write();
    hrefrg->Write();
    hrefdynkt->Write();
    hrefangu->Write();

    outFile->cd("EventLevelHistRaw");

    hjtpt1->Write();
    hjtpt2->Write();

    hAjref->Write();
    hXjref->Write();
    hdPhiref->Write();
    hAj->Write();
    hXj->Write();
    hdPhi->Write();
        
    return 0;
}
