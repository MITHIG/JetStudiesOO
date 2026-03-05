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
    string inputFileName = "../RootFiles/HistTFiles/JetSubstructureHistogramChain0304.root";

    TFile *file = TFile::Open(inputFileName.c_str());
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << inputFileName << std::endl;
        return 1;
    
    
    }

    TNamed* cutInfo = (TNamed*)file->Get("Cuts");
    TDirectory *dir = (TDirectory*)file->Get("EventLevelHistRaw");

    TH1F *hjtpt1 = (TH1F*)file->Get("EventLevelHistRaw/hjtpt1");
    TH1F *hjtpt2 = (TH1F*)file->Get("EventLevelHistRaw/hjtpt2");
    TH1F *hAj    = (TH1F*)file->Get("EventLevelHistRaw/hAj");
    TH1F *hAjref = (TH1F*)file->Get("EventLevelHistRaw/hAjref");
    TH1F *hXj    = (TH1F*)file->Get("EventLevelHistRaw/hXj");
    TH1F *hXjref = (TH1F*)file->Get("EventLevelHistRaw/hXjref");
    TH1F *hdPhi   = (TH1F*)file->Get("EventLevelHistRaw/hdPhi");
    TH1F *hdPhiref   = (TH1F*)file->Get("EventLevelHistRaw/hdPhiref");

    TH1F *hjtpt   = (TH1F*)file->Get("JetLevelHistRaw/hjtpt");
    TH1F *hjteta  = (TH1F*)file->Get("JetLevelHistRaw/hjteta");
    TH1F *hjtphi  = (TH1F*)file->Get("JetLevelHistRaw/hjtphi");
    TH1F *hjtsym  = (TH1F*)file->Get("JetLevelHistRaw/hjtsym");
    TH1F *hjtrg   = (TH1F*)file->Get("JetLevelHistRaw/hjtrg");
    TH1F *hjtdynkt= (TH1F*)file->Get("JetLevelHistRaw/hjtdynkt");
    TH1F *hjtangu = (TH1F*)file->Get("JetLevelHistRaw/hjtangu");


    TH1F *hrefpt   = (TH1F*)file->Get("JetLevelHistRaw/hrefpt");
    TH1F *hrefeta  = (TH1F*)file->Get("JetLevelHistRaw/hrefeta");
    TH1F *hrefphi  = (TH1F*)file->Get("JetLevelHistRaw/hrefphi");
    TH1F *hrefsym  = (TH1F*)file->Get("JetLevelHistRaw/hrefsym");
    TH1F *hrefrg   = (TH1F*)file->Get("JetLevelHistRaw/hrefrg");
    TH1F *hrefdynkt= (TH1F*)file->Get("JetLevelHistRaw/hrefdynkt");
    TH1F *hrefangu = (TH1F*)file->Get("JetLevelHistRaw/hrefangu");

    vector<TH1F*> jetHists = {hjtpt, hjteta, hjtphi, hjtsym, hjtrg, hjtdynkt, hjtangu,
                                hAj, hXj, hdPhi};
    vector<TH1F*> refHists = {hrefpt, hrefeta, hrefphi, hrefsym, hrefrg, hrefdynkt, hrefangu,
                                hAjref, hXjref, hdPhiref};

  
    for (TH1F* h : jetHists) {
        StyleHist(h, kRed, 20);
        NormalizePerEvt(h,h->GetEntries());
    }
    for (TH1F* h : refHists) {
        StyleHist(h, kBlue, 24);
        NormalizePerEvt(h,h->GetEntries());
    }

    gStyle->SetOptStat(0); // Disable statistics box
    string outfolder = "../Plots/0224JetPlotswithoutCut/";

    BlockCaptionInfo captionInfo;
    vector<std::string> cutInfoVector;

    std::stringstream ss(cutInfo->GetTitle());
    std::string item;
    while (std::getline(ss, item, ';')) cutInfoVector.push_back(item);

    vector<std::string> jetlevelcut = cutInfoVector;
    vector<std::string> observlevelcut = cutInfoVector;

    jetlevelcut.push_back(Form("NEvents: %.0f", hjtpt->GetEntries())); 
    observlevelcut.push_back(Form("NEvents: %.0f", hAj->GetEntries()));


    captionInfo = {jetlevelcut, 0.20, 0.44, 0.030, 0.04, true};
    DrawAndSave({hjtpt, hrefpt},
                 {"Reconstructed Jet", "Reference Jet"},
                outfolder + "JetPt.png",
                "pT",
                "dN/dpT",
                "Jet pT Distributions",
                0, 1600,
                -999, -999,
                false, true,
                captionInfo);

    captionInfo = {jetlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hjteta, hrefeta},
                    {"Reconstructed Jet", "Reference Jet"},
                    outfolder + "JetEta.png",
                    "eta",
                    "dN/deta",
                    "Jet eta Distributions",
                    -2.1, 2.1,
                    -999, -999,
                    false, false,
                    captionInfo);

    captionInfo = {jetlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hjtphi, hrefphi},
                    {"Reconstructed Jet", "Reference Jet"},
                    outfolder + "JetPhi.png",
                    "phi",
                    "dN/dphi",
                    "Jet phi Distributions",
                    -TMath::Pi(), TMath::Pi(),
                    -999, -999,
                    false, false,
                    captionInfo); 

    captionInfo = {jetlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hjtsym, hrefsym},
                    {"Reconstructed Jet", "Reference Jet"},
                        outfolder + "JetSymmetry.png",
                        "Symmetry",
                        "dN/dSymmetry",
                        " Jet Symmetry Distributions",
                        0, 2,
                        -999, -999,
                        false, false,
                        captionInfo);

    captionInfo = {jetlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hjtrg, hrefrg},
                    {"Reconstructed Jet", "Reference Jet"},
                        outfolder + "JetRG.png",
                        "RG",
                        "dN/dRG",
                        "Leading and Subleading Jet RG Distributions",
                        0, 1,
                        -999, -999,
                        false, false,
                        captionInfo);

    captionInfo = {jetlevelcut, 0.46, 0.70, 0.035, 0.04, true};
    DrawAndSave({hjtdynkt, hrefdynkt},
                    {"Reconstructed Jet", "Reference Jet"},
                        outfolder + "JetDynKt.png",
                        "DynKt",
                        "dN/dDynKt",
                        "Jet DynKt Distributions",
                        0, 150,
                        -999, -999,
                        false, true,
                        captionInfo);

    captionInfo = {jetlevelcut, 0.45, 0.54, 0.035, 0.04, true};
    DrawAndSave({hjtangu, hrefangu},
                    {"Reconstructed Jet", "Reference Jet"},
                        outfolder + "JetAngularity.png",
                        "Angularity",
                        "dN/dAngularity",
                        "Jet Angularity Distributions",
                        0, 1,
                        -999, -999,
                        false, false,
                        captionInfo);

    captionInfo = {observlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hAj, hAjref},
                    {"Reconstructed Jet", "Reference Jet"},
                    outfolder + "DiJetAsymmetry.png",
                    "A_{J}",
                    "dN/dA_{J}",
                    "A_{J} Asymmetry Distributions",
                    0, 1,
                    -999, -999,
                    false, false,
                    captionInfo);

    captionInfo = {observlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hXj, hXjref},
                    {"Reconstructed Jet", "Reference Jet"},
                    outfolder + "JetMomentumBalance.png",
                    "X_{J}",
                    "dN/dX_{J}",
                    "Jet Momentum Balance Distributions",
                    0, 1,
                    -999, -999,
                    false, false, 
                    captionInfo);

    captionInfo = {observlevelcut, 0.20, 0.54, 0.035, 0.04, true};
    DrawAndSave({hdPhi, hdPhiref},
                    {"Reconstructed Jet", "Reference Jet"},
                    outfolder + "JetDeltaPhi.png",
                    "#Delta#phi",
                    "dN/d#Delta#phi",
                    "Jet #Delta#phi Distributions",
                    0, TMath::Pi(),
                    -999, -999,
                    false, false,
                    captionInfo);

    TH1F* ptRatio = (TH1F*)hjtpt->Clone("ptRatio");
    ptRatio->Divide(hrefpt);
    TH1F* etaRatio = (TH1F*)hjteta->Clone("etaRatio");
    etaRatio->Divide(hrefeta);
    TH1F* phiRatio = (TH1F*)hjtphi->Clone("phiRatio");
    phiRatio->Divide(hrefphi);
    TH1F* symRatio = (TH1F*)hjtsym->Clone("symRatio");
    symRatio->Divide(hrefsym);
    TH1F* rgRatio = (TH1F*)hjtrg->Clone("rgRatio");
    rgRatio->Divide(hrefrg);
    TH1F* dynktRatio = (TH1F*)hjtdynkt->Clone("dynktRatio");
    dynktRatio->Divide(hrefdynkt);
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
    DrawAndSave({symRatio},
                    {"Jet Symmetry / Reference Jet Symmetry"},
                    outfolder + "JetSymmetryRatio.png",
                    "Symmetry Ratio",
                    "dN/d(Symmetry Ratio)",
                    "Jet Symmetry Ratio Distributions",
                    -999,-999,
                    0, 2);
    DrawAndSave({rgRatio},
                    {"Jet RG / Reference Jet RG"},
                    outfolder + "JetRGRatio.png",
                    "RG Ratio",
                    "dN/d(RG Ratio)",
                    "Jet RG Ratio Distributions",
                    -999,-999,
                    0, 2);
    DrawAndSave({dynktRatio},
                    {"Jet DynKt / Reference Jet DynKt"}, 
                    outfolder + "JetDynKtRatio.png",
                    "DynKt Ratio",
                    "dN/d(DynKt Ratio)",
                    "Jet DynKt Ratio Distributions",
                    0,150,
                    0, 2);
    DrawAndSave({anguRatio},
                    {"Jet Angularity / Reference Jet Angularity"},
                    outfolder + "JetAngularityRatio.png",
                    "Angularity Ratio",
                    "dN/d(Angularity Ratio)",
                    "Jet Angularity Ratio Distributions",
                    -999,-999,
                    0, 2);


    file->Close();
    return 0;
}
