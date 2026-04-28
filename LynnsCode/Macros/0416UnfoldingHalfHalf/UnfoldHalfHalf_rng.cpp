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
#include <ctime>
#include <TSystemDirectory.h>
#include <TList.h>
#include <TSystemFile.h> 
#include <chrono>
#include <sys/resource.h>
#include <TSystem.h>        // Add this line
#include <TString.h>        // Add this line too for TString class
#include <TRegexp.h>        // Add this line for TRegexp class
#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include <TRandom3.h>

using namespace std;

// Function to get list of ROOT files from a directory
vector<string> GetRootFiles(const string& pattern) {
    vector<string> rootFiles;

    TString dirname = gSystem->DirName(pattern.c_str());
    TString basename = gSystem->BaseName(pattern.c_str());

    void* dp = gSystem->OpenDirectory(dirname);
    if (!dp) {
        cout << "Error: cannot open directory " << dirname << endl;
        return rootFiles;
    }

    // wildcard matcher (compatible with older ROOT)
    TRegexp re(basename, true);

    const char* file;
    cout << "Scanning directory: " << dirname << " for files matching pattern: " << basename << endl;
    while ((file = gSystem->GetDirEntry(dp))) {
        TString fname = file;

        // skip . and ..
        if (fname == "." || fname == "..") continue;

        if (!fname.EndsWith(".root")) continue;
        if (fname.Index(re) == kNPOS) continue;
        rootFiles.push_back((dirname + "/" + fname).Data());
    }

    gSystem->FreeDirectory(dp);

    sort(rootFiles.begin(), rootFiles.end());

    cout << "Found " << rootFiles.size() << " ROOT files matching pattern: "
         << pattern << endl;

    return rootFiles;
}

int GetBin(double x, const double* bins, int nBins) {
    if (x < bins[0] || x >= bins[nBins]) return -1;

    for (int i = 0; i < nBins; i++) {
        if (x >= bins[i] && x < bins[i+1])
            return i + 1;  // ROOT bins are 1-based
    }
    return -1;
}

std::string FormatNEntriesLabel(int inputNum)
{

    string nentriesLabel;
    if (inputNum >= 1000000) {
        if (inputNum % 1000000 == 0) {
            nentriesLabel = Form("%iM", inputNum / 1000000);
        } else {
            nentriesLabel = Form("%.1fM", inputNum / 1000000.0);
        }
    } else if (inputNum >= 1000) {
        if (inputNum % 1000 == 0) {
            nentriesLabel = Form("%iK", inputNum / 1000);
        } else {
            nentriesLabel = Form("%.1fK", inputNum / 1000.0);
        }
    } else {
        nentriesLabel = Form("%d", inputNum);
    }
    return nentriesLabel;
}

void GenerateLogBins(){
    int nBins = 6;
    double xMin = 30.0;
    double xMax = 1500.0;

    std::vector<double> ptBins(nBins + 1);

    double logMin = std::log10(xMin);
    double logMax = std::log10(xMax);

    for (int i = 0; i <= nBins; i++) {
        double x = logMin + i * (logMax - logMin) / nBins;
        ptBins[i] = std::pow(10, x);
    }
    cout << "Logarithmic bins: ";
    for (const auto& bin : ptBins) {
        cout << TMath::Nint(bin) << " ";
    }
    cout << endl;
}

int main() {

    string MCpthat15String = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/*";
    string MCpthat0String = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/MinBias_OO_5p36TeV_hijing/MiniumBiasOO_MC_5p36TeV_HIJING/260316_213344/0000/*";
    string DataString = "/eos/cms/store/group/phys_heavyions/hbossi/OOJetSubstructure/DataForests/IonPhysics0/OO_Data_PromptReco_IonPhsyics0/260306_195006/0000/*";
    /*************************************************************************
     *                                                                       *
     *             TUNABLE PARAMETERS SECTION                                 *
     *                                                                       *
     *                                                                       *
     *************************************************************************/
    int nEventsCut = 10; // Set to 0 to process all events
    int startfilenum = 0; // Set to 0 to start from the first file, or a positive integer to skip files
    int fileCut = -1; // Set to -1 to process all files, or a positive integer to limit number of files
    bool MC = true;
    string outfoldername = "0428DebugTrivial/";
    string ForestFolder;
    if (MC) {
        ForestFolder = MCpthat15String;
    }
    else {
        ForestFolder = DataString;  
    }
    string outfiletag = "MC_10Files_NewBin_debug";
    bool L1MinBiasBool = true;
    bool HLTMinBiasBool = false;
    bool JetTriggerBool = false;
    bool CCFilterBool = true;
    bool PVFilterBool = true;
    bool zvtxCutBool = true;
    bool HFEFilterBool = true;

    float zvtxCutValue = 15.0;

    float jtPfCEFcut = 0.8;
    float jtPfNEFcut = 0.8;
    float jtPfMUFcut = 0.8;
    int jtPfCHMcut = 0;

    float jtptUpperCut = 450;
    float jtptCut = 80;
    float etaCut = 1.6;
    float HFEMaxCut = 14;
    /*************************************************************************
     *                                                                       *
     *                          Code here                                    *
     *                                                                       *
     *                                                                       *
     *************************************************************************/
    string cutString = "";
    string generalInfoString = "";

    int nMiss = 0;
    int nFake = 0;
    int nMatch = 0;

    if (L1MinBiasBool == 1){
        cutString += "L1 MinBias Trigger;";
    }
    if (HLTMinBiasBool == 1){
        cutString += "HLT MinBias Trigger;";
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
    if (HFEFilterBool == 1){
        cutString += Form("HF E_{Max}+ AND HF E_{Max}- > %i;", (int)HFEMaxCut);
    }
    cutString += Form("Jet PF CEF < %.1f;", jtPfCEFcut);
    cutString += Form("Jet PF NEF < %.1f;", jtPfNEFcut);
    cutString += Form("Jet PF MUF < %.1f;", jtPfMUFcut);
     cutString += Form("Jet PF CHM > %d;", jtPfCHMcut);
    cutString += Form("Match iff Reco&Gen %.1f < pt < %.1f GeV/c;", jtptCut, jtptUpperCut);
    cutString += Form("Jet |eta| < %.1f;", etaCut);

    string sampleType = MC ? "MC" : "Data";

    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "The system clock is currently at " << std::ctime(&t_c);
    time_t t = time(nullptr);
    struct tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
    char datebuf[64];
    strftime(datebuf, sizeof(datebuf), "%Y%m%d", &tm);
    std::string date(datebuf);

    string JetAnalyserTreeString = "akCs4PFJetAnalyzer/t";
    string HltTreeString = "hltanalysis/HltTree";
    string SkimTreeString = "skimanalysis/HltTree";
    string PPTracksTreeString = "ppTracks/trackTree";
    string HiEvtAnalyzersTreeString = "hiEvtAnalyzer/HiTree";
    // Get list of ROOT files
    vector<string> rootFiles = GetRootFiles(ForestFolder);

    cout << "Number of ROOT files to process: " << rootFiles.size() << endl;
    Long64_t eventsBeforeCuts = 0;
    Long64_t eventsAfterCuts = 0;
    Long64_t jetsBeforeSelection = 0;
    Long64_t totaljets = 0;
    int nEvents = 0;

    double ptBins[] = {80, 
                        95,
                        110, 
                        125,
                        150,
                        175,
                        200,
                        250,
                        350,
                        450};
 //   double ptBins[] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,450,500,600,700,800,900,1000,1100,1200,1300,1400,1500};
    const int nPtBins = sizeof(ptBins)/sizeof(double) - 1;


    TH1::SetDefaultSumw2();
    TH1F* hGenPt = new TH1F("hGenPt", "Gen", nPtBins, ptBins);
    TH1F* hRecoPt  = new TH1F("hRecoPt",  "Reco",  nPtBins, ptBins);
    TH1F* hFakePt  = new TH1F("hFakePt",  "Fake", nPtBins, ptBins);
    TH1F* hMissPt  = new TH1F("hMissPt",  "Missed", nPtBins, ptBins);

    TH1F* hMatchedRecoPt = new TH1F("hMatchedRecoPt", "Matched Reco", nPtBins, ptBins);
    TH1F* hMatchedGenPt = new TH1F("hMatchedGenPt", "Matched Gen", nPtBins, ptBins);
    RooUnfoldResponse response_pt(hRecoPt, hGenPt);
    
    TH1F* hGenNoWeight = new TH1F("hGenNoWeight", "Gen No Weight", nPtBins, ptBins);
    TH1F* hRecoNoWeight = new TH1F("hRecoNoWeight", "Reco No Weight", nPtBins, ptBins);

    TH1F* hGenPt2 = new TH1F("hGenPt2", "Gen 2", nPtBins, ptBins);
    TH1F* hRecoPt2  = new TH1F("hRecoPt2",  "Reco 2",  nPtBins, ptBins);
    TH1F* hFakePt2  = new TH1F("hFakePt2",  "Fake 2", nPtBins, ptBins);
    TH1F* hMissPt2  = new TH1F("hMissPt2",  "Missed 2", nPtBins, ptBins);
    TH1F* hMatchedRecoPt2 = new TH1F("hMatchedRecoPt2", "Matched Reco 2", nPtBins, ptBins);
    TH1F* hMatchedGenPt2 = new TH1F("hMatchedGenPt2", "Matched Gen 2", nPtBins, ptBins);
    TH1F* hGenNoWeight2 = new TH1F("hGenNoWeight2", "Gen No Weight 2", nPtBins, ptBins);
    TH1F* hRecoNoWeight2 = new TH1F("hRecoNoWeight2", "Reco No Weight 2", nPtBins, ptBins);

    hGenPt->Sumw2();
    hRecoPt->Sumw2();
    
    auto start_time = std::chrono::high_resolution_clock::now();

    TRandom3 rng(12345); // fixed seed for reproducibility
    int TrainCounter = 0;
    int TestCounter = 0;
     for (size_t filenum = startfilenum; filenum < rootFiles.size(); filenum++) {
            if (fileCut > 0 && filenum >= (size_t)fileCut) {
                cout << "Reached file limit of " << fileCut << ". Stopping." << endl;
                break;
            }
        std::string path = rootFiles[filenum];
        std::string eosFileName = "root://eoscms.cern.ch/" + path;
        cout << "Processing file " << (filenum + 1) << " of " << rootFiles.size() << ": " << rootFiles[filenum] << endl;
        cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count() << " seconds" << endl;
        TFile *file = TFile::Open(eosFileName.c_str());
        if (!file || file->IsZombie()) {
            cout << "Error: cannot open file " << eosFileName << endl;
            continue;
        }

        TTree *HltTree = (TTree*)file->Get(HltTreeString.c_str());
        TTree *SkimTree = (TTree*)file->Get(SkimTreeString.c_str());
        TTree *HiEvtAnalyzersTree = (TTree*)file->Get(HiEvtAnalyzersTreeString.c_str());
        TTree *PPTracksTree = (TTree*)file->Get(PPTracksTreeString.c_str());
        TTree *JetAnalyserTree = (TTree*)file->Get(JetAnalyserTreeString.c_str());

        int run, evt, lumi, nref, ngen;
        Float_t rawpt[50];
        Float_t jtpt[50],   jteta[50],  jtphi[50],  jty[50],  jtrg[50],  jtzg[50],  jtkt[50],  jtangu[50];
        Float_t refpt[50],  refeta[50], refphi[50], refy[50], refrg[50], refzg[50], refkt[50], refangu[50];
        Float_t genpt[50],  geneta[50], genphi[50], geny[50], genrg[50], genzg[50], genkt[50], genangu[50];
        Float_t jtPfCHF[50], jtPfNHF[50], jtPfCEF[50], jtPfNEF[50], jtPfMUF[50];
        int jtPfCHM[50];
        Int_t genmatchindex[25];   //[ngen]

        //Trigger variables
        int L1_MinimumBiasHF1_OR_BptxAND;
        int HLT_MinimumBiasHF_AND_BptxAND_v1;
        int L1_SingleJet60;
        int HLT_OxyL1SingleJet60_v1;
        float weight;

        //SkimFilters
        int pclusterCompatibilityFilter;
        int pprimaryVertexFilter;

        //HiEvtAnalyzer variables
        float hiHFMinus_pf;
        float hiHFEPlus_pf;
        float ptHat;

        //vtx
        int nvtx;
        vector<float>* xvtx = nullptr;
        vector<float>* yvtx = nullptr;
        vector<float>* zVtx = nullptr;

        vector<float>* xErrVtx = nullptr;
        vector<float>* yErrVtx = nullptr;
        vector<float>* zErrVtx = nullptr;

        HltTree->SetBranchStatus("*", 0); // Disable all branches
        SkimTree->SetBranchStatus("*", 0); // Disable all branches
        HiEvtAnalyzersTree->SetBranchStatus("*", 0); // Disable all branches
        PPTracksTree->SetBranchStatus("*", 0); // Disable all branches

        auto Enable = [](TTree* t, const vector<string>& branches){
            for (auto& b : branches) t->SetBranchStatus(b.c_str(), 1);
        };
        
        Enable(HltTree, {"L1_MinimumBiasHF1_OR_BptxAND", "HLT_MinimumBiasHF_AND_BptxAND_v1", "L1_SingleJet60", "HLT_OxyL1SingleJet60_v1"});
        Enable(SkimTree, {"pclusterCompatibilityFilter", "pprimaryVertexFilter"});
        Enable(HiEvtAnalyzersTree, {"hiHFMinus_pf", "hiHFEPlus_pf"});
        Enable(PPTracksTree, {"nVtx", "xVtx", "yVtx", "zVtx", "xErrVtx", "yErrVtx", "zErrVtx"});
        Enable(JetAnalyserTree, {"run", "evt", "lumi", "nref", "rawpt", "jtpt", "jteta", "jtphi", "jty", "jtrg", "jtzg", "jtkt", "jtangu",
                                "jtPfCHF", "jtPfNHF", "jtPfCEF", "jtPfNEF", "jtPfMUF", "jtPfCHM"});
        if (MC){
            Enable(JetAnalyserTree, {"ngen", "refpt", "refeta", "refphi", "refy", "refrg", "refzg", "refkt", "refangu",
                                     "genpt", "geneta", "genphi", "geny", "genrg", "genzg", "genkt", "genangu", "genmatchindex"});
            Enable(HiEvtAnalyzersTree, {"pthat", "weight"});
        }

        HltTree->SetBranchAddress("L1_MinimumBiasHF1_OR_BptxAND",&L1_MinimumBiasHF1_OR_BptxAND);
        HltTree->SetBranchAddress("HLT_MinimumBiasHF_AND_BptxAND_v1",&HLT_MinimumBiasHF_AND_BptxAND_v1);
        HltTree->SetBranchAddress("L1_SingleJet60",&L1_SingleJet60);
        HltTree->SetBranchAddress("HLT_OxyL1SingleJet60_v1",&HLT_OxyL1SingleJet60_v1);

        SkimTree->SetBranchAddress("pclusterCompatibilityFilter",&pclusterCompatibilityFilter);
        SkimTree->SetBranchAddress("pprimaryVertexFilter",&pprimaryVertexFilter);

        HiEvtAnalyzersTree->SetBranchAddress("hiHFMinus_pf",&hiHFMinus_pf);
        HiEvtAnalyzersTree->SetBranchAddress("hiHFEPlus_pf",&hiHFEPlus_pf);
        HiEvtAnalyzersTree->SetBranchAddress("pthat",&ptHat);
        HiEvtAnalyzersTree->SetBranchAddress("weight",&weight);
       
        PPTracksTree->SetBranchAddress("nVtx",&nvtx);
        PPTracksTree->SetBranchAddress("xVtx",&xvtx);
        PPTracksTree->SetBranchAddress("yVtx",&yvtx);
        PPTracksTree->SetBranchAddress("zVtx",&zVtx);
        PPTracksTree->SetBranchAddress("xErrVtx",&xErrVtx);
        PPTracksTree->SetBranchAddress("yErrVtx",&yErrVtx);
        PPTracksTree->SetBranchAddress("zErrVtx",&zErrVtx);

        JetAnalyserTree->SetBranchAddress("run",&run);
        JetAnalyserTree->SetBranchAddress("evt",&evt);
        JetAnalyserTree->SetBranchAddress("lumi",&lumi);
        JetAnalyserTree->SetBranchAddress("nref",&nref);
        JetAnalyserTree->SetBranchAddress("rawpt",&rawpt);
        JetAnalyserTree->SetBranchAddress("jtpt",&jtpt);
        JetAnalyserTree->SetBranchAddress("jteta",&jteta);
        JetAnalyserTree->SetBranchAddress("jtphi",&jtphi);
        JetAnalyserTree->SetBranchAddress("jty",&jty);
        JetAnalyserTree->SetBranchAddress("jtrg",&jtrg);
        JetAnalyserTree->SetBranchAddress("jtzg",&jtzg);
        JetAnalyserTree->SetBranchAddress("jtkt",&jtkt);
        JetAnalyserTree->SetBranchAddress("jtangu",&jtangu);
        JetAnalyserTree->SetBranchAddress("jtPfCHF",&jtPfCHF);
        JetAnalyserTree->SetBranchAddress("jtPfNHF",&jtPfNHF);
        JetAnalyserTree->SetBranchAddress("jtPfCEF",&jtPfCEF);
        JetAnalyserTree->SetBranchAddress("jtPfNEF",&jtPfNEF);
        JetAnalyserTree->SetBranchAddress("jtPfMUF",&jtPfMUF);
        JetAnalyserTree->SetBranchAddress("jtPfCHM",&jtPfCHM);
        if (MC){
            JetAnalyserTree->SetBranchAddress("ngen",&ngen);
            JetAnalyserTree->SetBranchAddress("refpt",&refpt);
            JetAnalyserTree->SetBranchAddress("refeta",&refeta);
            JetAnalyserTree->SetBranchAddress("refphi",&refphi);
            JetAnalyserTree->SetBranchAddress("refy",&refy);
            JetAnalyserTree->SetBranchAddress("refrg",&refrg);
            JetAnalyserTree->SetBranchAddress("refzg",&refzg);
            JetAnalyserTree->SetBranchAddress("refkt",&refkt);
            JetAnalyserTree->SetBranchAddress("refangu",&refangu);
            JetAnalyserTree->SetBranchAddress("genpt",&genpt);
            JetAnalyserTree->SetBranchAddress("geneta",&geneta);
            JetAnalyserTree->SetBranchAddress("genphi",&genphi);
            JetAnalyserTree->SetBranchAddress("geny",&geny);
            JetAnalyserTree->SetBranchAddress("genrg",&genrg);
            JetAnalyserTree->SetBranchAddress("genzg",&genzg);
            JetAnalyserTree->SetBranchAddress("genkt",&genkt);
            JetAnalyserTree->SetBranchAddress("genangu",&genangu);
            JetAnalyserTree->SetBranchAddress("genmatchindex",&genmatchindex);
        }

        Long64_t nEntries = PPTracksTree->GetEntries();        
        for (Long64_t entrynum = 0; entrynum < nEntries; entrynum++){
            SkimTree->GetEntry(entrynum);
            HltTree->GetEntry(entrynum);
            JetAnalyserTree->GetEntry(entrynum);
            PPTracksTree->GetEntry(entrynum);
            HiEvtAnalyzersTree->GetEntry(entrynum);

            totaljets += nref;
            eventsBeforeCuts++;
            nEvents++;
            if (L1MinBiasBool == 1){
                if (L1_MinimumBiasHF1_OR_BptxAND != 1) continue;
            }
            if (HLTMinBiasBool == 1){
                if (HLT_MinimumBiasHF_AND_BptxAND_v1 != 1) continue;
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
                if (zVtx->size() > 0 && TMath::Abs(zVtx->at(0)) > zvtxCutValue) continue;
            }
            if (HFEFilterBool == 1){
                if (hiHFMinus_pf < HFEMaxCut || hiHFEPlus_pf < HFEMaxCut) continue;
            }
            eventsAfterCuts++;

            bool isTrain = (rng.Uniform() > 0.5);

            if (isTrain) TrainCounter++;
            else TestCounter++;

            for (int j = 0; j < nref; j++){
                jetsBeforeSelection++;
                if (TMath::Abs(jteta[j]) > etaCut) continue;
                if (TMath::Abs(refeta[j]) > etaCut) continue;
               // bool JetSelectionPass = (jtPfCEF[j] < jtPfCEFcut) && (jtPfNEF[j] < jtPfNEFcut) && (jtPfMUF[j] < jtPfMUFcut) && (jtPfCHM[j] > jtPfCHMcut);
                //if (!JetSelectionPass) continue;
                bool recoPass = jtpt[j] > jtptCut && jtpt[j] < jtptUpperCut;
                bool genPass  = refpt[j] > jtptCut && refpt[j] < jtptUpperCut;
                bool JetQualityPass = (jtPfCEF[j] < jtPfCEFcut) && (jtPfNEF[j] < jtPfNEFcut) && (jtPfMUF[j] < jtPfMUFcut) && (jtPfCHM[j] > jtPfCHMcut);
                if (!JetQualityPass) continue;
                if (recoPass) {
                    if (isTrain) hRecoPt->Fill(jtpt[j], weight);
                    else hRecoPt2->Fill(jtpt[j], weight);
                    if (isTrain) hRecoNoWeight->Fill(jtpt[j]);
                    else hRecoNoWeight2->Fill(jtpt[j]);
                }
                if (recoPass && genPass) {
                    nMatch++;
                    if (isTrain) {
                        hMatchedRecoPt->Fill(jtpt[j], weight);
                        hMatchedGenPt->Fill(refpt[j], weight);
                        response_pt.Fill(jtpt[j], refpt[j], weight);
                    }
                    else {
                        hMatchedRecoPt2->Fill(jtpt[j], weight);
                        hMatchedGenPt2->Fill(refpt[j], weight);
                    }
                   // cout << "✓ MATCHED: " << Form(" reco pt: %14.2f    |  gen pt:  %14.2f", jtpt[j], refpt[j]) << endl;
                } else if (recoPass && !genPass) {
                    nFake++;
                   // cout << "✓ FAKE: " << Form(" reco pt: %14.2f    |    gen pt: %14.2f", jtpt[j], refpt[j]) << endl;
                    if (isTrain) hFakePt->Fill(jtpt[j], weight);
                    else hFakePt2->Fill(jtpt[j], weight);
                }
            }
            for (int j = 0; j < ngen; j++){
                 if (TMath::Abs(geneta[j]) > etaCut) continue;
                int idx = genmatchindex[j];
                bool hasMatch = (idx >= 0);
                bool genPass = genpt[j] > jtptCut && genpt[j] < jtptUpperCut;
                bool JetQualityPass = false;
                if (hasMatch) {
                    if (TMath::Abs(jteta[idx]) > etaCut) continue;
                    if (TMath::Abs(refeta[idx]) > etaCut) continue;
                    JetQualityPass = (jtPfCEF[idx] < jtPfCEFcut) && (jtPfNEF[idx] < jtPfNEFcut) && (jtPfMUF[idx] < jtPfMUFcut) && (jtPfCHM[idx] > jtPfCHMcut);
                }
                if (!JetQualityPass) continue;

                if (genPass) {
                    if (isTrain) hGenPt->Fill(genpt[j], weight);
                    else hGenPt2->Fill(genpt[j], weight);

                    if (isTrain) hGenNoWeight->Fill(genpt[j]);
                    else hGenNoWeight2->Fill(genpt[j]);
                }

                if ((genPass && hasMatch && (jtpt[idx] <= jtptCut || jtpt[idx] >= jtptUpperCut)) || (genPass && genmatchindex[j] < 0)) {
                    if (isTrain) {
                        hMissPt->Fill(genpt[j], weight);
                    } else {
                        hMissPt2->Fill(genpt[j], weight);
                    }
                   /* if (genmatchindex[j] >= 0) {
                        cout << "✓ MISSED: " << Form("reco pt %14.2f | gen pt: %14.2f ", jtpt[idx], genpt[j]) << endl;
                    } else {
                        cout << "✓ MISSED: " << Form("reco pt: No Match | gen pt: %14.2f ", genpt[j]) << endl;
                    }*/
                    nMiss++;
                }
            }

            if (nEvents < nEventsCut && nEventsCut != 0){
                cout << Form("Debug mode: stopping after %d entries.", nEventsCut) << endl;
                break;
            }
        }
        if (nEvents > nEventsCut && nEventsCut != 0){
            cout << Form("Debug mode: stopping after %d entries.", nEventsCut) << endl;
            break;
        }

        cout << "File Contains: " << nEntries << " entries." << endl;
        cout << "NMiss: " << nMiss << " | ";
        cout << "NFake: " << nFake << " | ";
        cout << "NMatch: " << nMatch << endl;

        file->Close();
        delete file;
        file = nullptr;
    }

    cout << "\n=== PROCESSING SUMMARY ===" << endl;
    cout << "NMatches: " << nMatch << endl;
    cout << "NMisses: " << nMiss << endl;
    cout << "NFakes: " << nFake << endl;
    cout << "Train Counter: " << TrainCounter << endl;
    cout << "Test Counter: " << TestCounter << endl;
    cout << "=======================" << endl;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    double total_seconds = duration.count() / 1000.0;

    // compute hours, minutes, seconds for a human-friendly display
    int hours = 0;
    int minutes = 0;
    double seconds = total_seconds;
    if (total_seconds > 0.0) {
        hours = static_cast<int>(total_seconds) / 3600;
        minutes = (static_cast<int>(total_seconds) % 3600) / 60;
        seconds = total_seconds - hours * 3600 - minutes * 60;
    }
    std::string total_time_hms = Form("%dh %dm %.2fs", hours, minutes, seconds);
    cout << "\n=== TIMING SUMMARY ===" << endl;
    cout << "Total processing time: " << total_time_hms << endl;
    double events_per_second = nEvents / total_seconds;
    cout << "Events processed per second: " << events_per_second << " Hz" << endl;
    cout << "Average time per event: " << (total_seconds * 1000.0) / nEvents << " ms" << endl;
    cout << "=======================" << endl;
    
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    cout << "\n=== MEMORY SUMMARY ===" << endl;
    cout << "Peak memory usage: " << usage.ru_maxrss / 1024.0 << " MB" << endl;
    cout << "Memory cleanup completed." << endl;
    cout << "=======================" << endl;

    TNamed* cutInfo = new TNamed("Cuts",cutString.c_str());

    // produce human-friendly label: 1K, 1.5K, 1M, 2.3M, or "All"
    long long nE = static_cast<long long>(nEvents); // ensure wide enough for formatting
    string nentriesLabel = FormatNEntriesLabel(nE);
    generalInfoString += Form("NEntries: %s;", nentriesLabel.c_str());
    generalInfoString += Form("Sample: %s;", sampleType.c_str());
    generalInfoString += Form("Tag: %s;", outfiletag.c_str());
    generalInfoString += Form("EventsBeforeCuts: %lld;", eventsBeforeCuts);
    generalInfoString += Form("EventsAfterCuts: %lld;", eventsAfterCuts);
    generalInfoString += Form("EventCutEfficiency: %.2f%%;", (double)eventsAfterCuts/eventsBeforeCuts*100.0);
    generalInfoString += Form("TotalJetsProcessed: %lld;", totaljets);
    generalInfoString += Form("TotalProcessingTime: %s;", total_time_hms.c_str());
    generalInfoString += Form("NMiss: %d;", nMiss);
    generalInfoString += Form("NFake: %d;", nFake);
    generalInfoString += Form("NMatch: %d;", nMatch);
    if(MC) {
        generalInfoString += Form("MC: Yes;");
    } else {
        generalInfoString += Form("MC: No;");
    }
    
    // Add individual cut efficiencies if cuts were applied
    
    TNamed* generalInfo = new TNamed("GeneralInfo", generalInfoString.c_str());
    cout << "Cuts applied: " << cutString << endl;
    cout << "General info: " << generalInfoString << endl;

    string outFileName = Form("/home/xirong/JetStudiesOO/LynnsCode/RootFiles/%s/Response_%s_%sEvts_%s_%s.root",
                                outfoldername.c_str(), sampleType.c_str(), nentriesLabel.c_str(), outfiletag.c_str(), date.c_str());

    TFile* outFile = new TFile(outFileName.c_str(), "RECREATE");
    
    cutInfo->Write();
    generalInfo->Write();

        hGenPt->Write();
        hGenNoWeight->Write();
        hRecoPt->Write();
        hRecoNoWeight->Write();
        hMatchedRecoPt->Write();
        hMatchedGenPt->Write();
        hFakePt->Write();
        hMissPt->Write();
        response_pt.Write("response_pt");

        hGenPt2->Write();
        hGenNoWeight2->Write();
        hRecoPt2->Write();
        hRecoNoWeight2->Write();
        hMatchedRecoPt2->Write();
        hMatchedGenPt2->Write();
        hFakePt2->Write();
        hMissPt2->Write();

    cout << "Output written to: " << outFileName << endl;
    delete outFile;
    return 0;
}

