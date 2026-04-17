// fill2025jet.C: quick macro to play with high pT jet sample from 2025 

#include <string>
#include <vector>
#include <iostream>



// list of functions
void GetFiles(char const *input, vector<string> &files); 
void FillChain(TChain &chain, vector<string> &files); 
int GetCentBin(float hiHFval); 
void fill2025jets(); 

// get the files 
void GetFiles(char const *input, vector<string> &files) {
  TSystemDirectory dir(input, input);
  TList *list = dir.GetListOfFiles();

  if (list) {
    TSystemFile *file;
    string fname;
    TIter next(list);
    while ((file = (TSystemFile *)next())) {
      fname = file->GetName();

      if (file->IsDirectory() && (fname.find(".") == string::npos)) {
        string newDir = string(input) + fname + "/";
        GetFiles(newDir.c_str(), files);
      } else if ((fname.find(".root") != string::npos)) {
        files.push_back(string(input) + fname);
        cout << files.back() << endl;
      }
    }
  }

  return;
}

// fill the tchain
void FillChain(TChain &chain, vector<string> &files) {
  for (auto file : files) {
    chain.Add(file.c_str());
  }
}


// main function
void fill2025jets(){

    //char const *input = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/QCD-dijet_pThat15-event-weighted_TuneCP5_5p36TeV_pythia8/OO_MC_DijetEmbedded_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260306_002843/0000/";
    char const *input = "/eos/cms/store/group/phys_heavyions/hbossi/mc_productions/Dijet_pThat-15to1200_TuneCP5_5p36TeV_pythia8/OO_MC_DijetNoEmbedding_pThat-15to1200_TuneCP5_5p36TeV_pythia8/260318_152047/0000/";

    std::cout << "Running over " << input << std::endl; 

    /* read in all files in the input folder */
    vector<string> files;
    GetFiles(input, files);
    std::cout << "Done getting the files " << std::endl; 


    const double Rjet = 0.4; 

    /* read in event information */
    TChain hiEventChain("hiEvtAnalyzer/HiTree"); 
    FillChain(hiEventChain, files);
    TTreeReader hiEventReader(&hiEventChain);
    TTreeReaderValue<float>   HFpf(hiEventReader, "hiHF_pf");
    TTreeReaderValue<float>   weight(hiEventReader, "weight"); 
    TTreeReaderValue<float>   zVertex(hiEventReader, "vz");
    TTreeReaderValue<int>   nTrk(hiEventReader, "hiNtracks");  

        std::cout << "Done with hiEvtAnalyzer" << std::endl; 

    /* read in filter information */
    TChain eventChain("skimanalysis/HltTree"); 
    FillChain(eventChain, files);
    TTreeReader eventReader(&eventChain);
    TTreeReaderValue<int>   vertexFilter(eventReader, "pprimaryVertexFilter");
    TTreeReaderValue<int>   clusterFilter(eventReader, "pclusterCompatibilityFilter"); 


    std::cout << "Done with skimanalysis" << std::endl; 


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

    std::cout << "Done with akCs4PFJetAnalyzer" << std::endl; 

    /* read in trigger information */
    TChain trigChain("hltanalysis/HltTree");
    FillChain(trigChain, files);
    TTreeReader trigReader(&trigChain);
    TTreeReaderValue<int> mb(trigReader, "HLT_MinimumBiasHF_OR_BptxAND_v1");

    std::cout << "Done with hltanalysis" << std::endl; 


    /* declare the histograms */
    TH1D* hJetPt      = new TH1D("hJetPt", "", 250, 0, 500); 
    TH1D* hRefJetPt   = new TH1D("hRefJetPt", "", 250, 0, 500); 
    TH1D* hJetRg      = new TH1D("hJetRg", "", 100, 0, 1.0); 
    TH1D* hRefJetRg   = new TH1D("hRefJetRg", "", 100, 0, 1.0); 
    TH2D* hJetPerf    = new TH2D("hJetPerf", "", 250, 0, 500, 50, 0.0, 2.0); 
    TH1D* hHiHF       = new TH1D("hHiHF_1D", "", 200, 0, 600); 
    TH2D* hHiHFvsnTrk = new TH2D("hHiHFvsnTrk", "", 200, 0, 600, 100, 0, 100); 
    TH1D* hCent       = new TH1D("hCent", "", 100, 0, 100); 
    TH2D* hJetPerfRaw    = new TH2D("hJetPerfRaw", "", 250, 0, 500, 50, 0.0, 2.0); 

    std::vector<int> centBins = {0, 10, 30, 50, 100}; 
    std::vector<TH2D*> hJetPerfHists; 
    for(int c = 0; c < centBins.size()-1; c++){
      TH2D* hJetPerfCent = new TH2D(Form("hJetPerf_cent_%d_%d", centBins.at(c), centBins.at(c+1)), "", 250, 0, 500, 50, 0.0, 2.0); 
      hJetPerfHists.push_back(hJetPerfCent);
    }
    Long64_t totalEvents = 1000000;// jetReader.GetEntries(true)*0.05;

    std::cout << " ---> Total number of events to process is " << totalEvents << std::endl;

    /* read in information from TTrees */
    for (Long64_t i = 0; i < totalEvents; i++) {
        jetReader.Next(); eventReader.Next(); trigReader.Next(); hiEventReader.Next(); 

        if (i % 20000 == 0) { 
            cout << "Entry: " << i << " / " <<  totalEvents << endl; 
        }
        
        // trigger selection
        if(*mb != 1) continue; 

        // z vertex filter
        if(*zVertex < -15.0 || *zVertex > 15.0) continue; 

        // event filters
        if(*vertexFilter == 0 || *clusterFilter == 0) continue; 

        float w = *weight; 

        int cent = GetCentBin(*HFpf); 
        
        // skip events with centrality outside of the range
        if(cent < 0)continue; 

        // now fill the centrality and/or event histograms 
        hCent->Fill(cent,w); 
        hHiHF->Fill(*HFpf, w); 
        hHiHFvsnTrk->Fill(*HFpf, *nTrk, w); 

        //find the cent bin
        int centBinIndex = -999; 
        for(int c = 0; c < centBins.size()-1; c++){
          if(cent >= centBins.at(c) && cent <= centBins.at(c+1)){
            centBinIndex = c; 
            break; 
          }
       }

       if(centBinIndex < 0) std::cout << "Something went wrong: " << cent << std::endl;

        float maxJetPt = -999;
        float maxJetPhi = -999;
        float maxJetEta = -999;

        int size = genJetRg.GetSize();
        if(size != *jetN)std::cout << "size of Rg: " << size << " size of nRef " << *jetN << std::endl;

        /* iterate through jets and find the jet with max pT */
        for (int j = 0; j < *jetN; ++ j) {
          // jet kinematic and quality selections 
          if (TMath::Abs(jetEta[j]) > 1.6) { continue; }
          if(jetRg[j] < 1e-5 ||  genJetRg[j] < 1e-5) continue;  
          if(jtPfCEF[j] > 0.8) continue; 
          if(jtPfMUF[j] > 0.8) continue; 
          if(jtPfNEF[j] > 0.9) continue; 
          if(jtPfCHM[j] < 1) continue; 


          // now fill the histograms
          hJetPt->Fill(jetPt[j], w); 
          hRefJetPt->Fill(genJetPt[j], w);
      
          hJetRg->Fill(jetRg[j]/Rjet, w);
          hRefJetRg->Fill(genJetRg[j]/Rjet, w);
          hJetPerf->Fill(genJetPt[j],jetPt[j]/ genJetPt[j], w); 
          hJetPerfRaw->Fill(genJetPt[j],rawPt[j]/ genJetPt[j], w); 
          hJetPerfHists.at(centBinIndex)->Fill(genJetPt[j],jetPt[j]/ genJetPt[j], w);
        
        
        }
        if (i % 20000 == 0) { 
            cout << "Here after loop for event: " << i << " / " <<  totalEvents << endl; 
        }

      
    } // end loop over the number of events

    TFile* outFile = new TFile("JetPerf_OOPYTHIA_NoEmbedded_March18th.root", "RECREATE"); 
    outFile->cd(); 
    hJetPt->Write(); 
    hRefJetPt->Write();
    hJetRg->Write();
    hRefJetRg->Write();
    hJetPerf->Write(); 
    hCent->Write(); 
    hHiHF->Write(); 
    hJetPerfRaw->Write(); 
    hHiHFvsnTrk->Write(); 
    for(int c = 0; c < centBins.size()-1; c++){
      hJetPerfHists.at(c)->Write(); 
    }


}

/* function to return the centrality bin */
int GetCentBin(float hiHFval){
  double vcent[201] ={0,0.91197,1.82394,2.73591,3.64788,4.55985,5.47182,6.38379,7.29576,8.20773,9.1197,10.0317,10.6667,10.7669,11.1084,11.4511,11.7967,12.1453,12.4985,12.8586,13.2279, 13.6047,13.9872,14.378,14.7734,15.1775,15.5875,16.0048,16.4344,16.8729,17.3249,17.7878,18.2572,18.7342,19.2173,19.7139,20.221,20.7368,21.2654,21.7955,22.3333,22.8819,23.446,24.0097,24.5881,25.179,25.7856,26.3914,27.0037,27.6247,28.2619,28.9063,29.5473,30.2005,30.8754,31.5574,32.2518,32.9558,33.6616,34.3925,35.1279,35.8711,36.618,37.3905,38.1709,38.9568,39.76,40.5757,41.3886,42.2135,43.0613,43.921,44.8002,45.6699,46.5569,47.4536,48.3673,49.2971,50.2437,51.1962,52.1656,53.1554,54.1508,55.1587,56.1668,57.1942,58.2508,59.3168,60.406,61.5058,62.6151,63.7513,64.8938,66.0488,67.2111,68.4189,69.6168,70.8422,72.0903,73.3433,74.6371,75.9344,77.2465,78.5776,79.9278,81.3019,82.7002,84.105,85.5389,86.9959,88.4836,89.9969,91.5244,93.0648,94.6373,96.2243,97.8198,99.454,101.105,102.798,104.466,106.186,107.925,109.692,111.487,113.265,115.092,116.936,118.856,120.819,122.768,124.744,126.732,128.739,130.809,132.907,135.033,137.173,139.34,141.543,143.777,146.028,148.298,150.674,153.07,155.464,157.912,160.369,162.846,165.358,167.907,170.503,173.094,175.76,178.495,181.231,183.991,186.784,189.674,192.575,195.507,198.523,201.592,204.653,207.796,210.983,214.245,217.554,220.881,224.256,227.741,231.249,234.786,238.444,242.096,245.842,249.627,253.523,257.577,261.697,265.88,270.16,274.601,279.162,283.888,288.745,293.767,298.968,304.526,310.308,316.408,322.893,329.956,337.457,345.746,354.995,365.685,378.512,395.16,420.836,526.238}; 
  int centrality = -999; 
  for(int index = 0; index < 201; index ++){
    if(hiHFval < vcent[index]){
      centrality = abs(200 - (index -1))/2; 
      break; 
    }
  }

  if(centrality < 0) std::cout << "Error in centrality determination, result = " << centrality  << " with HiHFval " << hiHFval << std::endl;

  return centrality; 
}