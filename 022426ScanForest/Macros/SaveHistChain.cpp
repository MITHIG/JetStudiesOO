
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TChain.h>
#include <TStyle.h>
#include <TNamed.h>
#include <algorithm>

static void StyleHist(TH1* h,
    int color,
    int marker){
  h->SetLineColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(marker);
  h->SetLineWidth(2);
}

using namespace std;


int SaveHistChain() {

    bool MinBiasBool = true;
    bool JetTriggerBool = false;

    bool CCFilterBool = true;
    bool PVFilterBool = true;
    
    bool zvtxCutBool = true;
    bool JetPtCutBool = true;
    bool JetSelectionsBool = true;
    bool HFEFilterBool = true;

    float zvtxCutValue = 15.0;

    float jtPfCEFcut = 0.8;
    float jtPfNEFcut = 0.8;
    float jtPfMUFcut = 0.8;

    float jtptCut = 30;
    float etaCut = 1.6;
    float HFEMaxCut = 14;

    string cutString = "";
    string generalInfoString = "";
    if (MinBiasBool == 1){
        cutString += "Minimum Bias Trigger (L1 and HLT);";
    }
    if (JetTriggerBool == 1){
        cutString += "Jet60 Trigger (L1 and HLT);";
    }
    if (CCFilterBool == 1){
        cutString += "Cluster Compatibility Filter;";
    }
    if (PVFilterBool == 1){
        cutString += "Primary Vertex Filter;";
    }
    if (JetPtCutBool == 1){
        cutString += Form("Jet pT > %.1f GeV/c;", jtptCut);
        cutString += Form("Jet |eta| < %.1f;", etaCut);
    }
    if (HFEFilterBool == 1){
        cutString += Form("HF E_{Max}+ AND HF E_{Max}- > %i;", (int)HFEMaxCut);
    }

    if (JetSelectionsBool == 1){
        cutString += Form("Jet PF CEF < %.1f;", jtPfCEFcut);
        cutString += Form("Jet PF NEF < %.1f;", jtPfNEFcut);
        cutString += Form("Jet PF MUF < %.1f;", jtPfMUFcut);
    }

    string ForestFolder = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/HiForestMiniAOD_346*";
    string JetAnalyserTreeString = "akCs4PFJetAnalyzer/t";
    string HltTreeString = "hltanalysis/HltTree";
    string SkimTreeString = "skimanalysis/HltTree";
    string PPTracksTreeString = "ppTracks/trackTree";
    string HiEvtAnalyzersTreeString = "hiEvtAnalyzer/HiTree";

    TChain* JetAnalyserTree = new TChain(JetAnalyserTreeString.c_str());
    TChain* SkimTree = new TChain(SkimTreeString.c_str());
    TChain* HltTree = new TChain(HltTreeString.c_str());
    TChain* PPTracksTree = new TChain(PPTracksTreeString.c_str());
    TChain* HiEvtAnalyzersTree = new TChain(HiEvtAnalyzersTreeString.c_str());

    JetAnalyserTree->Add((ForestFolder + "*.root").c_str());
    HltTree->Add((ForestFolder + "*.root").c_str());
 //   HFAdcanaTree->Add((ForestFolder + ForestSubfolder[0] + "*.root").c_str());
    SkimTree->Add((ForestFolder + "*.root").c_str());
    PPTracksTree->Add((ForestFolder + "*.root").c_str());
    HiEvtAnalyzersTree->Add((ForestFolder + "*.root").c_str());

    cout << "Number of entries in JetAnalyserTree: " << JetAnalyserTree->GetEntries() << endl;
   // cout << "Number of entries in HFAdcanaTree: " << HFAdcanaTree->GetEntries() << endl;
    cout << "Number of entries in SkimTree: " << SkimTree->GetEntries() << endl;
    cout << "Number of entries in HltTree: " << HltTree->GetEntries() << endl;
    cout << "Number of entries in PPTracksTree: " << PPTracksTree->GetEntries() << endl;
    cout << "Number of entries in HiEvtAnalyzersTree: " << HiEvtAnalyzersTree->GetEntries() << endl;
   
    int run, evt, lumi, nref;
    Float_t rawpt[50];
    Float_t jtpt[50],   jteta[50],  jtphi[50],  jty[50],  jtrg[50],  jtzg[50],  jtkt[50],  jtangu[50];
    Float_t refpt[50],  refeta[50], refphi[50], refy[50], refrg[50], refzg[50], refkt[50], refangu[50];
    Float_t genpt[50],  geneta[50], genphi[50], geny[50], genrg[50], genzg[50], genkt[50], genangu[50];
    Float_t jtPfCHF[50], jtPfNHF[50], jtPfCEF[50], jtPfNEF[50], jtPfMUF[50];

    //Jet-level variables
    float jtpt1 = -1;
    float jtpt2 = -1;
    float refpt1 = -1;
    float refpt2 = -1;
    float genpt1 = -1;
    float genpt2 = -1;

    float A_J = 0;
    float X_J = 0;
    float dPhi = 0;
    float A_J_ref = 0;
    float X_J_ref = 0;
    float dPhi_ref = 0;
    float A_J_gen = 0;
    float X_J_gen = 0;
    float dPhi_gen = 0;
    float pi = TMath::Pi();

    //Trigger variables
    int L1_MinimumBiasHF1_OR_BptxAND;
    int HLT_MinimumBiasHF_AND_BptxAND_v1;
    int L1_SingleJet60;
    int HLT_OxyL1SingleJet60_v1;

    //SkimFilters
    int pclusterCompatibilityFilter;
    int pprimaryVertexFilter;

    //HiEvtAnalyzer variables
    float hiHFMinus_pf;
    float hiHFEPlus_pf;
    float ptHat;

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
    TH1F* hjty= new TH1F("hjty", "Jet Symmetry Distribution; Jet Symmetry; Number of Jets", 100, 0, 2);
    TH1F* hjtrg = new TH1F("hjtrg", "Jet RG Distribution; Jet RG; Number of Jets", 100, 0, 2);
    TH1F* hjtzg = new TH1F("hjtzg", "Jet Zg Distribution; Jet Zg; Number of Jets", 100, 0, 2);
    TH1F* hjtkt = new TH1F("hjtkt", "Jet Kt Distribution; Jet Kt (GeV/c); Number of Jets", 100, 0, 500);
    TH1F* hjtangu = new TH1F("hjtangu", "Jet Angularity Distribution; Jet Angularity; Number of Jets", 400, -2, 2);

    TH1F* hrefpt = new TH1F("hrefpt", "Reference Jet pT Distribution; Reference Jet pT (GeV/c); Number of Jets", 100, 0, 2000);
    TH1F* hrefeta = new TH1F("hrefeta", "Reference Jet eta Distribution; Reference Jet eta; Number of Jets", 100, -5, 5);
    TH1F* hrefphi = new TH1F("hrefphi", "Reference Jet phi Distribution; Reference Jet phi; Number of Jets", 100, -TMath::Pi(), TMath::Pi());
    TH1F* hrefy = new TH1F("hrefy", "Reference Jet y Distribution; Reference Jet y; Number of Jets", 100, -5, 5);
    TH1F* hrefrg = new TH1F("hrefrg", "Reference Jet RG Distribution; Reference Jet RG; Number of Jets", 200, 0, 2);
    TH1F* hrefzg = new TH1F("hrefzg", "Reference Jet Zg Distribution; Reference Jet Zg; Number of Jets", 100, 0, 2);
    TH1F* hrefkt = new TH1F("hrefkt", "Reference Jet Kt Distribution; Reference Jet Kt (GeV/c); Number of Jets", 100, 0, 500);
    TH1F* hrefangu = new TH1F("hrefangu", "Reference Jet Angularity Distribution; Reference Jet Angularity; Number of Jets", 400, -2, 2);

    TH1F* hgenpt = new TH1F("hgenpt", "Gen Jet pT Distribution; Gen Jet pT (GeV/c); Number of Jets", 100, 0, 2000);
    TH1F* hgeneta = new TH1F("hgeneta", "Gen Jet eta Distribution; Gen Jet eta; Number of Jets", 100, -5, 5);
    TH1F* hgenphi = new TH1F("hgenphi", "Gen Jet phi Distribution; Gen Jet phi; Number of Jets", 100, -TMath::Pi(), TMath::Pi());
    TH1F* hgeny = new TH1F("hgeny", "Gen Jet y Distribution; Gen Jet y; Number of Jets", 100, -5, 5);
    TH1F* hgenrg = new TH1F("hgenrg", "Gen Jet RG Distribution; Gen Jet RG; Number of Jets", 200, 0, 2);
    TH1F* hgenzg = new TH1F("hgenzg", "Gen Jet Zg Distribution; Gen Jet Zg; Number of Jets", 100, 0, 2);
    TH1F* hgenkt = new TH1F("hgenkt", "Gen Jet Kt Distribution; Gen Jet Kt (GeV/c); Number of Jets", 100, 0, 500);
    TH1F* hgenangu = new TH1F("hgenangu", "Gen Jet Angularity Distribution; Gen Jet Angularity; Number of Jets", 400, -2, 2);

    TH1F* hjtPfCHF = new TH1F("hjtPfCHF", "Jet PF Charged Hadron Energy Fraction Distribution; Jet PF Charged Hadron Energy Fraction; Number of Jets", 100, 0, 1);
    TH1F* hjtPfNHF = new TH1F("hjtPfNHF", "Jet PF Neutral Hadron Energy Fraction Distribution; Jet PF Neutral Hadron Energy Fraction; Number of Jets", 100, 0, 1);
    TH1F* hjtPfCEF = new TH1F("hjtPfCEF", "Jet PF Charged Em Energy Fraction Distribution; Jet PF Charged EM Energy Fraction; Number of Jets", 100, 0, 1);
    TH1F* hjtPfNEF = new TH1F("hjtPfNEF", "Jet PF Neutral EM Energy Fraction Distribution; Jet PF Neutral EM Energy Fraction; Number of Jets", 100, 0, 1);
    TH1F* hjtPfMUF = new TH1F("hjtPfMUF", "Jet PF Muon Energy Fraction Distribution; Jet PF Muon Energy Fraction; Number of Jets", 100, 0, 1);

    TH1F* hxvtx = new TH1F("hxvtx", "Primary Vertex X Position Distribution; Primary Vertex X Position (cm); Number of Events", 100, -0.5, 0.5);
    TH1F* hyvtx = new TH1F("hyvtx", "Primary Vertex Y Position Distribution; Primary Vertex Y Position (cm); Number of Events", 100, -0.5, 0.5);
    TH1F* hzvtx = new TH1F("hzvtx", "Primary Vertex Z Position Distribution; Primary Vertex Z Position (cm); Number of Events", 100, -50, 50);    

    TH1F* hxErrVtx = new TH1F("hxErrVtx", "Primary Vertex X Position Error Distribution; Primary Vertex X Position Error (cm); Number of Events", 100, 0, 0.1);
    TH1F* hyErrVtx = new TH1F("hyErrVtx", "Primary Vertex Y Position Error Distribution; Primary Vertex Y Position Error (cm); Number of Events", 100, 0, 0.1);
    TH1F* hzErrVtx = new TH1F("hzErrVtx", "Primary Vertex Z Position Error Distribution; Primary Vertex Z Position Error (cm); Number of Events", 100, 0, 0.1);

    TH2F* hpthatvjtpt = new TH2F("hpthatvjtpt", "pthat vs jetpt;#hat{p}_{T};jet p_{T}",
                   100, 0, 500,
                   100, 0, 500);

    gStyle->SetOptStat(0); // Disable statistics box
    JetAnalyserTree->SetBranchAddress("run",&run);
    JetAnalyserTree->SetBranchAddress("evt",&evt);
    JetAnalyserTree->SetBranchAddress("nref",&nref);
    JetAnalyserTree->SetBranchAddress("lumi",&lumi);

    JetAnalyserTree->SetBranchAddress("rawpt",rawpt);

    JetAnalyserTree->SetBranchAddress("jtpt",jtpt);
    JetAnalyserTree->SetBranchAddress("jteta",jteta);
    JetAnalyserTree->SetBranchAddress("jtphi",jtphi);
    JetAnalyserTree->SetBranchAddress("jty",jty);
    JetAnalyserTree->SetBranchAddress("jtrg",jtrg);
    JetAnalyserTree->SetBranchAddress("jtzg",jtzg);
    JetAnalyserTree->SetBranchAddress("jtkt",jtkt);
    JetAnalyserTree->SetBranchAddress("jtangu",jtangu);
    
    JetAnalyserTree->SetBranchAddress("refpt",refpt);
    JetAnalyserTree->SetBranchAddress("refeta",refeta);
    JetAnalyserTree->SetBranchAddress("refphi",refphi);
    JetAnalyserTree->SetBranchAddress("refrg",refrg);
    JetAnalyserTree->SetBranchAddress("refzg",refzg);
    JetAnalyserTree->SetBranchAddress("refy",refy);
    JetAnalyserTree->SetBranchAddress("refkt",refkt);
    JetAnalyserTree->SetBranchAddress("refangu",refangu);

    JetAnalyserTree->SetBranchAddress("genpt",genpt);
    JetAnalyserTree->SetBranchAddress("geneta",geneta);
    JetAnalyserTree->SetBranchAddress("genphi",genphi);
    JetAnalyserTree->SetBranchAddress("geny",geny);
    JetAnalyserTree->SetBranchAddress("genrg",genrg);
    JetAnalyserTree->SetBranchAddress("genzg",genzg);
    JetAnalyserTree->SetBranchAddress("genkt",genkt);
    JetAnalyserTree->SetBranchAddress("genangu",genangu);

    JetAnalyserTree->SetBranchAddress("jtPfCHF",jtPfCHF);
    JetAnalyserTree->SetBranchAddress("jtPfNHF",jtPfNHF);
    JetAnalyserTree->SetBranchAddress("jtPfCEF",jtPfCEF);
    JetAnalyserTree->SetBranchAddress("jtPfNEF",jtPfNEF);
    JetAnalyserTree->SetBranchAddress("jtPfMUF",jtPfMUF);

    HltTree->SetBranchAddress("L1_MinimumBiasHF1_OR_BptxAND",&L1_MinimumBiasHF1_OR_BptxAND);
    HltTree->SetBranchAddress("HLT_MinimumBiasHF_AND_BptxAND_v1",&HLT_MinimumBiasHF_AND_BptxAND_v1);
    HltTree->SetBranchAddress("L1_SingleJet60",&L1_SingleJet60);
    HltTree->SetBranchAddress("HLT_OxyL1SingleJet60_v1",&HLT_OxyL1SingleJet60_v1);

    SkimTree->SetBranchAddress("pclusterCompatibilityFilter",&pclusterCompatibilityFilter);
    SkimTree->SetBranchAddress("pprimaryVertexFilter",&pprimaryVertexFilter);

    HiEvtAnalyzersTree->SetBranchAddress("hiHFMinus_pf",&hiHFMinus_pf);
    HiEvtAnalyzersTree->SetBranchAddress("hiHFEPlus_pf",&hiHFEPlus_pf);
    HiEvtAnalyzersTree->SetBranchAddress("pthat",&ptHat);
    
    //PPTracks variables
    int nvtx;
    vector<float>* xvtx = nullptr;
    vector<float>* yvtx = nullptr;
    vector<float>* zVtx = nullptr;

    vector<float>* xErrVtx = nullptr;
    vector<float>* yErrVtx = nullptr;
    vector<float>* zErrVtx = nullptr;

    PPTracksTree->SetBranchAddress("nVtx",&nvtx);
    PPTracksTree->SetBranchAddress("xVtx",&xvtx);
    PPTracksTree->SetBranchAddress("yVtx",&yvtx);
    PPTracksTree->SetBranchAddress("zVtx",&zVtx);
    PPTracksTree->SetBranchAddress("xErrVtx",&xErrVtx);
    PPTracksTree->SetBranchAddress("yErrVtx",&yErrVtx);
    PPTracksTree->SetBranchAddress("zErrVtx",&zErrVtx);

    Long64_t nEntries = PPTracksTree->GetEntries();

    for (Long64_t entrynum = 0; entrynum < nEntries; entrynum++){
        SkimTree->GetEntry(entrynum);
        HltTree->GetEntry(entrynum);
        JetAnalyserTree->GetEntry(entrynum);
        SkimTree->GetEntry(entrynum);
        PPTracksTree->GetEntry(entrynum);
        HiEvtAnalyzersTree->GetEntry(entrynum);

        if (entrynum % 1000 == 0){
            cout << "Processing entry " << entrynum << " out of " << nEntries << endl;
        }

        if (MinBiasBool == 1){
            if (L1_MinimumBiasHF1_OR_BptxAND != 1 || HLT_MinimumBiasHF_AND_BptxAND_v1 != 1) continue;
        }
        if (JetTriggerBool == 1){
            if (L1_SingleJet60 != 1 || HLT_OxyL1SingleJet60_v1 != 1) continue;
        }
        if (CCFilterBool == 1){
            if (pclusterCompatibilityFilter != 1) continue;
        }
        if (PVFilterBool == 1){
            if (pprimaryVertexFilter != 1) continue;
        }
        if (zvtxCutBool == 1){
            if (TMath::Abs(zVtx->at(0)) > zvtxCutValue) continue;
        }
        if (HFEFilterBool == 1){
            if (hiHFMinus_pf < HFEMaxCut || hiHFEPlus_pf < HFEMaxCut) continue;
        }
        

        for (int j = 0; j < nref; j++){
            if (JetPtCutBool == 1){
                if (jtpt[j] < jtptCut) continue;
                if (TMath::Abs(jteta[j]) > etaCut) continue;
            }
            if (JetSelectionsBool == 1){
                if (jtPfCEF[j] > jtPfCEFcut) continue;
                if (jtPfNEF[j] > jtPfNEFcut) continue;
                if (jtPfMUF[j] > jtPfMUFcut) continue;
            }


            hjtpt->Fill(jtpt[j]);
            hjteta->Fill(jteta[j]);
            hjtphi->Fill(jtphi[j]);
            hjtrg->Fill(jtrg[j]);
            hjtzg->Fill(jtzg[j]);
            hjtkt->Fill(jtkt[j]);
            hjtangu->Fill(jtangu[j]);
            hjty->Fill(jty[j]);
    

            hrefpt->Fill(refpt[j]);
            hrefeta->Fill(refeta[j]);
            hrefphi->Fill(refphi[j]);
            hrefrg->Fill(refrg[j]);
            hrefzg->Fill(refzg[j]);
            hrefy->Fill(refy[j]);
            hrefkt->Fill(refkt[j]);
            hrefangu->Fill(refangu[j]);

            hgenpt->Fill(genpt[j]);
            hgeneta->Fill(geneta[j]);
            hgenphi->Fill(genphi[j]);
            hgeny->Fill(geny[j]);
            hgenrg->Fill(genrg[j]);
            hgenzg->Fill(genzg[j]);
            hgeny->Fill(geny[j]);
            hgenangu->Fill(genangu[j]);

            hjtPfCEF->Fill(jtPfCEF[j]);
            hjtPfNEF->Fill(jtPfNEF[j]);
            hjtPfMUF->Fill(jtPfMUF[j]);
            hjtPfNHF->Fill(jtPfNHF[j]);
            hjtPfCHF->Fill(jtPfCHF[j]);
        }

        for (int vtx = 0; vtx < nvtx; vtx++){
            hxvtx->Fill(xvtx->at(vtx));
            hyvtx->Fill(yvtx->at(vtx));
            hzvtx->Fill(zVtx->at(vtx));
            hxErrVtx->Fill(xErrVtx->at(vtx));
            hyErrVtx->Fill(yErrVtx->at(vtx));
            hzErrVtx->Fill(zErrVtx->at(vtx));
        }

        A_J = 0;
        X_J = 0;

        A_J_ref = 0;
        X_J_ref = 0;   

        dPhi_ref = 0;
        dPhi = 0;

        if (nref <= 2) continue;

        vector<int> idx(nref);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int i1, int i2) {
            return jtpt[i1] > jtpt[i2];
        });

        jtpt1 = jtpt[idx[0]];
        jtpt2 = jtpt[idx[1]];
        if (jtpt2 > jtpt1){
            cout << "Warning: jtpt2 is greater than jtpt1 for entry " << entrynum << ". Swapping values." << endl;
            swap(jtpt1, jtpt2);
        }

        float phi1 = jtphi[idx[0]];
        float phi2 = jtphi[idx[1]];
        dPhi = phi1 - phi2;
        if (pi <= dPhi && dPhi <= 2 * pi){
            dPhi = (dPhi - 2*pi);
        }
        else if (-2*pi <= dPhi && dPhi <= -pi){
            dPhi = dPhi + 2*pi;
        }
        dPhi = TMath::Abs(dPhi);

        if (dPhi < 0.1){
            cout << "dPhi: " << dPhi << " for entry " << entrynum << endl;
            cout << "jtphi1: " << phi1 << ", jtphi2: " << phi2 << endl;
            cout << "jtpt1: " << jtpt1 << ", jtpt2: " << jtpt2 << endl;
        }

        A_J = (jtpt1-jtpt2)/(jtpt1+jtpt2);
        X_J = jtpt2/jtpt1;
        
        vector<int> refidx(nref);
        iota(refidx.begin(), refidx.end(), 0);
        sort(refidx.begin(), refidx.end(), [&](int i1, int i2) {
            return refpt[i1] > refpt[i2];
        });

        refpt1 = refpt[refidx[0]];
        refpt2 = refpt[refidx[1]];

        if (refpt2 > refpt1){
            cout << "Warning: refpt2 is greater than refpt1" << endl;
            swap(refpt1,refpt2);
        }

        A_J_ref = (refpt1-refpt2)/(refpt1+refpt2);
        X_J_ref = refpt2/refpt1;

        float phi1_ref = refphi[refidx[0]];
        float phi2_ref = refphi[refidx[1]];
        dPhi_ref = phi1_ref - phi2_ref;

        if (pi <= dPhi_ref && dPhi_ref <= 2 * pi){
            dPhi_ref = (dPhi_ref - 2*pi);
        }
        else if (-2*pi <= dPhi_ref && dPhi_ref <= -pi){
            dPhi_ref = dPhi_ref + 2*pi;
        }
        dPhi_ref = TMath::Abs(dPhi_ref);
    
        
        hjtpt1->Fill(jtpt1);
        hjtpt2->Fill(jtpt2);
        hAj->Fill(A_J);
        hXj->Fill(X_J);
        hAjref->Fill(A_J_ref);
        hXjref->Fill(X_J_ref);
        hdPhiref->Fill(dPhi_ref);
        hdPhi->Fill(dPhi);
        hpthatvjtpt->Fill(ptHat, jtpt1);

    }
    /* delete JetAnalyserTree;
    delete HFAdcanaTree;
    delete SkimTree;
    delete PPTracksTree;*/

    cout << "Cuts applied: " << cutString << endl;
    TNamed* cutInfo = new TNamed("Cuts",cutString.c_str());

    TFile* outFile = new TFile("../RootFiles/HistTFiles/JetSubstructureHistogramChain_031026.root", "RECREATE");
    cutInfo->Write();

    outFile->mkdir("JetLevelHistRaw");
    outFile->mkdir("EventLevelHistRaw");
    outFile->mkdir("TrackLevelHistRaw");

    outFile->cd("TrackLevelHistRaw");
    hxvtx->Write();
    hyvtx->Write();
    hzvtx->Write();
    hxErrVtx->Write();
    hyErrVtx->Write();
    hzErrVtx->Write();

    outFile->cd("JetLevelHistRaw");
    hjtpt->Write();
    hjteta->Write();
    hjtphi->Write();
    hjty->Write();
    hjtrg->Write();
    hjtzg->Write();
    hjtkt->Write();
    hjtangu->Write();
    hrefpt->Write();
    hrefeta->Write();
    hrefphi->Write();
    hrefy->Write();
    hrefrg->Write();
    hrefzg->Write();
    hrefkt->Write();
    hrefangu->Write();
    hgenpt->Write();
    hgeneta->Write();
    hgenphi->Write();
    hgeny->Write();
    hgenrg->Write();
    hgenzg->Write();
    hgenkt->Write();
    hgenangu->Write();
    hjtPfCHF->Write();
    hjtPfNHF->Write();
    hjtPfCEF->Write();
    hjtPfNEF->Write();
    hjtPfMUF->Write();

    hpthatvjtpt->Write();

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

