#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TLegend.h>
#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include "RooUnfold.h"
#include "RooUnfoldResponse.h"
#include "RooUnfoldBayes.h"
#include <TStyle.h>
#include <TColor.h>

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
  double ly1 = ly2 - 0.03 * std::max<size_t>(n, 1);
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

TH1D* CreatePlotHist(TH1* original, const string& name, bool normalizeByWidth = true) {
    TH1D* plotHist = (TH1D*)original->Clone(name.c_str());
    if (normalizeByWidth) {
        plotHist->Scale(1.0 / plotHist->GetSumOfWeights(), "width");
    } else {
        plotHist->Scale(1.0 / plotHist->GetSumOfWeights());
    }
    return plotHist;
}



void DrawAndSaveNormalized(const std::vector<TH1*>& originals,
                          const std::vector<std::string>& labels,
                          const std::string& outPng,
                          const std::string& xTitle,
                          const std::string& yTitle = "Normalized entries",
                          const std::string& title = "",
                          float xMin = -999, float xMax = -999,
                          float yMin = -999, float yMax = -999,
                          bool logx = false,
                          bool logy = false,
                          bool normalizeByWidth = true,
                          BlockCaptionInfo captionInfo = {}) {
    
    std::vector<TH1*> plotHists;
    for (size_t i = 0; i < originals.size(); ++i) {
        if (originals[i]) {
            string plotName = originals[i]->GetName() + string("_plot_") + to_string(i);
            TH1D* plotHist = CreatePlotHist(originals[i], plotName, normalizeByWidth);
            plotHists.push_back(plotHist);
        } else {
            plotHists.push_back(nullptr);
        }
    }
    
    DrawAndSave(plotHists, labels, outPng, xTitle, yTitle, title, 
                xMin, xMax, yMin, yMax, logx, logy, captionInfo);
    
    // Cleanup
    for (auto* h : plotHists) {
        if (h) delete h;
    }
}

static void StyleHist(TH1* h, int color, int marker = 20, int lineStyle = 1){
  h->SetLineColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(marker);
  h->SetLineWidth(2);
  h->SetLineStyle(lineStyle);
}

TH1D* Unfold(TH1* hInput,
            int nIterations,
            TH1* hMatchedRecoPt,
            TH1* hMatchedGenPt,
            TH1* hRecoPt,
            TH1* hGenPt,
            RooUnfoldResponse* response_pt) {

    TH1D* hGenResponse = (TH1D*)response_pt->Htruth();
    TH1D* hRecoResponse = (TH1D*)response_pt->Hmeasured();
    TH1D* hPurityPt = (TH1D*)hMatchedRecoPt->Clone("hPurityPt");
    hPurityPt->Divide(hRecoPt); // hMatched/hRecoPt

    TH1D* hEfficiencyPt = (TH1D*)hMatchedGenPt->Clone("hEfficiencyPt");
    hEfficiencyPt->Divide(hGenPt); // hMatched/hGenPt ()
 //   hEfficiencyPt->Print("all");

    TH1D* hPurityCorrectedPt = (TH1D*)hInput->Clone("hPurityCorrectedPt");
    hPurityCorrectedPt->Multiply(hPurityPt); // hRecoPt * (hMatchedPt/hRecoPt) = hMatchedPt

    RooUnfoldBayes unfold(response_pt, hPurityCorrectedPt, nIterations);

    TH1D* hUnfolded = (TH1D*)unfold.Hreco()->Clone("hUnfolded");
    TH1D* hUnfoldFinal = (TH1D*)hUnfolded->Clone("hUnfoldFinal");
    hUnfoldFinal->Divide(hEfficiencyPt);
    cout << "Unfolding complete!" << endl;

    return hUnfoldFinal;
}

TH1D* Unfold_nocorr(TH1* hInput,
            int nIterations,
            RooUnfoldResponse* response_pt) {

    RooUnfoldBayes unfold(response_pt, hInput, nIterations);
    TH1D* hUnfolded = (TH1D*)unfold.Hreco()->Clone("hUnfolded");
   
    return hUnfolded;
}


TH1D* Refold(TH1* hUnfolded,
            TH1* hMatchedRecoPt,
            TH1* hMatchedGenPt,
            TH1* hRecoPt,
            TH1* hGenPt,
            RooUnfoldResponse* response_pt) {

    TH1D* hGenResponse = (TH1D*)response_pt->Htruth();
    TH1D* hRecoResponse = (TH1D*)response_pt->Hmeasured();
    TH1D* hPurityPt = (TH1D*)hMatchedRecoPt->Clone("hPurityPt");
    hPurityPt->Divide(hRecoPt); // hMatched/hRecoPt
    TH1D* hEfficiencyPt = (TH1D*)hMatchedGenPt->Clone("hEfficiencyPt");
    hEfficiencyPt->Divide(hGenPt); // hMatched/hGenPt

    TH1D* hUnfoldedRaw = (TH1D*)hUnfolded->Clone("hUnfoldedRaw");
    hUnfoldedRaw->Multiply(hEfficiencyPt);

    TH1D* hRefolded = (TH1D*)response_pt->ApplyToTruth(hUnfoldedRaw)->Clone("hRefolded");
    TH1D* hRefoldedFinal = (TH1D*)hRefolded->Clone("hRefoldedFinal");
    hRefoldedFinal->Divide(hPurityPt);

    return hRefoldedFinal;
}
template <typename T>
void CheckFileExist(T* obj, const std::string& filename = "") {
    if (!obj) {
        cout << "Error: Could not open file" << filename << endl;
        exit(1);
    }
}

static int RainbowColor(int i, int n) {
  double hue = (n == 1) ? 0.0 : (double)i / (double)(n);

  Float_t r, g, b;

  TColor::HLStoRGB(hue * 360.0, 0.5, 0.5, r, g, b);
  return TColor::GetColor(r, g, b);
}

int PlotUnfoldingHalfHalf(){
    gStyle->SetOptStat(0);

    string MCFile = "/home/xirong/JetStudiesOO/LynnsCode/RootFiles/0428RooUnfoldVer2/Response_MC_99.7MEvts_MC_FullPD_20260429.root";
    string outfolder = "/home/xirong/JetStudiesOO/LynnsCode/Plots/Response/0428UnfoldingRooUnfold2";
    string outFileTag = "5050test_FullPD";
    TFile *MCFilef = TFile::Open(MCFile.c_str(), "READ");
    CheckFileExist(MCFilef, MCFile);

    TH1D* hGenPt = (TH1D*)MCFilef->Get("hGenPt");
    CheckFileExist(hGenPt, "hGenPt");
    TH1D* hGenNoWeightPt = (TH1D*)MCFilef->Get("hGenNoWeight");
    CheckFileExist(hGenNoWeightPt, "hGenNoWeightPt");
    TH1D* hRecoNoWeightPt = (TH1D*)MCFilef->Get("hRecoNoWeight");
    CheckFileExist(hRecoNoWeightPt, "hRecoNoWeightPt");
    TH1D* hRecoPt = (TH1D*)MCFilef->Get("hRecoPt");
    CheckFileExist(hRecoPt, "hRecoPt");
    TH1D* hFakePt = (TH1D*)MCFilef->Get("hFakePt");
    CheckFileExist(hFakePt, "hFakePt");
    TH1D* hMatchedRecoPt = (TH1D*)MCFilef->Get("hMatchedRecoPt");
    CheckFileExist(hMatchedRecoPt, "hMatchedRecoPt");
    TH1D* hMatchedGenPt = (TH1D*)MCFilef->Get("hMatchedGenPt");
    CheckFileExist(hMatchedGenPt, "hMatchedGenPt");
    TH1D* hGenPt2 = (TH1D*)MCFilef->Get("hGenPt2");
    CheckFileExist(hGenPt2, "hGenPt2");
    TH1D* hGenNoWeightPt2 = (TH1D*)MCFilef->Get("hGenNoWeight2");
    CheckFileExist(hGenNoWeightPt2, "hGenNoWeight2");
    TH1D* hRecoNoWeightPt2 = (TH1D*)MCFilef->Get("hRecoNoWeight2");
    CheckFileExist(hRecoNoWeightPt2, "hRecoNoWeight2");
    TH1D* hRecoPt2 = (TH1D*)MCFilef->Get("hRecoPt2");
    CheckFileExist(hRecoPt2, "hRecoPt2");
    TH1D* hFakePt2 = (TH1D*)MCFilef->Get("hFakePt2");
    CheckFileExist(hFakePt2, "hFakePt2");
    TH1D* hMatchedRecoPt2 = (TH1D*)MCFilef->Get("hMatchedRecoPt2");
    CheckFileExist(hMatchedRecoPt2, "hMatchedRecoPt2");
    TH1D* hMatchedGenPt2 = (TH1D*)MCFilef->Get("hMatchedGenPt2");
    CheckFileExist(hMatchedGenPt2, "hMatchedGenPt2");
    RooUnfoldResponse* response_pt = (RooUnfoldResponse*)MCFilef->Get("response_pt");
    TH1D* hGenResponse = (TH1D*)response_pt->Htruth();
    CheckFileExist(hGenResponse, "hGenResponse");
    TH1D* hRecoResponse = (TH1D*)response_pt->Hmeasured();
    CheckFileExist(hRecoResponse, "hRecoResponse");
    
    TH1D* hPurityPt = (TH1D*)hMatchedRecoPt->Clone("hPurityPt");
    hPurityPt->Divide(hRecoPt); // hMatched/hRecoPt
    TH1D* hEfficiencyPt = (TH1D*)hMatchedGenPt->Clone("hEfficiencyPt");
    hEfficiencyPt->Divide(hGenPt); // hMatched/hGenPt ()

    TH1D* hUnfolded_trivial =  Unfold(hRecoPt, 1, hMatchedRecoPt, hMatchedGenPt, hRecoPt, hGenPt, response_pt);
    TH1D* hUnfolded_nocorr_trivial = Unfold_nocorr(hRecoResponse, 1, response_pt);

    TH1D* hRefolded_trivial = Refold(hUnfolded_trivial, hMatchedRecoPt, hMatchedGenPt, hRecoPt, hGenPt, response_pt);
    TH1D* hRefolded_nocorr_trivial = Refold(hUnfolded_nocorr_trivial, hMatchedRecoPt, hMatchedGenPt, hRecoPt, hGenPt, response_pt);

    string outfoldertemp = outfolder + "IndividualDebug/";
    vector<TH1*> hUnfoldedIters;
    vector<TH1*> hUnfolded_onlyCorr1s;
    vector<TH1*> hUnfolded_onlyCorr2s;
    vector<TH1*> hUnfolded_nocorrs;

    vector<std::string> iterLabels;
    vector<TH1*> hUnfoldedIterRatios;
    vector<TH1*> hUnfolded_nocorrs_ratios;
    vector<TH1*> hUnfolded_onlyCorr1_ratios;
    vector<TH1*> hUnfolded_onlyCorr2_ratios;
    int n = 4; // number of iterations to test

    for (int nIterations = 1; nIterations <= n; ++nIterations) {
        cout << "Performing unfolding with " << nIterations << " iterations..." << endl;
        TH1D* hUnfoldedIter = Unfold(hRecoPt2, nIterations, hMatchedRecoPt, hMatchedGenPt, hRecoPt, hGenPt2, response_pt);
        TH1D* hUnfolded_nocorr = Unfold_nocorr(hMatchedRecoPt2, nIterations, response_pt);
        int col = RainbowColor(nIterations, n);
        StyleHist(hUnfoldedIter, col, 24 + (nIterations % 5),2);
        StyleHist(hUnfolded_nocorr, col, 24 + (nIterations % 5), 2);
        hUnfoldedIters.push_back(hUnfoldedIter);        
        hUnfolded_nocorrs.push_back(hUnfolded_nocorr);
        iterLabels.push_back("Unfolded (Iter " + to_string(nIterations) + ")");
    }

    for (int i = 0; i < hUnfoldedIters.size(); ++i) {
        TH1D* hRatioIter = (TH1D*)hUnfoldedIters[i]->Clone("hRatioIter");
        TH1D* hRatioIter_nocorr = (TH1D*)hUnfolded_nocorrs[i]->Clone("hRatioIter_nocorr");

        hRatioIter->Divide(hGenPt2);
        hRatioIter_nocorr->Divide(hMatchedGenPt2);

        StyleHist(hRatioIter, RainbowColor(i+1, n), 20 + ((i+1) % 5));
        hUnfoldedIterRatios.push_back(hRatioIter);
        hUnfolded_nocorrs_ratios.push_back(hRatioIter_nocorr);

    }

    StyleHist(hGenPt, kBlue);
    StyleHist(hRecoPt, kRed);
    StyleHist(hMatchedRecoPt, kRed+1, 24, 1);
    StyleHist(hMatchedGenPt, kBlue+1, 24, 1);
    StyleHist(hUnfolded_trivial, kGreen+2);
    StyleHist(hRefolded_trivial, kMagenta+2);
    StyleHist(hMatchedGenPt2, kBlue+1, 24, 1);
    StyleHist(hGenPt2, kBlue);
    StyleHist(hMatchedRecoPt2, kRed+1, 24, 1);
    StyleHist(hRecoPt2, kRed);
    
    // TRIVIAL TEST
    CheckFileExist(hGenPt, "hGenPt");
    CheckFileExist(hRecoPt, "hRecoPt");
    CheckFileExist(hMatchedRecoPt, "hMatchedRecoPt");
    CheckFileExist(hMatchedGenPt, "hMatchedGenPt");
    CheckFileExist(hUnfolded_trivial, "hUnfolded_trivial");
    CheckFileExist(hRefolded_trivial, "hRefolded_trivial");

    cout << "Trivial Unfolding: Checking Gen and Reco Ratios:" << endl;
    TH1D* hResponseGen = (TH1D*)response_pt->Htruth();
    TH1D* hResponseReco = (TH1D*)response_pt->Hmeasured();
    for (int i = 1; i <= hResponseGen->GetNbinsX(); ++i) {
        double response_gen = hResponseGen->GetBinContent(i);
        double response_reco = hResponseReco->GetBinContent(i);
        double gen = hMatchedGenPt->GetBinContent(i);
        double reco = hMatchedRecoPt->GetBinContent(i);
        cout << "Bin " << i << ": Ratio Gen: " << (gen > 0 ? response_gen / gen : 0) 
             << ", Ratio Reco: " << (reco > 0 ? response_reco / reco : 0) << endl;

        double unfolded_value = hUnfolded_nocorr_trivial->GetBinContent(i);
        double ratio_unfoldedovergen = (gen > 0) ? unfolded_value / gen : 0;
        double ratio_unfoldedoverresponsegen = (response_gen > 0) ? unfolded_value / response_gen : 0;
        cout << "       Unfolded/Gen Ratio: " << ratio_unfoldedovergen << ", Unfolded/ResponseGen Ratio: " << ratio_unfoldedoverresponsegen << endl;
    }


    StyleHist(hMatchedRecoPt, kRed+1, 24, 2);
    StyleHist(hMatchedGenPt, kBlue+1, 24, 2);
    StyleHist(hRecoPt, kRed, 24,1);
    StyleHist(hGenPt, kBlue, 24,1);

    DrawAndSaveNormalized({hGenPt, hRecoPt, hMatchedRecoPt, hMatchedGenPt,hUnfolded_trivial, hRefolded_trivial}, 
                {"Gen Pt", "Reco Pt", "Reco Pt (w Match)", "Gen Pt (w Match)", "Unfolded Pt", "Refolded Pt"},
                outfolder + "/" + outFileTag + "_TrivialTest.png",
                "p_{T} (GeV/c)", "Entries", "Input Histograms",
                -999, -999,
                1e-6, 1,
                false,true
                );

    TH1D* hRatio_trivial = (TH1D*)hUnfolded_trivial->Clone("hRatio_trivial");
    TH1D* hRatio_nocorr_trivial = (TH1D*)hUnfolded_nocorr_trivial->Clone("hRatio_nocorr_trivial");
    hRatio_nocorr_trivial->Divide(hGenResponse);
    hRatio_trivial->Divide(hGenPt);
    StyleHist(hRatio_trivial, kGreen+2);
    StyleHist(hRatio_nocorr_trivial, kGreen+2);

    cout << "Trivial Unfolding ratio (Unfolded/Gen) values:" << endl;
    for (int i = 1; i <= hRatio_trivial->GetNbinsX(); ++i) {
        double ratio = hRatio_trivial->GetBinContent(i);
        double ratio_nocorr = hRatio_nocorr_trivial->GetBinContent(i);
        cout << "Bin " << i << ": " << ratio << " (No Corr: " << ratio_nocorr << ")" << endl;
    }

    DrawAndSave({hMatchedRecoPt, hMatchedGenPt, hRecoPt, hGenPt}, 
                {"Matched Reco Pt", "Matched Gen Pt", "Reco Pt", "Gen Pt"},
                outfolder + "/" + outFileTag + "RecoGen Comparison.png",
                "p_{T} (GeV/c)", "Entries", "Matched vs All",
                -999, -999,
                1e-6, 10000,
                false,true
                );
    DrawAndSave({hRatio_trivial}, {"Unfolded/Gen (Trivial)"}, 
                outfolder + "/" + outFileTag + "_TrivialRatio.png",
                "p_{T} (GeV/c)", "Ratio", "Ratio of Unfolded to Gen (Trivial)",
                -999, -999,
                0.9999, 1.0001,
                false,false
                );

    /// Iteration check
    
    StyleHist(hGenPt2, kBlue);
    StyleHist(hRecoPt2, kRed);

    vector<string> iterLabels2 = iterLabels;
    vector<string> iterLabels3 = iterLabels;
    hUnfoldedIters.push_back(hGenPt2);
    iterLabels.push_back("Gen");
    hUnfoldedIters.push_back(hRecoPt2);
    iterLabels.push_back("Reco");

    hUnfolded_nocorrs.push_back(hMatchedGenPt2);
    hUnfolded_nocorrs.push_back(hMatchedRecoPt2);
    

    DrawAndSaveNormalized(hUnfoldedIters, iterLabels,
                  outfolder + "/" + outFileTag + "_UnfoldedIters.png",
                  "p_{T} (GeV/c)", "dN/dp_{T}",
                  "Unfolded p_{T} distribution with different iterations",
                  -999, -999,
                  1e-6, 1,
                  false,true
                  );

    DrawAndSave(hUnfoldedIterRatios, iterLabels,
                  outfolder + "/" + outFileTag + "_UnfoldedIterRatios.png",
                  "p_{T} (GeV/c)", "Ratio to Gen",
                  "Ratio of Unfolded p_{T} distribution to Gen with different iterations",
                  -999, -999,
                  0, 2,
                  false,false
                  );

    DrawAndSaveNormalized(hUnfolded_nocorrs, iterLabels,
      outfolder + "/" + outFileTag + "_Unfolded_nocorrs.png",
      "p_{T} (GeV/c)", "dN/dp_{T}",
      "Unfolded p_{T} distribution with different iterations (No Correction)",
      -999, -999,
      1e-6, 1,
      false,true
      );

    DrawAndSave(hUnfolded_nocorrs_ratios, iterLabels,
      outfolder + "/" + outFileTag + "_Unfolded_nocorrs_ratios.png",
      "p_{T} (GeV/c)", "Ratio to Gen",
      "Ratio of Unfolded p_{T} distribution to Gen with different iterations (No Correction)",
      -999, -999,
      0, 2,
      false,false
      );

    StyleHist(hPurityPt, kRed, 20, 1);
    StyleHist(hEfficiencyPt, kBlue, 21, 2);
    DrawAndSave({hPurityPt, hEfficiencyPt}, {"Purity", "Efficiency"},
                outfolder + "/" + outFileTag + "_Purity_Efficiency.png",
                "p_{T} (GeV/c)", "Correction Factor",
                "Purity and Efficiency vs p_{T}",
                -999, -999,
                0, 1.5,
                false,false
    );
       cout << "Number of Events in Each Bin:" << endl;
      for (int i = 1; i <= hRecoPt->GetNbinsX(); ++i) {
        cout << hRecoNoWeightPt->GetBinContent(i) << " (Train), " << hRecoNoWeightPt2->GetBinContent(i) << " (Test)" << endl;
      }

      cout << "Reco Absolute Error" << endl;
      for (int i = 1; i <= hRecoPt->GetNbinsX(); ++i) {
        double errTrain = hRecoPt->GetBinError(i);
        double errTest = hRecoPt2->GetBinError(i);
        cout << "Bin " << i << ": " << errTrain << " (Train), " << errTest << " (Test)" << endl;
      }

      TH1D* hRelErrRecoPt = (TH1D*)hRecoPt->Clone("hRelErrRecoPt");
      TH1D* hRelErrRecoPt2 = (TH1D*)hRecoPt2->Clone("hRelErrRecoPt2");
      cout << "Reco Relative Error" << endl;
      for (int i = 1; i <= hRecoPt->GetNbinsX(); ++i) {
        double relErr = hRecoPt->GetBinError(i)/hRecoPt->GetBinContent(i);
        double relErr2 = hRecoPt2->GetBinError(i)/hRecoPt2->GetBinContent(i);
        cout << "Bin " << i << ": " << relErr << " (Train), " << relErr2 << " (Test)" << endl;
        hRelErrRecoPt->SetBinContent(i, relErr);
        hRelErrRecoPt2->SetBinContent(i, relErr2);
      }

      StyleHist(hRelErrRecoPt, kRed, 20, 1);
      StyleHist(hRelErrRecoPt2, kBlue, 21, 2);

        DrawAndSave({hRelErrRecoPt, hRelErrRecoPt2}, {"Relative Error (Train)", "Relative Error (Test)"},
                  outfolder + "/" + outFileTag + "_RelativeError.png",
                  "p_{T} (GeV/c)", "Relative Error",
                  "Relative Error of Reco p_{T} distribution",
                  -999, -999,
                  0, 1,
                  false,false
    );
    TCanvas* c = new TCanvas("c", "Response Matrix", 800, 800);
    FormatCanvas(c, false, false);
    c->SetLogz();
    response_pt->Hresponse()->SetMinimum(1e-6);
    response_pt->Hresponse()->Draw("COLZ");
    response_pt->Hresponse()->GetXaxis()->SetTitle("Reco p_{T} (GeV/c)");
    response_pt->Hresponse()->GetYaxis()->SetTitle("Gen p_{T} (GeV/c)");
    response_pt->Hresponse()->SetTitle("Response Matrix: 1D jet pt");
    c->SaveAs((outfolder + "ResponseMatrix_pt_logz_"+ outFileTag +".png").c_str());

    return 0;
}
