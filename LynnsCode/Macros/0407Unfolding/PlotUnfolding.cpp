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

struct BlockCaptionInfo{
  std::vector<std::string> text;
  double x;
  double y;
  double size;
  double spacing;
  bool useNDC;
};
static void FormatCanvas(TCanvas* c, bool logx, bool logy) {
  c->SetTicks(1,1);
  c->SetLeftMargin(0.17);
  c->SetBottomMargin(0.15);
  c->SetTopMargin(0.08);
  c->SetRightMargin(0.15);

  if (logx) c->SetLogx();
  if (logy) c->SetLogy();
}
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


int PlotUnfolding(){

    string dataFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0408Unfolding/Response_Data_93.4MEvts_Data_xrd_20260409.root";
    string MCFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0408Unfolding/Response_MC_99.7MEvts_MC_xrd_20260409.root";
    string outfolder = "/home/xirong/JetStudiesOO/LynnsCode/Plots/Response/0409Unfolding/";

    TFile *dataf = TFile::Open(dataFile.c_str(), "READ");
    TFile *MCf = TFile::Open(MCFile.c_str(), "READ");
    if (!dataf || dataf->IsZombie()) {
        cout << "Error opening file: " << dataFile << endl;
        return 1;
    }
    if (!MCf || MCf->IsZombie()) {
        cout << "Error opening file: " << MCFile << endl;
        return 1;
    }

    TH1F* hDataPt = (TH1F*)dataf->Get("hDataPt");
    TH1F* hGenPt = (TH1F*)MCf->Get("hGenPt");
    TH1F* hRecoPt = (TH1F*)MCf->Get("hRecoPt");
    RooUnfoldResponse* response_pt = (RooUnfoldResponse*)MCf->Get("response_pt");    // ===== UNFOLDING =====
    // Add these checks:
    if (!hDataPt) {
        cout << "Error: Could not find hDataPt in data file" << endl;
        return 1;
    }
    if (!hGenPt) {
        cout << "Error: Could not find hGenPt in MC file" << endl;
        return 1;
    }
    if (!hRecoPt) {
        cout << "Error: Could not find hRecoPt in MC file" << endl;
        return 1;
    }
    if (!response_pt) {
        cout << "Error: Could not find response_pt in MC file" << endl;
        return 1;
    }
    RooUnfoldBayes unfold(response_pt, hDataPt, 4);  // Use pointer, not reference
    TH1F* hUnfolded = (TH1F*)unfold.Hunfold();
    RooUnfoldBayes unfold_toy(response_pt, hRecoPt, 4);  // Use pointer, not reference
    TH1F* hUnfoldedToy = (TH1F*)unfold_toy.Hunfold();
    cout << "Unfolding complete!" << endl;
    cout << "Unfolded histogram: " << hUnfolded->GetName() << endl;
    // ===== END UNFOLDING =====

    TNamed* cutInfo= (TNamed*)MCf->Get("Cuts");
    TNamed* generalInfo = (TNamed*)MCf->Get("GeneralInfo");

    cout << "Debug: Cuts applied: " << cutInfo->GetTitle() << endl;
    cout << "Debug: General info: " << generalInfo->GetTitle() << endl;

    vector<std::string> cutInfoVector;
    std::stringstream ssCut(cutInfo->GetTitle());
    std::string item;
    while (std::getline(ssCut, item, ';')) cutInfoVector.push_back(item);

    vector<std::string> generalInfoVector;
    std::stringstream ssGen(generalInfo->GetTitle());
    while (std::getline(ssGen, item, ';')) generalInfoVector.push_back(item);

    vector<std::string> captionLines = {};
    captionLines.push_back(generalInfoVector[0]);

    BlockCaptionInfo captionInfo;
    captionInfo = {captionLines, 0.20, 0.80, 0.03, 0.03, true};

    gStyle->SetOptStat(0); // Disable statistics box
    gStyle->SetPaintTextFormat(".2f");
 
    StyleHist(hGenPt, kBlue, 20);
    StyleHist(hRecoPt, kRed, 21);
    StyleHist(hDataPt, kBlack, 22);
    StyleHist(hUnfolded, kGreen+2, 23);
    StyleHist(hUnfoldedToy, kGreen+2, 24);
    vector<TH1*> histsToCheck = {hGenPt, hRecoPt, hDataPt, hUnfolded};
    vector<string> histNames = {"hGenPt", "hRecoPt", "hDataPt", "hUnfolded"};

    DrawAndSave({hGenPt, hRecoPt, hDataPt, hUnfolded}, 
                {"Gen", "Reco", "Measured data", "Unfolded"}, 
                outfolder + "PtComparison.png", 
                "p_{T} (GeV/c)", 
                "dN/dp_{T}", 
                "p_{T} distribution: Gen vs Reco vs Data vs Unfolded", 
                30, -999, 1e-6, 5e5, true, true, captionInfo);
    DrawAndSave({hGenPt, hRecoPt, hUnfoldedToy}, 
                {"Gen", "Reco","Unfolded (Toy)"},
                outfolder + "PtComparison_Toy.png",
                "p_{T} (GeV/c)",
                "dN/dp_{T}",
                "p_{T} distribution: Gen vs Unfolded (Toy)",
                30, -999, 1e-6, 5e5, true, true, captionInfo);

    //Plot Response matrix
    TCanvas* c = new TCanvas("c", "Response Matrix", 800, 800);
    FormatCanvas(c, false, false);
    c->SetLogz();
    response_pt->Hresponse()->Draw("COLZ");
    DrawTLatexLines(captionInfo);

    c->SaveAs((outfolder + "ResponseMatrix_pt_logz.png").c_str());
}
