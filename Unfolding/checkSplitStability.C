// Hannah Bossi <hannah.bossi@cern.ch>
// checkUnfoldingStab.C : Plots the ratio of unfolded distributions to the eigth iteration
// 02/12/2021       

void checkSplitStability(bool isTrivial = false){

   int colors[20] = {kRed+2, kRed-4, kOrange+7, kOrange, kYellow-4, kSpring+10, kSpring, kGreen-3, kGreen+3, kTeal-7, kTeal, kAzure+10, kAzure-4, kBlue+2, kViolet+8, kViolet-1, kMagenta+1, kMagenta-4, kPink+7, kPink-4};
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);

  //=============================== Do the main work =====================================
  TFile*_file0; 
  if(isTrivial)     _file0  = TFile::Open("Unfold2D_Trivial_042326.root");
  else              _file0  = TFile::Open("Unfold2D_SplitMCTest_042326.root");

  TH2D* denomHist2D = (TH2D*)_file0->Get("true"); 
  
  // Get the pT axis to determine all bins
  TAxis* pTAxis = denomHist2D->GetYaxis();
  int nPtBins = pTAxis->GetNbins();
  
  // Loop over all pT bins
  for(int ptBin = 1; ptBin <= nPtBins; ptBin++){
    
    // =================================== drawing ====================================
    TLegend* leg2 = new TLegend(0.65, 0.4, 0.85, 0.87);
    leg2->SetFillColor(0);
    leg2->SetBorderSize(0);
    leg2->SetTextFont(42);
    leg2->SetTextSize(0.04);

    TCanvas *c = new TCanvas("c", "canvas", 800, 800);
    // Upper plot will be in pad1
    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    pad1->SetBottomMargin(0); // Upper and lower plot are joined
    pad1->SetGridx();
    pad1->SetTicks(1,1);          // Vertical grid
    //pad1->SetLogy();
    pad1->Draw();             // Draw the upper pad: pad1
    pad1->cd();               // pad1 becomes the current pad
    
    // lower plot will be in pad2
    c->cd();          // Go back to the main canvas before defining pad2
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);
    pad2->SetGridx(); // vertical grid
    pad2->SetTicks(1,1); 
    pad2->Draw();
    pad2->cd();
    //==================================================================================

    // Get bin edges
    Double_t pTLow = pTAxis->GetBinLowEdge(ptBin);
    Double_t pTHigh = pTAxis->GetBinUpEdge(ptBin);
    
    // Project for this pT bin
    TH1D* denomHist = (TH1D*)denomHist2D->ProjectionX("denomHistProjY", ptBin, ptBin);
    denomHist->SetName("denomHist");
    denomHist->Scale(1./denomHist->Integral(), "width");
    denomHist->SetMarkerStyle(20);
    denomHist->SetMarkerColor(kBlue); 
    denomHist->SetLineWidth(2); 
    
    pad1->cd();
    denomHist->Draw("same"); 

    TLatex* cms = new TLatex(0.12,0.92,"#bf{CMS}         PYTHIA8 + HIJING OO(5.36 TeV)");
    cms->SetNDC();
    cms->SetTextSize(0.05);
    cms->SetTextFont(42);
    cms->Draw("same");
    
    // Add pT bin label to plot
    TLatex* ptLabel = new TLatex(0.15, 0.6, Form("%.0f < #it{p}_{T,jet} < %.0f GeV", pTLow, pTHigh));
    ptLabel->SetNDC();
    ptLabel->SetTextFont(42);
    ptLabel->SetTextSize(0.04);
    ptLabel->Draw("same");
    
    pad2->cd(); 
    std::stringstream ss;
    for(int i = 1; i < 15 ; i++){
      ss << "Bayesian_Unfoldediter" << i; 
      TH2D* unfolded2D  = (TH2D*)_file0->Get(ss.str().c_str());
      ss.str("");
      ss << "projHist_" << i;
      TH1D* unfolded = (TH1D*) unfolded2D->ProjectionX(ss.str().c_str(), ptBin, ptBin);
      ss.str("");
      unfolded->Scale(1./unfolded->Integral(), "width");
      unfolded->SetMarkerStyle(20);
      unfolded->SetMarkerColor(colors[i]);
      unfolded->SetLineColor(colors[i]);
      unfolded->SetLineWidth(2);
      pad1->cd();
      unfolded->Draw("same");
      pad2->cd();
      TH1D *h3 = (TH1D*)unfolded->Clone("h3");
      ss << "numerator" << i;
      h3->SetName(ss.str().c_str());
      ss.str("");
      h3->Divide(denomHist);
      h3->GetYaxis()->SetRangeUser(0.8, 1.1999999);
      h3->GetYaxis()->SetLabelSize(0.09);
      h3->GetYaxis()->SetTitleSize(0.09);
      h3->GetYaxis()->SetTitle("Ratio to True  ");
      h3->GetYaxis()->SetTitleOffset(0.5);
      h3->GetXaxis()->SetLabelSize(0.09);
      h3->GetXaxis()->SetTitleOffset(1.1);
      h3->GetXaxis()->SetTitleSize(0.12);
      h3->GetXaxis()->SetTitle("#it{R}_{g, corr}");
      h3->Draw("same");
      ss << "Iteration " << i;
      leg2->AddEntry(unfolded, ss.str().c_str());
      ss.str("");
    }
    pad1->cd();
    leg2->AddEntry(denomHist, "Truth");
    leg2->Draw("same");
    
    // Save with pT bin range in filename
    stringstream ss2; 
    if(isTrivial)ss2 << "./Trivial2DPlots/Trivial_Stability_pT" << (int)pTLow << "_" << (int)pTHigh << "_042326.pdf";
    else ss2 << "./SplitMC2DPlots/SplitMC_Stability_pT" << (int)pTLow << "_" << (int)pTHigh << "_042326.pdf";
    c->SaveAs(ss2.str().c_str());
    ss2.str("");
    
    // Clean up for next iteration
    delete c;
    delete ptLabel;
    delete leg2;
    delete denomHist;
  }
  //====================================================================
}