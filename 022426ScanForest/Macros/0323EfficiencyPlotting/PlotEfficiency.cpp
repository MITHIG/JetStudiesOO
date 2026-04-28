#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>
#include <vector>
#include <TMath.h>
#include <TCanvas.h>

static void FormatCanvas(TCanvas* c, bool logx, bool logy) {
  c->SetTicks(1,1);
  c->SetLeftMargin(0.14);
  c->SetBottomMargin(0.12);
  c->SetTopMargin(0.08);
  c->SetRightMargin(0.05);
  gPad->SetGrid();

  if (logx) c->SetLogx();
  if (logy) c->SetLogy();
}
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

static void StyleHist(TH1* h,
    int color,
    int marker){
  h->SetLineColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(marker);
  h->SetLineWidth(2);
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

int PlotEfficiency(){

    string outfiletag = "DataEfficiencyDebug";

    TFile* f = TFile::Open("/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/EfficiencyFiles/EfficiencyResults_DataEfficiencyDebug.root", "READ");
    if (!f || f->IsZombie()) {
        cout << "Error: Could not open file!" << endl;
        return 1;
    }
    TTree* t = (TTree*)f->Get("t");
    TNamed* cutInfo = (TNamed*)f->Get("CutInfo");
    if (!t || !cutInfo) {
        cout << "Error: Could not retrieve TTree or CutInfo from file!" << endl;
        return 1;
    }

    cout << "Obtained CutInfo: " << cutInfo->GetTitle() << endl;

    vector<string>* EventCutLabels = nullptr;
    vector<Long64_t>* EventCutValues = nullptr;
    vector<string>* JetCutLabels = nullptr;
    vector<Long64_t>* JetCutValues = nullptr;

    t->SetBranchAddress("EventCutLabels", &EventCutLabels);
    t->SetBranchAddress("EventCutValues", &EventCutValues);
    t->SetBranchAddress("JetCutLabels", &JetCutLabels);
    t->SetBranchAddress("JetCutValues", &JetCutValues);
    cout << "Set branch addresses for EventCutLabels, EventCutValues, JetCutLabels, and JetCutValues." << endl;
    
    t->GetEntry(0);
    std::string nentriesLabel;
    int nEntries = EventCutValues->at(0);
    nentriesLabel = FormatNEntriesLabel(nEntries);
    cout << "Formatted nEntries label: " << nentriesLabel << endl;

    BlockCaptionInfo captionInfo;
    vector<std::string> cutInfoDataVector;

    cutInfoDataVector.push_back(Form("NEvents: %s", nentriesLabel.c_str()));
    std::stringstream ssCut(cutInfo->GetTitle());
    std::string item;
    while (std::getline(ssCut, item, ';')) cutInfoDataVector.push_back(item);

    captionInfo.text = cutInfoDataVector;
    captionInfo.x = 0.50;
    captionInfo.y = 0.80;
    captionInfo.size = 0.03;
    captionInfo.spacing = 0.03;
    captionInfo.useNDC = true;


    TCanvas* c1 = new TCanvas("c1", "Efficiency Summary", 800, 800);
    c1->cd();

    FormatCanvas(c1, false, false);
    TH1F* h = new TH1F("h", "h", 13, 0.5, 13.5);
    h->GetYaxis()->SetTitle("Number of Jets");
    h->SetTitle("Jet Selection Efficiency Summary: 1PD");
    // Draw the histogram
    h->Draw("hist");

    DrawTLatexLines(captionInfo);

    TLatex latex;
    latex.SetTextSize(0.02);
    float offset = 0.35; // Adjust this value to position the labels better
    for (size_t i = 0; i < JetCutLabels->size(); i++) {
        h->SetBinContent(i + 1, JetCutValues->at(i));
        h->GetXaxis()->SetBinLabel(i + 1, JetCutLabels->at(i).c_str());
        double x = h->GetBinCenter(i+1);
        double y = h->GetBinContent(i+1);
        string label = FormatNEntriesLabel(JetCutValues->at(i));
        cout << "Bin " << i + 1 << ": " << JetCutLabels->at(i) << " = " << label << endl;
        latex.DrawLatex(x-offset, y, label.c_str());
    }


    TCanvas* c2 = new TCanvas("c2", "Event Cut Efficiency", 800, 800);
    c2->cd();

    FormatCanvas(c2, false, false);

    TH1F* h_percentage = new TH1F("h_percentage", "h_percentage", 2, 0.5, 2.5);
    h_percentage->GetYaxis()->SetTitle("Efficiency (%)");
    h_percentage->SetTitle("Event Selection Efficiency");
    for (size_t i = 0; i < JetCutLabels->size(); i++) {
        h_percentage->SetBinContent(i+1, (float)JetCutValues->at(i)/JetCutValues->at(0)*100.0);
        h_percentage->GetXaxis()->SetBinLabel(i + 1, JetCutLabels->at(i).c_str());
        double x = h_percentage->GetBinCenter(i+1);
        double y = h_percentage->GetBinContent(i+1);
        string label = Form("%.2f%%", (double)JetCutValues->at(i)/JetCutValues->at(0)*100.0);
        cout << "Bin " << i + 1 << ": " << JetCutLabels->at(i) << " = " << label << endl;
        latex.DrawLatex(x-offset, y, label.c_str());
    }


    gStyle->SetPaintTextFormat(".2f");

    h_percentage->Draw("hist text0");

    c1->SaveAs(Form("/home/xirong/JetStudiesOO/022426ScanForest/Plots/EfficiencyPlots/0323JetSelEfficiency/EfficiencySummary_%s_%s.png", outfiletag.c_str(), nentriesLabel.c_str()));
    c2->SaveAs(Form("/home/xirong/JetStudiesOO/022426ScanForest/Plots/EfficiencyPlots/0323JetSelEfficiency/EventCutEfficiencyPercent_%s_%s.png", outfiletag.c_str(), nentriesLabel.c_str()));
     
    return 0;
}