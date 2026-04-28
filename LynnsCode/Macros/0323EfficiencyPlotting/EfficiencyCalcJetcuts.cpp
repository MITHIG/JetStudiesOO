
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
    while ((file = gSystem->GetDirEntry(dp))) {
        TString fname = file;

        // skip . and ..
        if (fname == "." || fname == "..") continue;

        if (!fname.EndsWith(".root")) continue;
        if (fname.Index(re) == kNPOS) continue;
        rootFiles.push_back((dirname + "/" + fname).Data());
        cout << "Adding file: " << (dirname + "/" + fname).Data() << endl;
    }

    gSystem->FreeDirectory(dp);

    sort(rootFiles.begin(), rootFiles.end());

    cout << "Found " << rootFiles.size() << " ROOT files matching pattern: "
         << pattern << endl;

    return rootFiles;
}
int EfficiencyCalcJetcuts() {

    string MCpthat15String = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/*";
    string MCpthat0String = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/MinBias_OO_5p36TeV_hijing/MiniumBiasOO_MC_5p36TeV_HIJING/260316_213344/0000/*";
    string DataString = "/eos/cms/store/group/phys_heavyions/hbossi/OOJetSubstructure/DataForests/IonPhysics0/OO_Data_PromptReco_IonPhsyics0/260306_195006/0000/*";
    /*************************************************************************
     *                                                                       *
     *                      TUNABLE PARAMETERS SECTION                       *
     *                                                                       *
     *                                                                       *
     *************************************************************************/
    int nevents = 0; // Set to 0 to process all events
    bool MC = false;
    bool L1MinBiasBool = true;
    bool HLTMinBiasBool = false;
    bool CCFilterBool = true;
    bool PVFilterBool = true;
    bool zvtxCutBool = true;
    bool HFEFilterBool = true;
    float jtPfCEFcut = 0.8;
    float jtPfNEFcut = 0.8;
    float jtPfMUFcut = 0.8;
    int jtPfCHMcut = 0;
    float etaCut = 1.6;
    float HFEMaxCut = 14;
    float zvtxCutValue = 15.0;

    string outfoldername = "EfficiencyFiles";
   // string ForestFolder = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/HiForestMiniAOD_5*";
    string ForestFolder = DataString;
    string outfiletag = "DataEfficiency1PD_DebugSegfault";
    /*************************************************************************
     *                                                                       *
     *                          Code here                                    *
     *                                                                       *
     *                                                                       *
     *************************************************************************/


    string cutString = "";
    string generalInfoString = "";
    string jetselstring = "";

    if (L1MinBiasBool == 1){
        cutString += "L1 MinBias Trigger;";
    }
    if (HLTMinBiasBool == 1){
        cutString += "HLT MinBias Trigger;";
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

    jetselstring += Form("Jet PF CEF < %.1f;", jtPfCEFcut);
    jetselstring += Form("Jet PF NEF < %.1f;", jtPfNEFcut);
    jetselstring += Form("Jet PF MUF < %.1f;", jtPfMUFcut);
    jetselstring += Form("Jet PF CHM > %d;", jtPfCHMcut);

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

    TChain* JetAnalyserTree = new TChain(JetAnalyserTreeString.c_str());
    TChain* SkimTree = new TChain(SkimTreeString.c_str());
    TChain* HltTree = new TChain(HltTreeString.c_str());
    TChain* PPTracksTree = new TChain(PPTracksTreeString.c_str());
    TChain* HiEvtAnalyzersTree = new TChain(HiEvtAnalyzersTreeString.c_str());
 
    // Get list of ROOT files
    vector<string> rootFiles = GetRootFiles(ForestFolder);
    cout << "Number of ROOT files to process: " << rootFiles.size() << endl;
    
    if (rootFiles.empty()) {
        cout << "ERROR: No ROOT files found in " << ForestFolder << endl;
        // Clean up TChains before returning
        delete JetAnalyserTree;
        delete SkimTree; 
        delete HltTree;
        delete PPTracksTree;
        delete HiEvtAnalyzersTree;
        return -1;
    }
    
    for (const string& file : rootFiles) {
        JetAnalyserTree->Add(file.c_str());
        HltTree->Add(file.c_str());
        SkimTree->Add(file.c_str());
        PPTracksTree->Add(file.c_str());
        HiEvtAnalyzersTree->Add(file.c_str());
        if (JetAnalyserTree->GetEntries() > nevents && nevents > 0){
            cout << "Reached event limit of " << nevents << ". Stopping file addition." << endl;
            cout << endl;
            break;
        }
    }

    cout << "Number of entries: " << JetAnalyserTree->GetEntries() << endl;
    cout << endl;

    std::ofstream out("timing.txt");
   
    const int NmaxJet = 500;
    int run, evt, lumi, nref;

    Float_t rawpt[NmaxJet];
    Float_t jtpt[NmaxJet],   jteta[NmaxJet],  jtphi[NmaxJet],  jty[NmaxJet],  jtrg[NmaxJet],  jtzg[NmaxJet],  jtkt[NmaxJet],  jtangu[NmaxJet];
    Float_t refpt[NmaxJet],  refeta[NmaxJet], refphi[NmaxJet], refy[NmaxJet], refrg[NmaxJet], refzg[NmaxJet], refkt[NmaxJet], refangu[NmaxJet];
    Float_t genpt[NmaxJet],  geneta[NmaxJet], genphi[NmaxJet], geny[NmaxJet], genrg[NmaxJet], genzg[NmaxJet], genkt[NmaxJet], genangu[NmaxJet];
    Float_t jtPfCHF[NmaxJet], jtPfNHF[NmaxJet], jtPfCEF[NmaxJet], jtPfNEF[NmaxJet], jtPfMUF[NmaxJet];
    int jtPfCHM[NmaxJet];

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
    float weight;

    //SkimFilters
    int pclusterCompatibilityFilter;
    int pprimaryVertexFilter;

    //HiEvtAnalyzer variables
    float hiHFMinus_pf;
    float hiHFEPlus_pf;
    float ptHat;

    gStyle->SetOptStat(0); // Disable statistics box

    HltTree->SetBranchStatus("*", 0); // Disable all branches
    SkimTree->SetBranchStatus("*", 0); // Disable all branches
    HiEvtAnalyzersTree->SetBranchStatus("*", 0); // Disable all branches
    PPTracksTree->SetBranchStatus("*", 0); // Disable all branches

    HltTree->SetBranchStatus("L1_MinimumBiasHF1_OR_BptxAND", 1);
    HltTree->SetBranchStatus("HLT_MinimumBiasHF_AND_BptxAND_v1", 1);
    HltTree->SetBranchStatus("L1_SingleJet60", 1);
    HltTree->SetBranchStatus("HLT_OxyL1SingleJet60_v1", 1);
    SkimTree->SetBranchStatus("pclusterCompatibilityFilter", 1);
    SkimTree->SetBranchStatus("pprimaryVertexFilter", 1);
    HiEvtAnalyzersTree->SetBranchStatus("hiHFMinus_pf", 1);
    HiEvtAnalyzersTree->SetBranchStatus("hiHFEPlus_pf", 1);
    if (MC){
        HiEvtAnalyzersTree->SetBranchStatus("pthat", 1);
        HiEvtAnalyzersTree->SetBranchStatus("weight", 1);
    }
    PPTracksTree->SetBranchStatus("nVtx", 1);
    PPTracksTree->SetBranchStatus("xVtx", 1);
    PPTracksTree->SetBranchStatus("yVtx", 1);
    PPTracksTree->SetBranchStatus("zVtx", 1);
    PPTracksTree->SetBranchStatus("xErrVtx", 1);
    PPTracksTree->SetBranchStatus("yErrVtx", 1);
    PPTracksTree->SetBranchStatus("zErrVtx", 1);
    JetAnalyserTree->SetBranchStatus("*", 0);

    JetAnalyserTree->SetBranchStatus("run", 1);
    JetAnalyserTree->SetBranchStatus("evt", 1);
    JetAnalyserTree->SetBranchStatus("lumi", 1);
    JetAnalyserTree->SetBranchStatus("nref", 1);

    JetAnalyserTree->SetBranchStatus("rawpt", 1);
    JetAnalyserTree->SetBranchStatus("jtpt", 1);
    JetAnalyserTree->SetBranchStatus("jteta", 1);
    JetAnalyserTree->SetBranchStatus("jtphi", 1);
    JetAnalyserTree->SetBranchStatus("jty", 1);
    JetAnalyserTree->SetBranchStatus("jtrg", 1);
    JetAnalyserTree->SetBranchStatus("jtzg", 1);
    JetAnalyserTree->SetBranchStatus("jtkt", 1);
    JetAnalyserTree->SetBranchStatus("jtangu", 1);

    JetAnalyserTree->SetBranchStatus("jtPfCHF", 1);
    JetAnalyserTree->SetBranchStatus("jtPfNHF", 1);
    JetAnalyserTree->SetBranchStatus("jtPfCEF", 1);
    JetAnalyserTree->SetBranchStatus("jtPfNEF", 1);
    JetAnalyserTree->SetBranchStatus("jtPfMUF", 1);
    JetAnalyserTree->SetBranchStatus("jtPfCHM", 1);

    if (MC) {
        JetAnalyserTree->SetBranchStatus("refpt", 1);
        JetAnalyserTree->SetBranchStatus("refeta", 1);
        JetAnalyserTree->SetBranchStatus("refphi", 1);
        JetAnalyserTree->SetBranchStatus("refy", 1);
        JetAnalyserTree->SetBranchStatus("refrg", 1);
        JetAnalyserTree->SetBranchStatus("refzg", 1);
        JetAnalyserTree->SetBranchStatus("refkt", 1);
        JetAnalyserTree->SetBranchStatus("refangu", 1);

        JetAnalyserTree->SetBranchStatus("genpt", 1);
        JetAnalyserTree->SetBranchStatus("geneta", 1);
        JetAnalyserTree->SetBranchStatus("genphi", 1);
        JetAnalyserTree->SetBranchStatus("geny", 1);
        JetAnalyserTree->SetBranchStatus("genrg", 1);
        JetAnalyserTree->SetBranchStatus("genzg", 1);
        JetAnalyserTree->SetBranchStatus("genkt", 1);
        JetAnalyserTree->SetBranchStatus("genangu", 1);
    }


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
    
    if(MC){
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
    }

    JetAnalyserTree->SetBranchAddress("jtPfCHF",jtPfCHF);
    JetAnalyserTree->SetBranchAddress("jtPfNHF",jtPfNHF);
    JetAnalyserTree->SetBranchAddress("jtPfCEF",jtPfCEF);
    JetAnalyserTree->SetBranchAddress("jtPfNEF",jtPfNEF);
    JetAnalyserTree->SetBranchAddress("jtPfMUF",jtPfMUF);
    JetAnalyserTree->SetBranchAddress("jtPfCHM",jtPfCHM);

    HltTree->SetBranchAddress("L1_MinimumBiasHF1_OR_BptxAND",&L1_MinimumBiasHF1_OR_BptxAND);
    HltTree->SetBranchAddress("HLT_MinimumBiasHF_AND_BptxAND_v1",&HLT_MinimumBiasHF_AND_BptxAND_v1);
    HltTree->SetBranchAddress("L1_SingleJet60",&L1_SingleJet60);
    HltTree->SetBranchAddress("HLT_OxyL1SingleJet60_v1",&HLT_OxyL1SingleJet60_v1);

    SkimTree->SetBranchAddress("pclusterCompatibilityFilter",&pclusterCompatibilityFilter);
    SkimTree->SetBranchAddress("pprimaryVertexFilter",&pprimaryVertexFilter);

    HiEvtAnalyzersTree->SetBranchAddress("hiHFMinus_pf",&hiHFMinus_pf);
    HiEvtAnalyzersTree->SetBranchAddress("hiHFEPlus_pf",&hiHFEPlus_pf);

    if(MC){
        HiEvtAnalyzersTree->SetBranchAddress("pthat",&ptHat);
        HiEvtAnalyzersTree->SetBranchAddress("weight",&weight);
    }
    else{
        weight = 1.0;
    }
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
    int totaljets = 0;
    
    // Event counting variables
    Long64_t eventsBeforeCuts = 0;
    Long64_t eventsAfterCuts = 0;
    Long64_t eventsL1MinBias = 0;
    Long64_t eventsHLTMinBias = 0;

    Long64_t eventsCCFilter = 0;
    Long64_t eventsPVFilter = 0;
    Long64_t eventsZvtxCut = 0;
    Long64_t eventsHFEFilter = 0;
    Long64_t jetsBeforeSelection = 0;
    Long64_t jetsAfterEtaCut = 0;
    Long64_t jetsAfterJetPfSelection = 0;
    Long64_t jetsGT10GeV = 0;
    Long64_t jetsGT20GeV = 0;
    Long64_t jetsGT30GeV = 0;
    Long64_t jetsGT40GeV = 0;
    Long64_t jetsGT50GeV = 0;
    Long64_t jetsGT60GeV = 0;
    Long64_t jetsGT70GeV = 0;
    Long64_t jetsGT80GeV = 0;
    Long64_t jetsGT90GeV = 0;
    Long64_t jetsGT100GeV = 0;
    Long64_t jetsGT120GeV = 0;
    Long64_t jetsGT140GeV = 0;
    Long64_t jetsGT200GeV = 0;

    auto start_time = std::chrono::high_resolution_clock::now();
    cout << "Starting processing..." << endl;
    for (Long64_t entrynum = 0; entrynum < nEntries; entrynum++){
        SkimTree->GetEntry(entrynum);
        HltTree->GetEntry(entrynum);
        JetAnalyserTree->GetEntry(entrynum);
        PPTracksTree->GetEntry(entrynum);
        HiEvtAnalyzersTree->GetEntry(entrynum);
        totaljets += nref;
        eventsBeforeCuts++;

        if (entrynum % 1000000 == 0){
            // Get current memory usage
            struct rusage current_usage;
            getrusage(RUSAGE_SELF, &current_usage);
            double current_memory_mb = current_usage.ru_maxrss / 1024.0;
            
            // Calculate memory increase from start
            static double initial_memory_mb = 0;
            if (entrynum == 0) {
                initial_memory_mb = current_memory_mb;
            }
            double memory_increase = current_memory_mb - initial_memory_mb;
            
            cout << endl;
            cout << "Processing entry " << entrynum << " out of " << nEntries << "; " << (float)entrynum/nEntries*100 << "%" << endl;
            cout << "Number of Jets in this event: " << nref << endl;
            cout << "Total jets processed so far: " << totaljets << endl;
            cout << "Events that passed cuts so far: " << eventsAfterCuts << endl;
            cout << "Current memory usage: " << current_memory_mb << " MB (+" << memory_increase << " MB from start)" << endl;
            cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count() << " seconds" << endl;
            out << "Entry: " << entrynum << ", Memory (MB): " << current_memory_mb << ", Time Elapsed (s): " << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count() << endl;
            
            if(MC){
                cout << "pthat: " << ptHat << "; weight: " << weight << endl;
            }
            cout << endl;
        }

        if (L1MinBiasBool == 1){
            if (L1_MinimumBiasHF1_OR_BptxAND != 1) continue;
        }
        eventsL1MinBias++;
        
        if (HLTMinBiasBool == 1){
            if (HLT_MinimumBiasHF_AND_BptxAND_v1 != 1) continue;
        }
        eventsHLTMinBias++;
                
        if (CCFilterBool == 1){
            if (pclusterCompatibilityFilter != 1) continue;
        }
        eventsCCFilter++;
        
        if (PVFilterBool == 1){
            if (pprimaryVertexFilter != 1) continue;
        }
        eventsPVFilter++;
        
        if (zvtxCutBool == 1){
            if (nvtx > 0 && zVtx && zVtx->size() > 0 && TMath::Abs(zVtx->at(0)) > zvtxCutValue) continue;
        }
        eventsZvtxCut++;
        
        if (HFEFilterBool == 1){
            if (hiHFMinus_pf < HFEMaxCut || hiHFEPlus_pf < HFEMaxCut) continue;
        }
        eventsHFEFilter++;
        eventsAfterCuts++;
        // Safety check for nref bounds
        if (nref > 50) {
            cout << "WARNING: nref (" << nref << ") exceeds array bounds (50). Limiting to 50." << endl;
            nref = 50;
        }

        for (int j = 0; j < nref; j++){
            jetsBeforeSelection++;
            if (TMath::Abs(jteta[j]) > etaCut) continue;
            jetsAfterEtaCut++;
            if (jtPfCEF[j] > jtPfCEFcut) continue;
            if (jtPfNEF[j] > jtPfNEFcut) continue;
            if (jtPfMUF[j] > jtPfMUFcut) continue;
            if (jtPfCHM[j] < 0) continue; 
            jetsAfterJetPfSelection++;

            if (jtpt[j] < 10) continue;
            jetsGT10GeV++;
            if (jtpt[j] < 20) continue;
            jetsGT20GeV++;
            if (jtpt[j] < 30) continue;
            jetsGT30GeV++;
            if (jtpt[j] < 40) continue;
            jetsGT40GeV++;
            if (jtpt[j] < 50) continue;
            jetsGT50GeV++;
            if (jtpt[j] < 60) continue;
            jetsGT60GeV++;
            if (jtpt[j] < 70) continue;
            jetsGT70GeV++;
            if (jtpt[j] < 80) continue;
            jetsGT80GeV++;
            if (jtpt[j] < 90) continue;
            jetsGT90GeV++;
            if (jtpt[j] < 100) continue;
            jetsGT100GeV++;
        }

        if (entrynum > nevents && nevents != 0){
            cout << Form("Debug mode: stopping after %d entries.", nevents) << endl;
            break;
        }
    }
    // Split into event/jet labels and values, then merge back for existing output logic
    std::vector<std::string> eventlabels = {
        "NEvents Before Cuts",
        "L1 MinBias Trigger",
        "HLT MinBias Trigger",
        "Cluster Compatibility Filter",
        "Primary Vertex Filter",
        "z-vertex Cut",
        "HF E_{Max} Filter"
    };

    std::vector<Long64_t> eventvalues = {
        eventsBeforeCuts,
        eventsL1MinBias,
        eventsHLTMinBias,
        eventsCCFilter,
        eventsPVFilter,
        eventsZvtxCut,
        eventsHFEFilter
    };

    std::vector<std::string> jetlabels = {
        "Jets Before Selection",
        "Jets After Eta Cut",
        ">10 GeV",
        ">20 GeV",
        ">30 GeV",
        ">40 GeV",
        ">50 GeV",
        ">60 GeV",
        ">70 GeV",
        ">80 GeV",
        ">90 GeV",
        ">100 GeV"
    };

    std::vector<Long64_t> jetvalues = {
        jetsBeforeSelection,
        jetsAfterEtaCut,
        jetsGT10GeV,
        jetsGT20GeV,
        jetsGT30GeV,
        jetsGT40GeV,
        jetsGT50GeV,
        jetsGT60GeV,
        jetsGT70GeV,
        jetsGT80GeV,
        jetsGT90GeV,
        jetsGT100GeV
    };


    cout << "\n=== PROCESSING SUMMARY ===" << endl;
    cout << "Events before cuts: " << eventsBeforeCuts << endl;
    cout << "Events after cuts: " << eventsAfterCuts << endl;
    cout << "Event cut efficiency: " << (double)eventsAfterCuts/eventsBeforeCuts*100.0 << "%" << endl;
    cout << "Jets before selection: " << jetsBeforeSelection << endl;
    cout << "Total jets processed: " << totaljets << endl;
    cout << "=========================" << endl;


    out.close();

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
    cout << "Total processing time (H:M:S): " << total_time_hms << endl; double events_per_second = nEntries / total_seconds;
    cout << "Events processed per second: " << events_per_second << " Hz" << endl;
    cout << "Average time per event: " << (total_seconds * 1000.0) / nEntries << " ms" << endl;
    cout << "=======================" << endl;
    // Print memory usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    cout << "\n=== MEMORY SUMMARY ===" << endl;
    cout << "Peak memory usage: " << usage.ru_maxrss / 1024.0 << " MB" << endl;
    cout << "Memory cleanup completed." << endl;
    cout << "=======================" << endl;

    TFile* outFile = new TFile(Form("/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/%s/EfficiencyResults_%s.root", outfoldername.c_str(), outfiletag.c_str()), "RECREATE");
    TTree *outtree = new TTree("t", "t");

    outFile->cd();
    TNamed* cutInfo = new TNamed("CutInfo", cutString.c_str());
    cutInfo->Write();
    outtree->Branch("EventCutLabels", &eventlabels);
    outtree->Branch("EventCutValues", &eventvalues);
    outtree->Branch("JetCutLabels", &jetlabels);
    outtree->Branch("JetCutValues", &jetvalues);
    outtree->Fill();
    outtree->Write();

    delete outtree;
    delete cutInfo;

    outFile->Close();
    delete outFile;

    return 0;
}

