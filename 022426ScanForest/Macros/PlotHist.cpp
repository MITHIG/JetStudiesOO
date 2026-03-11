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
  leg->SetTextSize(0.035);
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
    string inputFileName = "../RootFiles/HistTFiles/JetSubstructureHistogramChain_031026.root";

    TFile *file = TFile::Open(inputFileName.c_str());
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << inputFileName << std::endl;
        return 1;
    
    
    }

    TNamed* cutInfo = (TNamed*)file->Get("Cuts");
    TDirectory *dir = (TDirectory*)file->Get("EventLevelHistRaw");
    // Event-level histograms (kept for use elsewhere in the macro)
    TH1F *hjtpt1 = (TH1F*)file->Get("EventLevelHistRaw/hjtpt1");
    TH1F *hjtpt2 = (TH1F*)file->Get("EventLevelHistRaw/hjtpt2");
    TH1F *hAj    = (TH1F*)file->Get("EventLevelHistRaw/hAj");
    TH1F *hAjref = (TH1F*)file->Get("EventLevelHistRaw/hAjref");
    TH1F *hXj    = (TH1F*)file->Get("EventLevelHistRaw/hXj");
    TH1F *hXjref = (TH1F*)file->Get("EventLevelHistRaw/hXjref");
    TH1F *hdPhi   = (TH1F*)file->Get("EventLevelHistRaw/hdPhi");
    TH1F *hdPhiref   = (TH1F*)file->Get("EventLevelHistRaw/hdPhiref");

    // Jet-level histograms (all under JetLevelHistRaw)
    TH1F *hjtpt   = (TH1F*)file->Get("JetLevelHistRaw/hjtpt");
    TH1F *hjteta  = (TH1F*)file->Get("JetLevelHistRaw/hjteta");
    TH1F *hjtphi  = (TH1F*)file->Get("JetLevelHistRaw/hjtphi");
    TH1F *hjty    = (TH1F*)file->Get("JetLevelHistRaw/hjty");
    TH1F *hjtrg   = (TH1F*)file->Get("JetLevelHistRaw/hjtrg");
    TH1F *hjtzg   = (TH1F*)file->Get("JetLevelHistRaw/hjtzg");
    TH1F *hjtkt   = (TH1F*)file->Get("JetLevelHistRaw/hjtkt");
    TH1F *hjtangu = (TH1F*)file->Get("JetLevelHistRaw/hjtangu");

    TH1F *hrefpt   = (TH1F*)file->Get("JetLevelHistRaw/hrefpt");
    TH1F *hrefeta  = (TH1F*)file->Get("JetLevelHistRaw/hrefeta");
    TH1F *hrefphi  = (TH1F*)file->Get("JetLevelHistRaw/hrefphi");
    TH1F *hrefy    = (TH1F*)file->Get("JetLevelHistRaw/hrefy");
    TH1F *hrefrg   = (TH1F*)file->Get("JetLevelHistRaw/hrefrg");
    TH1F *hrefzg   = (TH1F*)file->Get("JetLevelHistRaw/hrefzg");
    TH1F *hrefkt   = (TH1F*)file->Get("JetLevelHistRaw/hrefkt");
    TH1F *hrefangu = (TH1F*)file->Get("JetLevelHistRaw/hrefangu");

    TH1F *hgenpt   = (TH1F*)file->Get("JetLevelHistRaw/hgenpt");
    TH1F *hgeneta  = (TH1F*)file->Get("JetLevelHistRaw/hgeneta");
    TH1F *hgenphi  = (TH1F*)file->Get("JetLevelHistRaw/hgenphi");
    TH1F *hgeny    = (TH1F*)file->Get("JetLevelHistRaw/hgeny");
    TH1F *hgenrg   = (TH1F*)file->Get("JetLevelHistRaw/hgenrg");
    TH1F *hgenzg   = (TH1F*)file->Get("JetLevelHistRaw/hgenzg");
    TH1F *hgenkt   = (TH1F*)file->Get("JetLevelHistRaw/hgenkt");
    TH1F *hgenangu = (TH1F*)file->Get("JetLevelHistRaw/hgenangu");

    TH1F *hjtPfCHF = (TH1F*)file->Get("JetLevelHistRaw/hjtPfCHF");
    TH1F *hjtPfNHF = (TH1F*)file->Get("JetLevelHistRaw/hjtPfNHF");
    TH1F *hjtPfCEF = (TH1F*)file->Get("JetLevelHistRaw/hjtPfCEF");
    TH1F *hjtPfNEF = (TH1F*)file->Get("JetLevelHistRaw/hjtPfNEF");
    TH1F *hjtPfMUF = (TH1F*)file->Get("JetLevelHistRaw/hjtPfMUF");

    TH2F *hpthatvjtpt = (TH2F*)file->Get("JetLevelHistRaw/hpthatvjtpt");
    
    TDirectory *dirTrack = (TDirectory*)file->Get("TrackLevelHistRaw");
    TH1F *hxvtx = (TH1F*)file->Get("TrackLevelHistRaw/hxvtx");
    TH1F *hyvtx = (TH1F*)file->Get("TrackLevelHistRaw/hyvtx");
    TH1F *hzvtx = (TH1F*)file->Get("TrackLevelHistRaw/hzvtx");
    TH1F *hxErrVtx = (TH1F*)file->Get("TrackLevelHistRaw/hxErrVtx");
    TH1F *hyErrVtx = (TH1F*)file->Get("TrackLevelHistRaw/hyErrVtx");
    TH1F *hzErrVtx = (TH1F*)file->Get("TrackLevelHistRaw/hzErrVtx");

    // grouped hist vectors
    std::vector<TH1F*> jetHists = {hjtpt, hjteta, hjtphi, hjty, hjtrg, hjtzg, hjtkt, hjtangu,hAj, hXj, hdPhi};
    std::vector<TH1F*> refHists = {hrefpt, hrefeta, hrefphi, hrefy, hrefrg, hrefzg, hrefkt, hrefangu, hAjref, hXjref, hdPhiref};
    std::vector<TH1F*> genHists = {hgenpt, hgeneta, hgenphi, hgeny, hgenrg, hgenzg, hgenkt, hgenangu};

    std::vector<TH1F*> evtHists = {hjtpt1, hjtpt2, hAj, hAjref, hXj, hXjref, hdPhi, hdPhiref};
    std::vector<TH1F*> pfHists  = {hjtPfCHF, hjtPfNHF, hjtPfCEF, hjtPfNEF, hjtPfMUF};

  
    for (TH1F* h : jetHists) {
        StyleHist(h, kRed, 20);
        NormalizePerEvt(h,h->GetEntries());
    }
    for (TH1F* h : refHists) {
        StyleHist(h, kBlue, 24);
        NormalizePerEvt(h,h->GetEntries());
    }
    for (TH1F* h : genHists) {
        StyleHist(h, kGreen+2, 25);
        NormalizePerEvt(h,h->GetEntries());
    }
    hjtPfCHF->SetFillColor(kRed-10);
    hjtPfNHF->SetFillColor(kBlue-10);
    hjtPfCEF->SetFillColor(kGreen-10);
    hjtPfNEF->SetFillColor(kYellow-10);
    hjtPfMUF->SetFillColor(kMagenta-10);

    gStyle->SetOptStat(0); // Disable statistics box
    string outfolder = "../Plots/0307MCPlotsWithCutQaa/";

    BlockCaptionInfo captionInfo;
    vector<std::string> cutInfoVector;

    std::stringstream ss(cutInfo->GetTitle());
    std::string item;
    while (std::getline(ss, item, ';')) cutInfoVector.push_back(item);

    vector<std::string> jetlevelcut = cutInfoVector;
    vector<std::string> observlevelcut = cutInfoVector;
    vector<std::string> tracklevelcut = cutInfoVector;

    jetlevelcut.push_back(Form("NEvents: %.0f", hjtpt->GetEntries())); 
    observlevelcut.push_back(Form("NEvents: %.0f", hAj->GetEntries()));
    tracklevelcut.push_back(Form("NEvents: %.0f", hxvtx->GetEntries()));
    captionInfo = {jetlevelcut, 0.20, 0.44, 0.030, 0.04, true};
   
    captionInfo = {jetlevelcut, 0.20, 0.60, 0.020, 0.04, true};
    DrawAndSave({hjtpt, hrefpt, hgenpt},
                 {"jtpt", "refpt","genpt"},
                outfolder + "JetPt.png",
                "pT",
                "dN/dpT",
                "Jet pT Distributions",
                0, 1600,
                -999, -999,
                false, true,
                captionInfo);

    captionInfo = {jetlevelcut, 0.20, 0.60, 0.020, 0.04, true};
    DrawAndSave({hjteta, hrefeta, hgeneta},
                {"jteta", "refeta", "geneta"},
                outfolder + "JetEta.png",
                "eta",
                "dN/d(eta)",
                "Jet eta Distributions",
                -2.1, 2.1,
                -999, -999,
                false, false,
                captionInfo);
    captionInfo = {jetlevelcut, 0.20, 0.60, 0.020, 0.04, true};
            
    DrawAndSave({hjtphi, hrefphi, hgenphi},
                  {"jtphi", "refphi", "genphi"},
                  outfolder + "JetPhi.png",
                  "phi",
                  "dN/d(phi)",
                  "Jet phi Distributions",
                  -TMath::Pi(), TMath::Pi(),
                  -999, -999,
                  false, false,
                  captionInfo);
    captionInfo = {jetlevelcut, 0.60, 0.60, 0.020, 0.04, true};
    DrawAndSave({hjtrg, hrefrg, hgenrg},
                  {"jtrg", "refrg", "genrg"},
                  outfolder + "JetRG.png",
                  "RG",
                  "dN/d(RG)",
                  "Jet RG Distributions",
                  0, 1,
                  -999, -999,
                  false, false,
                  captionInfo);

    captionInfo = {jetlevelcut, 0.50, 0.60, 0.030, 0.04, true};
    DrawAndSave({hjtkt, hrefkt, hgenkt},
                  {"jtkt", "refkt", "genkt"},
                  outfolder + "JetKt.png",
                  "Kt",
                  "dN/d(Kt)",
                  "Jet Kt Distributions",
                  0, 500,
                  -999, -999,
                  false, true,
                  captionInfo);
    DrawAndSave({hjtangu, hrefangu, hgenangu},
                  {"jtangu", "refangu", "genangu"},
                  outfolder + "JetAngularity.png",
                  "Angularity",
                  "dN/d(Angularity)",
                  "Jet Angularity Distributions",
                  -2.1, 2.1,
                  -999, -999,
                  false, false,
                  captionInfo);

    DrawStack({hjtPfCHF, hjtPfNHF, hjtPfCEF, hjtPfNEF, hjtPfMUF},
                  {"jtPfCHF", "jtPfNHF", "jtPfCEF", "jtPfNEF", "jtPfMUF"},
                  outfolder + "JetPF.png",
                  "PF Energy Fraction",
                  "dN/d(PF Energy Fraction)",
                  "Jet PF Energy Fraction Distributions",
                  0, 1,
                  -999, -999,
                  false, true,
                  captionInfo);

    captionInfo = {jetlevelcut, 0.60, 0.60, 0.020, 0.04, true};
    DrawAndSave({hjtzg, hrefzg, hgenzg},
                 {"jtzg", "refzg","genzg"},
                outfolder + "JetZg.png",
                "Zg",
                "dN/d(Zg)",
                "Jet Zg Distributions",
                0, 1,
                -999, -999,
                false, false,
                captionInfo);


    TH1F* ptRatio = (TH1F*)hjtpt->Clone("ptRatio");
    ptRatio->Divide(hrefpt);
    TH1F* etaRatio = (TH1F*)hjteta->Clone("etaRatio");
    etaRatio->Divide(hrefeta);
    TH1F* phiRatio = (TH1F*)hjtphi->Clone("phiRatio");
    phiRatio->Divide(hrefphi);
    TH1F* rgRatio = (TH1F*)hjtrg->Clone("rgRatio");
    rgRatio->Divide(hrefrg);
    TH1F* anguRatio = (TH1F*)hjtangu->Clone("anguRatio");
    anguRatio->Divide(hrefangu);
    
    DrawAndSave({ptRatio},
                    {"Jet pT / Reference Jet pT"},
                    outfolder + "JetPtRatio.png",
                    "pT Ratio",
                    "dN/d(pT Ratio)",
                    "Jet pT Ratio Distributions",
                    -999,-999,
                    0, 2);
    DrawAndSave({etaRatio},
                    {"Jet eta / Reference Jet eta"},
                    outfolder + "JetEtaRatio.png",
                    "eta Ratio",
                    "dN/d(eta Ratio)",
                    "Jet eta Ratio Distributions",
                    -2.1, 2.1,
                    0, 2);
    DrawAndSave({phiRatio}, 
                    {"Jet phi / Reference Jet phi"},
                    outfolder + "JetPhiRatio.png",
                    "phi Ratio",
                    "dN/d(phi Ratio)",
                    "Jet phi Ratio Distributions",
                    -TMath::Pi(), TMath::Pi(),
                    0, 2);
    DrawAndSave({rgRatio},
                    {"Jet RG / Reference Jet RG"},
                    outfolder + "JetRGRatio.png",
                    "RG Ratio",
                    "dN/d(RG Ratio)",
                    "Jet RG Ratio Distributions",
                    -999,-999,
                    0, 2);
    DrawAndSave({anguRatio},
                    {"Jet Angularity / Reference Jet Angularity"},
                    outfolder + "JetAngularityRatio.png",
                    "Angularity Ratio",
                    "dN/d(Angularity Ratio)",
                    "Jet Angularity Ratio Distributions",
                    -999,-999,
                    0, 2);

    captionInfo = {observlevelcut, 0.50, 0.70, 0.020, 0.04, true};
    DrawAndSave({hAj, hAjref},
                    {"Aj", "Ajref"},
                    outfolder + "Aj.png",
                    "Aj",
                    "dN/d(Aj)",
                    "Jet Aj Distributions",
                    0, 1,
                    -999, -999,
                    false, false,
                    captionInfo);

    captionInfo = {observlevelcut, 0.20, 0.60, 0.020, 0.04, true};
    DrawAndSave({hXj, hXjref},
                    {"Xj", "Xjref"},
                    outfolder + "Xj.png",
                    "Xj",
                    "dN/d(Xj)",
                    "Jet Xj Distributions",
                    0, 1,
                    -999, -999,
                    false, false,
                    captionInfo); 
    DrawAndSave({hdPhi, hdPhiref},
                    {"dPhi", "dPhiref"},
                    outfolder + "dPhi.png",
                    "dPhi",
                    "dN/d(dPhi)",
                    "Jet dPhi Distributions",
                    -TMath::Pi(), TMath::Pi(),
                    -999, -999,
                    false, false,
                     captionInfo);
    
    captionInfo = {tracklevelcut, 0.20, 0.60, 0.020, 0.04, true};
    DrawAndSave({hxvtx},
                    {"xvtx"},
                    outfolder + "xvtx.png",
                    "xvtx",
                    "dN/d(xvtx)",
                    "Vertex x Position Distributions",
                    -0.1, 0.1,
                    -999, -999,
                    false, false,
                    captionInfo); 

    DrawAndSave({hyvtx},
                    {"yvtx"},
                    outfolder + "yvtx.png",
                    "yvtx",
                    "dN/d(yvtx)",
                    "Vertex y Position Distributions",
                    -0.1, 0.1,
                    -999, -999,
                    false, false,
                    captionInfo); 

    DrawAndSave({hzvtx},
                    {"zvtx"},
                    outfolder + "zvtx.png",
                    "zvtx",
                    "dN/d(zvtx)",
                    "Vertex z Position Distributions",
                    -15, 15,
                    -999, -999,
                    false, false,
                    captionInfo);

    TCanvas* c = new TCanvas("c", "canvas", 800, 800);
    c->SetTicks(1,1);
    c->SetLeftMargin(0.14);
    c->SetBottomMargin(0.12);
    c->SetTopMargin(0.08);
    c->SetRightMargin(0.14);
    hpthatvjtpt->SetTitle("pthat vs Leading Jet pT; pthat; Jet pT");
    hpthatvjtpt->Draw("COLZ");
    c->SaveAs((outfolder + "pthatvjtpt.png").c_str());

    file->Close();

    return 0;
}
