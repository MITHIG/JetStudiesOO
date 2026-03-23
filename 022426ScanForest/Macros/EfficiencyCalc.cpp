#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <map>
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


int EfficiencyCalc(){

    string outfolder = "/home/xirong/JetStudiesOO/022426ScanForest/Plots/0319DebugPlots/";

    string inputFileDATA = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0318OnePD/JetSubHist_Data_93.4MEvts_Data1PD_withjetcuts_20260319.root";
    string inputFileMCpthat15 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/0318OnePD/JetSubHist_MC_99.7MEvts_pthat151PD_withjetcuts_20260319.root";
    string inputFileMCpthat0 = "/home/xirong/JetStudiesOO/022426ScanForest/RootFiles/031726_100KUpdatedKt/JetSubHist_MC_31KEvts_MinBiaspthat0_noptcut_20260317_173426.root";

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
    TNamed* generalInfoMCpthat15 = (TNamed*)fileMCpthat15->Get("GeneralInfo");
    //TNamed* generalInfoMCpthat0 = (TNamed*)fileMCpthat0->Get("GeneralInfo");

  
    cout << "Debug: Cuts applied in Data: " << cutInfoData->GetTitle() << endl;
    cout << "Debug: Cuts applied in MC pthat15: " << cutInfoMCpthat15->GetTitle() << endl;
    cout << "Debug: Cuts applied in MC pthat0: " << cutInfoMCpthat0->GetTitle() << endl;
    cout << "GeneralInfo Data:" << generalInfoData->GetTitle() << endl;
    cout << "GeneralInfo MC pthat15:" << generalInfoMCpthat15->GetTitle() << endl;
    //cout << "GeneralInfo MC pthat0:" << generalInfoMCpthat0->GetTitle() << endl;
    cout << "Debug: Data histogram entries:" << endl;

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


  std::map<std::string, double> result;

  for (const auto& s : generalInfoDataVector) {
      std::stringstream ss(s);

      std::string word1, word2, word3, tag;
      double value;

      ss >> tag >> value;

      cout << "Parsed tag: " << tag << ", value: " << value << endl;

      if (!ss.fail()) {
          result[tag] = value;
      }
  }
    fileDATA->Close();
    fileMCpthat15->Close();
    fileMCpthat0->Close();
    return 0;
}
