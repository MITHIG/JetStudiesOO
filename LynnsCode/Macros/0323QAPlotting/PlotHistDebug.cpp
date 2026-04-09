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

  if (xMin != -999 && xMax != -999) h0->GetXaxis()->SetRangeUser(xMin, xMax);
  if (yMin != -999 && yMax != -999) h0->GetYaxis()->SetRangeUser(yMin, yMax);
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


int PlotHistDebug(){

    string outfolder = "/home/xirong/JetStudiesOO/022426ScanForest/Plots/0320DebugPlots/";

    string inputFileDATA = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0318OnePD/JetSubHist_Data_93.4MEvts_Data1PD_withjetcuts_20260319.root";
    string inputFileMCpthat15_1 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_13.5MEvts_pthat15_1PD_1_withjetcuts_20260320.root";
    string inputFileMCpthat15_2 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_13.7MEvts_pthat15_2PD_2_withjetcuts_20260320.root";
    string inputFileMCpthat15_3 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_14.1MEvts_pthat15_1PD_3_withjetcuts_20260320.root";
    string inputFileMCpthat15_4 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_13.7MEvts_pthat15_1PD_4_withjetcuts_20260320.root";
    string inputFileMCpthat15_5 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_13.7MEvts_pthat15_1PD_5_withjetcuts_20260320.root";
    string inputFileMCpthat15_6 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_13.6MEvts_pthat15_1PD_6_withjetcuts_20260320.root";
    string inputFileMCpthat15_7 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_12.8MEvts_pthat15_1PD_7_withjetcuts_20260320.root";
    string inputFileMCpthat15_8 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_3.6MEvts_pthat15_1PD_8_withjetcuts_20260320.root";
    string inputFileMCpthat15_9 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0320MultipleHistMethod/JetSubHist_MC_1.2MEvts_pthat15_1PD_9_withjetcuts_20260320.root";


    TFile *fileDATA = TFile::Open(inputFileDATA.c_str());
    TFile *fileMCpthat15_1 = TFile::Open(inputFileMCpthat15_1.c_str());
    TFile *fileMCpthat15_2 = TFile::Open(inputFileMCpthat15_2.c_str());
    TFile *fileMCpthat15_3 = TFile::Open(inputFileMCpthat15_3.c_str());
    TFile *fileMCpthat15_4 = TFile::Open(inputFileMCpthat15_4.c_str());
    TFile *fileMCpthat15_5 = TFile::Open(inputFileMCpthat15_5.c_str());
    TFile *fileMCpthat15_6 = TFile::Open(inputFileMCpthat15_6.c_str());
    TFile *fileMCpthat15_7 = TFile::Open(inputFileMCpthat15_7.c_str());
    TFile *fileMCpthat15_8 = TFile::Open(inputFileMCpthat15_8.c_str());
    TFile *fileMCpthat15_9 = TFile::Open(inputFileMCpthat15_9.c_str());

    if (!fileDATA || !fileMCpthat15_1 || !fileMCpthat15_2 || !fileMCpthat15_3 || !fileMCpthat15_4 || !fileMCpthat15_5 || !fileMCpthat15_6 || !fileMCpthat15_7 || !fileMCpthat15_8 || !fileMCpthat15_9) {
        cout << "Error opening files!" << endl;
        return 1;
    }

    TNamed* cutInfoData = (TNamed*)fileDATA->Get("Cuts");
    TNamed* cutInfoMCpthat15_1 = (TNamed*)fileMCpthat15_1->Get("Cuts");
  

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
    TH1F* hjtpt_pthat15_1 = (TH1F*)fileMCpthat15_1->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_1 = (TH1F*)fileMCpthat15_1->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_1 = (TH1F*)fileMCpthat15_1->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_2 = (TH1F*)fileMCpthat15_2->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_2 = (TH1F*)fileMCpthat15_2->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_2 = (TH1F*)fileMCpthat15_2->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_3 = (TH1F*)fileMCpthat15_3->Get("JetLevelHistRaw/hjtpt");  
    TH1F* hrefpt_pthat15_3 = (TH1F*)fileMCpthat15_3->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_3 = (TH1F*)fileMCpthat15_3->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_4 = (TH1F*)fileMCpthat15_4->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_4 = (TH1F*)fileMCpthat15_4->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_4 = (TH1F*)fileMCpthat15_4->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_5 = (TH1F*)fileMCpthat15_5->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_5 = (TH1F*)fileMCpthat15_5->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_5 = (TH1F*)fileMCpthat15_5->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_6 = (TH1F*)fileMCpthat15_6->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_6 = (TH1F*)fileMCpthat15_6->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_6 = (TH1F*)fileMCpthat15_6->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_7 = (TH1F*)fileMCpthat15_7->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_7 = (TH1F*)fileMCpthat15_7->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_7 = (TH1F*)fileMCpthat15_7->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_8 = (TH1F*)fileMCpthat15_8->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_8 = (TH1F*)fileMCpthat15_8->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_8 = (TH1F*)fileMCpthat15_8->Get("JetLevelHistRaw/hjtrg");

    TH1F* hjtpt_pthat15_9 = (TH1F*)fileMCpthat15_9->Get("JetLevelHistRaw/hjtpt");
    TH1F* hrefpt_pthat15_9 = (TH1F*)fileMCpthat15_9->Get("JetLevelHistRaw/hrefpt");
    TH1F* hjtrg_pthat15_9 = (TH1F*)fileMCpthat15_9->Get("JetLevelHistRaw/hjtrg");

    std::vector<TH1F*> dataHists = {hjtpt_data, hjteta_data, hjtphi_data, hjty_data, hjtrg_data, hjtzg_data, hjtkt_data, hjtangu_data,hjtpt1_data, hjtpt2_data, hAj_data, hXj_data, hdPhi_data};

    std::vector<TH1F*> pthists = {hjtpt_pthat15_1, hjtpt_pthat15_2, hjtpt_pthat15_3, hjtpt_pthat15_4, hjtpt_pthat15_5, hjtpt_pthat15_6, hjtpt_pthat15_7, hjtpt_pthat15_8, hjtpt_pthat15_9};
    std::vector<TH1F*> refpthists = {hrefpt_pthat15_1, hrefpt_pthat15_2, hrefpt_pthat15_3, hrefpt_pthat15_4, hrefpt_pthat15_5, hrefpt_pthat15_6, hrefpt_pthat15_7, hrefpt_pthat15_8, hrefpt_pthat15_9};
    std::vector<TH1F*> rgHists = {hjtrg_pthat15_1, hjtrg_pthat15_2, hjtrg_pthat15_3, hjtrg_pthat15_4, hjtrg_pthat15_5, hjtrg_pthat15_6, hjtrg_pthat15_7, hjtrg_pthat15_8, hjtrg_pthat15_9};
    for (auto h : dataHists) {
      StyleHist(h, kBlack, 21);
      h->Scale(1.0 / h->GetSumOfWeights());
    }
    for (auto h : pthists) {
      StyleHist(h, kRed, 22);
      h->Scale(1.0 / h->GetSumOfWeights());
    }
    for (auto h : rgHists) {
      StyleHist(h, kRed, 23);
      h->Scale(1.0 / h->GetSumOfWeights());
    }
    for (auto h : refpthists) {
      StyleHist(h, kGreen+2, 24);
      h->Scale(1.0 / h->GetSumOfWeights());
    }

    gStyle->SetOptStat(0); // Disable statistics box

    TCanvas *c = new TCanvas("c","c",1200,1200);
    c->Divide(3,3);
    for (int i = 0; i < 9; i++) {
        c->cd(i+1);
        hjtpt_data->Draw("E1 HIST");
        pthists[i]->Draw("E1 HIST SAME");
        gPad->SetLogy();
        
    }

    TCanvas *c2 = new TCanvas("c2","c2",1200,1200);
    c2->Divide(3,3);

    for (int i = 0; i < 9; i++) {
        c2->cd(i+1);
        hjtrg_data->Draw("E1 HIST");
        rgHists[i]->SetTitle(Form("Set #%d ", i+1));
        rgHists[i]->Draw("E1 HIST SAME");
    }

    TCanvas *c3 = new TCanvas("c3","c3",1200,1200);
    c3->Divide(3,3);
    c3->SetLogy();

    for (int i = 0; i < 9; i++) {
      c3->cd(i+1);
      if (gPad) gPad->SetLogy();
      pthists[i]->Draw("E1 HIST");
      refpthists[i]->SetTitle(Form("Set #%d ", i+1));
      refpthists[i]->Draw("E1 HIST SAME");
    }

    c->SaveAs((outfolder + "DebugPlots_pt.png").c_str());
    c2->SaveAs((outfolder + "DebugPlots_rg.png").c_str());
    c3->SaveAs((outfolder + "DebugPlots_refpt.png").c_str());
}
