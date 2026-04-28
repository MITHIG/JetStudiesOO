#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>

struct BlockCaptionInfo{
  std::vector<std::string> text;
  double x;
  double y;
  double size;
  double spacing;
  bool useNDC;
};

static void DrawTLatexLines(const BlockCaptionInfo& info)
{
  TLatex lat;
  lat.SetNDC(info.useNDC);
  lat.SetTextSize(info.size);
  double yy = info.y;
  for (const auto& s : info.text) {
    lat.SetTextFont(132);  // Helvetica (normal, not bold)
    lat.DrawLatex(info.x, yy, s.c_str());
    yy -= info.spacing;
  }
}

static void DrawAndSave(const std::vector<TH1*>& hs,
                        const std::vector<std::string>& labels,
                        const std::string& outPng,
                        const std::string& xTitle,
                        const std::string& yTitle = "Normalized entries",
                        const std::string& title = "",
                        float xMin = -999, float xMax = -999,
                        float yMin = -999, float yMax = -999,
                        bool logx = false,
                        bool logy = false,
                        BlockCaptionInfo captionInfo = {}) {
  if (hs.empty()) return;

  // basic sanity on labels
  std::vector<std::string> lab = labels;
  if (lab.size() != hs.size()) {
    lab.resize(hs.size());
    for (size_t i = 0; i < hs.size(); ++i) {
      if (i < labels.size()) lab[i] = labels[i];
      else lab[i] = std::string("hist") + std::to_string(i+1);
    }
  }

  TCanvas* c = new TCanvas("c", "canvas", 800, 800);
  c->SetTicks(1,1);
  c->SetLeftMargin(0.14);
  c->SetBottomMargin(0.12);
  c->SetTopMargin(0.08);
  c->SetRightMargin(0.05);

  if (logx) c->SetLogx();
  if (logy) c->SetLogy();

  // set titles on first histogram
  TH1* h0 = hs[0];
  if (!title.empty()) h0->SetTitle(title.c_str());
  h0->GetXaxis()->SetTitle(xTitle.c_str());
  h0->GetYaxis()->SetTitle(yTitle.c_str());

  // determine y range using all histograms
  double maxy = 0;
  for (auto h : hs) if (h) maxy = std::max(maxy, h->GetMaximum());
  if (maxy > 0) h0->SetMaximum(1.25 * maxy);

  // allow setting x/y min and max independently (use current axis limits when one side is left as -999)
  double curXmin = h0->GetXaxis()->GetXmin();
  double curXmax = h0->GetXaxis()->GetXmax();
  double newXmin = (xMin != -999) ? xMin : curXmin;
  double newXmax = (xMax != -999) ? xMax : curXmax;
  h0->GetXaxis()->SetRangeUser(newXmin, newXmax);

  double curYmin = h0->GetYaxis()->GetXmin();
  double curYmax = h0->GetYaxis()->GetXmax();
  double newYmin = (yMin != -999) ? yMin : curYmin;
  double newYmax = (yMax != -999) ? yMax : curYmax;
  h0->GetYaxis()->SetRangeUser(newYmin, newYmax);
  // draw histograms (assume they are already styled)
  bool first = true;
  for (auto h : hs) {
    if (!h) continue;
    if (first) { h->Draw("E1 HIST"); first = false; }
    else       { h->Draw("E1 HIST SAME"); }
  }

  DrawTLatexLines(captionInfo);
  // legend placement: adapt height to number of entries
  const size_t n = hs.size();
  double ly2 = 0.9;
  double ly1 = ly2 - 0.05 * std::max<size_t>(n, 1);
  if (ly1 < 0.1) ly1 = 0.1;
  TLegend* leg = new TLegend(0.5, ly1, 0.9, ly2);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->SetTextFont(132);
  leg->SetTextSize(0.020);
  for (size_t i = 0; i < hs.size(); ++i) {
    if (!hs[i]) continue;
    leg->AddEntry(hs[i], lab[i].c_str(), "l");
  }
  leg->Draw();
  c->SaveAs(outPng.c_str());
}

static void DrawStack(const std::vector<TH1*>& hs,
                        const std::vector<std::string>& labels,
                        const std::string& outPng,
                        const std::string& xTitle,
                        const std::string& yTitle = "Normalized entries",
                        const std::string& title = "",
                        float xMin = -999, float xMax = -999,
                        float yMin = -999, float yMax = -999,
                        bool logx = false,
                        bool logy = false,
                        BlockCaptionInfo captionInfo = {}) {
  if (hs.empty()) return;

  // basic sanity on labels
  std::vector<std::string> lab = labels;
  if (lab.size() != hs.size()) {
    lab.resize(hs.size());
    for (size_t i = 0; i < hs.size(); ++i) {
      if (i < labels.size()) lab[i] = labels[i];
      else lab[i] = std::string("hist") + std::to_string(i+1);
    }
  }

  TCanvas* c = new TCanvas("c", "canvas", 800, 800);
  c->SetTicks(1,1);
  c->SetLeftMargin(0.14);
  c->SetBottomMargin(0.12);
  c->SetTopMargin(0.08);
  c->SetRightMargin(0.05);

  if (logx) c->SetLogx();
  if (logy) c->SetLogy();

  // set titles on first histogram

  THStack *hsStack = new THStack("hsStack", title.c_str());
  for (auto h : hs) {
    if (!h) continue;
    hsStack->Add(h);
  }

  hsStack->SetTitle(title.c_str());
  hsStack->Draw("HIST");

  hsStack->GetXaxis()->SetTitle(xTitle.c_str());
  hsStack->GetYaxis()->SetTitle(yTitle.c_str());
  if (xMin != -999 && xMax != -999) hsStack->GetXaxis()->SetRangeUser(xMin, xMax);
  if (yMin != -999 && yMax != -999) hsStack->GetYaxis()->SetRangeUser(yMin, yMax);

  
  DrawTLatexLines(captionInfo);
  // legend placement: adapt height to number of entries
  const size_t n = hs.size();
  double ly2 = 0.9;
  double ly1 = ly2 - 0.05 * std::max<size_t>(n, 1);
  if (ly1 < 0.1) ly1 = 0.1;
  TLegend* leg = new TLegend(0.5, ly1, 0.9, ly2);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->SetTextFont(132);
  leg->SetTextSize(0.035);
  for (size_t i = 0; i < hs.size(); ++i) {
    if (!hs[i]) continue;
    leg->AddEntry(hs[i], lab[i].c_str(), "f");
  }
  leg->Draw();

  c->SaveAs(outPng.c_str());
}

static void StyleHist(TH1* h, int color, int marker){
  h->SetLineColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(marker);
  h->SetLineWidth(2);
}

static void Normalize(TH1* h){
  double integral = h->Integral(0, h->GetNbinsX()+1);
  if (integral > 0) h->Scale(1.0 / integral);
}

static void NormalizePerEvt(TH1* h, double nEvts){
  if (nEvts > 0) h->Scale(1.0 / nEvts);
}


int PlotHist(){

    string outfolder = "/home/xirong/JetStudiesOO/022426ScanForest/Plots/03201PDPlots/";

    string inputFileDATA = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0318OnePD/JetSubHist_Data_93.4MEvts_Data1PD_withjetcuts_20260319.root";
    string inputFileMCpthat15 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/mergedPTHat15_1PD.root";
    string inputFileMCpthat0 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0318OnePD/JetSubHist_MC_31KEvts_pthat01PD_withjetcuts_20260319.root";
    string inputFileMCpthat15_forcuts = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/mergedPTHat15_1PD.root";

    TFile *fileDATA = TFile::Open(inputFileDATA.c_str());
    TFile *fileMCpthat15 = TFile::Open(inputFileMCpthat15.c_str());
    TFile *fileMCpthat0 = TFile::Open(inputFileMCpthat0.c_str());

    if (!fileDATA || !fileMCpthat15 || !fileMCpthat0) {
        cout << "Error opening files!" << endl;
        return 1;
    }

    TNamed* cutInfoData = (TNamed*)fileDATA->Get("Cuts");
    TNamed* cutInfoMCpthat15 = (TNamed*)fileMCpthat15->Get("Cuts");
    TNamed* cutInfoMCpthat0 = (TNamed*)fileMCpthat0->Get("Cuts");

    TNamed* generalInfoData = (TNamed*)fileDATA->Get("GeneralInfo");
   // TNamed* generalInfoMCpthat15 = (TNamed*)fileMCpthat15->Get("GeneralInfo");
   // TNamed* generalInfoMCpthat0 = (TNamed*)fileMCpthat0->Get("GeneralInfo");

  
    cout << "Debug: Cuts applied in Data: " << cutInfoData->GetTitle() << endl;
    cout << "Debug: Cuts applied in MC pthat15: " << cutInfoMCpthat15->GetTitle() << endl;
    cout << "Debug: Cuts applied in MC pthat0: " << cutInfoMCpthat0->GetTitle() << endl;
    cout << "GeneralInfo:" << generalInfoData->GetTitle() << endl;
    cout << "Debug: Data histogram entries:" << endl;

    TH1F* hjtpt_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtpt");
    TH1F* hjteta_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjteta");
    TH1F* hjtphi_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtphi");
    TH1F* hjty_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjty");
    TH1F* hjtrg_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtrg");
    TH1F* hjtzg_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtzg");
    TH1F* hjtkt_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtkt");
    TH1F* hjtangu_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtangu");

    TH1F* hjtPfCHF_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtPfCHF");
    TH1F* hjtPfNHF_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtPfNHF");
    TH1F* hjtPfCEF_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtPfCEF");
    TH1F* hjtPfNEF_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtPfNEF");
    TH1F* hjtPfMUF_data = (TH1F*)fileDATA->Get("JetLevelHistRaw/hjtPfMUF");

    TH1F* hjtpt1_data = (TH1F*)fileDATA->Get("EventLevelHistRaw/hjtpt1");
    TH1F* hjtpt2_data = (TH1F*)fileDATA->Get("EventLevelHistRaw/hjtpt2");
    TH1F* hAj_data = (TH1F*)fileDATA->Get("EventLevelHistRaw/hAj");
    TH1F* hXj_data = (TH1F*)fileDATA->Get("EventLevelHistRaw/hXj");
    TH1F* hdPhi_data = (TH1F*)fileDATA->Get("EventLevelHistRaw/hdPhi");

    // Repeat same retrieval for MC pthat15
    // Jet-level (pthat15)
    TH1F* hjtpt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtpt");
    TH1F* hjteta_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjteta");
    TH1F* hjtphi_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtphi");
    TH1F* hjty_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjty");
    TH1F* hjtrg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtrg");
    TH1F* hjtzg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtzg");
    TH1F* hjtkt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtkt");
    TH1F* hjtangu_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtangu");

    TH1F* hjtPfCHF_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtPfCHF");
    TH1F* hjtPfNHF_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtPfNHF");
    TH1F* hjtPfCEF_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtPfCEF");
    TH1F* hjtPfMUF_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hjtPfMUF");

    TH1F* hjtpt1_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hjtpt1");
    TH1F* hjtpt2_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hjtpt2");
    TH1F* hAj_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hAj");
    TH1F* hXj_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hXj");
    TH1F* hdPhi_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hdPhi");

    TH1F* hrefpt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefpt");
    TH1F* hrefeta_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefeta");
    TH1F* hrefphi_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefphi");
    TH1F* hrefy_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefy");
    TH1F* hrefrg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefrg");
    TH1F* hrefzg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefzg");
    TH1F* hrefkt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefkt");
    TH1F* hrefangu_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hrefangu");

    TH1F* hrefpt1_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hrefpt1");
    TH1F* hrefpt2_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hrefpt2");
    TH1F* hAj_ref_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hAjref");
    TH1F* hXj_ref_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hXjref");
    TH1F* hdPhi_ref_pthat15 = (TH1F*)fileMCpthat15->Get("EventLevelHistRaw/hdPhiref");

    TH1F* hgenpt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenpt");
    TH1F* hgeneta_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgeneta");
    TH1F* hgenphi_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenphi");
    TH1F* hgeny_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgeny");
    TH1F* hgenrg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenrg");
    TH1F* hgenzg_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenzg");
    TH1F* hgenkt_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenkt");
    TH1F* hgenangu_pthat15 = (TH1F*)fileMCpthat15->Get("JetLevelHistRaw/hgenangu");

    std::vector<TH1F*> dataHists = {hjtpt_data, hjteta_data, hjtphi_data, hjty_data, hjtrg_data, hjtzg_data, hjtkt_data, hjtangu_data,hjtpt1_data, hjtpt2_data, hAj_data, hXj_data, hdPhi_data};
    std::vector<TH1F*> mcPthat15jtHists = {hjtpt_pthat15, hjteta_pthat15, hjtphi_pthat15, hjty_pthat15, hjtrg_pthat15, hjtzg_pthat15, hjtkt_pthat15, hjtangu_pthat15};
    std::vector<TH1F*> mcPthat15refHists = {hrefpt_pthat15, hrefeta_pthat15, hrefphi_pthat15, hrefy_pthat15, hrefrg_pthat15, hrefzg_pthat15, hrefkt_pthat15, hrefangu_pthat15};
    std::vector<TH1F*> mcPthat15genHists = {hgenpt_pthat15, hgeneta_pthat15, hgenphi_pthat15, hgeny_pthat15, hgenrg_pthat15, hgenzg_pthat15, hgenkt_pthat15, hgenangu_pthat15};
    //std::vector<TH1F*> mcPthat0Hists = {hjtpt_pthat0, hjteta_pthat0, hjtphi_pthat0, hjty_pthat0, hjtrg_pthat0, hjtzg_pthat0, hjtkt_pthat0, hjtangu_pthat0,};

    for (auto h : dataHists) {
      StyleHist(h, kBlack, 21);
      h->Scale(1.0 / h->GetSumOfWeights());
    }
    for (auto h : mcPthat15jtHists) {
      StyleHist(h, kRed, 22);
      h->Scale(1.0 / h->GetSumOfWeights());
    }
    for (auto h : mcPthat15refHists) {
      StyleHist(h, kGreen+2, 23);
      h->Scale(1.0 / h->GetSumOfWeights());
    } 
    for (auto h : mcPthat15genHists) {
      StyleHist(h, kBlue+1, 24);
      h->Scale(1.0 / h->GetSumOfWeights());
    }

    gStyle->SetOptStat(0); // Disable statistics box

    BlockCaptionInfo captionInfo;
    vector<std::string> cutInfoDataVector;
    vector<std::string> generalInfoDataVector;

    std::stringstream ssCut(cutInfoData->GetTitle());
    std::string item;
    while (std::getline(ssCut, item, ';')) cutInfoDataVector.push_back(item);

    std::stringstream ssGen(generalInfoData->GetTitle());
    while (std::getline(ssGen, item, ';')) generalInfoDataVector.push_back(item);

    // Debug Caption Info
    for (const auto& s : cutInfoDataVector) cout << "Cut Info: " << s << endl;
    for (const auto& s : generalInfoDataVector) cout << "General Info: " << s << endl;


    vector<std::string> captionLines = cutInfoDataVector;
      captionLines.push_back(generalInfoDataVector[4]);
    captionLines.push_back(generalInfoDataVector[5]);
    captionLines.push_back(generalInfoDataVector[7]);
    captionLines.push_back(generalInfoDataVector[8]);

    /*************************************************************************
     *                                                                       *
     *                      MC, Data Comparison                              *
     *                                                                       *
     *************************************************************************/

    std::vector<std::string> obsNames = {"hjtpt", "hjteta", "hjtphi", "hjty", "hjtrg", "hjtzg", "hjtkt", "hjtangu"};
    std::vector<std::string> xTitles = {
      "Jet p_{T} (GeV/c)",
      "Jet #eta",
      "Jet #phi",
      "Jet y",
      "Jet rg",
      "Jet zg",
      "Jet kt",
      "Jet angularity"
    };
    
    std::vector<std::pair<float,float>> xranges = {
      {0, 1000},    // pt
      {-2.5f, 2.5f},// eta
      {-3.2f, 3.2f},// phi
      {-2.5f, 2.5f},// y
      {0, 1.0f},   // rg
      {0, 1.0f},   // zg
      {0, 100},    // kt
      {0, 1} // angularity: auto
    };

    std::vector<std::pair<float,float>> yranges = {
      {10e-10, -999},    // pt
      {-999, -999},    // eta
      {-999, -999},    // phi
      {-999, -999},    // y
      {-999, -999},   // rg
      {-999, -999},   // zg
      {-999, -999},   // kt
      {-999, -999}    // angularity: auto
    };


    std::vector<BlockCaptionInfo> captionInfos = {
      {captionLines, 0.6, 0.6, 0.023, 0.022, true}, //pt
      {captionLines, 0.4, 0.5, 0.023, 0.022, true},// eta
      {captionLines, 0.6, 0.6, 0.023, 0.022, true},// phi
      {captionLines, 0.20, 0.55, 0.023, 0.022, true},// y
      {captionLines, 0.6, 0.6, 0.023, 0.022, true},// rg
      {captionLines, 0.6, 0.6, 0.023, 0.022, true},// zg
      {captionLines, 0.6, 0.6, 0.023, 0.022, true},// kt
      {captionLines, 0.6, 0.6, 0.023, 0.022, true} // angularity: auto
    };

    for (size_t i = 0; i < dataHists.size() && i < mcPthat15jtHists.size(); ++i) {
      TH1F* hdata = dataHists[i];
      TH1F* hMC = mcPthat15jtHists[i];
      if (!hdata || !hMC) continue;
      captionInfo = captionInfos[i];

      DrawAndSave({hdata, hMC},
            {"Data", "MC Reco"},
            outfolder + obsNames[i] + "_datamc_comp.png",
            xTitles[i],
            Form("dN/d%s", xTitles[i].c_str()),
            std::string("Comparison of jet-level and reference for ") + xTitles[i],
            xranges[i].first, xranges[i].second,
            yranges[i].first, yranges[i].second,
            false, true,
            captionInfo);

      std::string rname = std::string("ratio_") + obsNames[i] + "_mcovedata";

      TH1F* ratio = (TH1F*)hMC->Clone(rname.c_str());
      ratio->SetDirectory(nullptr);
      ratio->Divide(hdata); // now MC / Data
      StyleHist(ratio, kBlack, 21);

      std::string outname = outfolder + obsNames[i] + "_mcoverdata.png";
      float xmin = xranges[i].first;
      float xmax = xranges[i].second;
      // use y range 0,2 for pt-like distributions, otherwise auto (you can adjust if desired)
      float ymin = 0;
      float ymax = 2;
      DrawAndSave({ratio},
            {"Jet / Ref"},
            outname,
            xTitles[i],
            "Jet / Ref",
            std::string("Ratio of jet-level to ref-level for ") + xTitles[i],
            xmin, xmax, ymin, ymax,
            false, false,
            captionInfo);
      delete ratio;
    }
  

    /*************************************************************************
     *                                                                       *
     *                      Jt vs Ref vs Gen comparison:                     *
     *                                                                       *
     *************************************************************************/

    captionInfos = {
      {captionLines, 0.6, 0.75, 0.020, 0.02, true}, //pt
      {captionLines, 0.4, 0.5, 0.023, 0.02, true},// eta
      {captionLines, 0.6, 0.6, 0.020, 0.02, true},// phi
      {captionLines, 0.20, 0.55, 0.020, 0.02, true},// y
      {captionLines, 0.6, 0.6, 0.020, 0.02, true},// rg
      {captionLines, 0.6, 0.6, 0.020, 0.02, true},// zg
      {captionLines, 0.6, 0.6, 0.020, 0.02, true},// kt
      {captionLines, 0.6, 0.6, 0.020, 0.02, true} // angularity: auto
    };

    for (size_t i = 0; i < mcPthat15jtHists.size() && i < mcPthat15refHists.size(); ++i) {
      TH1F* hj = mcPthat15jtHists[i];
      TH1F* hr = mcPthat15refHists[i];
      TH1F* hg = mcPthat15genHists[i];
      if (!hj || !hr || !hg) continue;
      captionInfo = captionInfos[i];

      DrawAndSave({hj, hr, hg},
            {"Jet Reco", "Ref", "Gen"},
            outfolder + obsNames[i] + "_jt_and_ref_and_gen.png",
            xTitles[i],
            Form("dN/d%s", xTitles[i].c_str()),
            std::string("Comparison of jet-level and reference for ") + xTitles[i],
            xranges[i].first, xranges[i].second,
            yranges[i].first, yranges[i].second,
            false, true,
            captionInfo);

      std::string rname = std::string("ratio_") + obsNames[i] + "_jt_over_ref";

      TH1F* ratio = (TH1F*)hj->Clone(rname.c_str());
      ratio->SetDirectory(nullptr); // avoid ownership by current file/canvas
      ratio->Divide(hr);
      StyleHist(ratio, kBlack, 21);

      std::string outname = outfolder + obsNames[i] + "_jt_over_ref.png";
      float xmin = xranges[i].first;
      float xmax = xranges[i].second;
      // use y range 0..2 for pt-like distributions, otherwise auto (you can adjust if desired)
      float ymin = -999, ymax = -999;
      if (i == 0) { ymin = 0; ymax = 2; } // pt
      else if (i == 4 || i == 6 || i==5) { ymin = 0; ymax = 2; } // R, kt maybe
      DrawAndSave({ratio},
            {"Jet / Ref"},
            outname,
            xTitles[i],
            "Jet / Ref",
            std::string("Ratio of jet-level to ref-level for ") + xTitles[i],
            xmin, xmax, ymin, ymax,
            false, false,
            captionInfo);

      delete ratio;
    }
  

    fileDATA->Close();
    fileMCpthat15->Close();
    fileMCpthat0->Close();

    return 0;
}
