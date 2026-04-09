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


int PlotResponse(){


    string dataFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0408Unfolding/Response_Data_93.4MEvts_Data_xrd_20260409.root";
    string MCFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0408Unfolding/Response_MC_99.7MEvts_MC_xrd_20260409.root";
    string outfolder = "/home/xirong/JetStudiesOO/022426ScanForest/Plots/Response/0331ResponsePlotsNew/";

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

    TNamed* cutInfo= (TNamed*)file->Get("Cuts");
    TNamed* generalInfo = (TNamed*)file->Get("GeneralInfo");
  
    cout << "Debug: Cuts applied: " << cutInfo->GetTitle() << endl;
    cout << "Debug: General info: " << generalInfo->GetTitle() << endl;

    TH2F* hResponse_pt = (TH2F*)file->Get("hResponse_pt");
    if (!hResponse_pt) {
        cout << "Error: hResponse_pt not found in file!" << endl;
        return 1;
    }
    TH2F* hResponse_rg = (TH2F*)file->Get("hResponse_rg");
    if (!hResponse_rg) {
        cout << "Error: hResponse_rg not found in file!" << endl;
        return 1;
    }
    TH2F* hResponse_zg = (TH2F*)file->Get("hResponse_zg");
    if (!hResponse_zg) {
        cout << "Error: hResponse_zg not found in file!" << endl;
        return 1;
    }

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
 
    // PT response
    captionInfo = {captionLines, 0.20, 0.80, 0.03, 0.03, true};
    TCanvas* c = new TCanvas("c", "Response Matrix", 800, 800);
    FormatCanvas(c, false, false);
    c->SetLogz();
    hResponse_pt->Draw("COLZ TEXT");
    hResponse_pt->GetXaxis()->SetTitleOffset(1.4);   // increase >1 to move down
    hResponse_pt->SetTitle("Response Matrix: Jet p_{T}");
    DrawTLatexLines(captionInfo);

    c->SaveAs((outfolder + "ResponseMatrix_pt_logz.png").c_str());

    // Rg response
    gStyle->SetPaintTextFormat(".0f");
    captionInfo = {captionLines, 0.20, 0.85, 0.03, 0.03, true};
    c = new TCanvas("c", "Response Matrix", 800, 800);
    FormatCanvas(c, false, false);
    hResponse_rg->GetXaxis()->SetTitleOffset(1.4);   // increase >1 to move down
    hResponse_rg->SetTitle("Response Matrix: Jet R_{g}");
    hResponse_rg->Draw("COLZ TEXT");
    DrawTLatexLines(captionInfo);
    c->SetLogz();
    c->SaveAs((outfolder + "ResponseMatrix_rg_logz.png").c_str());


    // Zg response
    c = new TCanvas("c", "Response Matrix", 800, 800);
    FormatCanvas(c, false, false);
    hResponse_zg->GetXaxis()->SetTitleOffset(1.4);   // increase >1 to move down
    hResponse_zg->SetTitle("Response Matrix: Jet Z_{g}");
    hResponse_zg->Draw("COLZ TEXT");
    c->SetLogz();
    c->SaveAs((outfolder + "ResponseMatrix_zg_logz.png").c_str());

    return 0;
}
