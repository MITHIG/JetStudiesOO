// draw2025jets.c
// Hannah Bossi, <hannah.bossi@cern.ch>


void drawJetPerf(){

    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    string tag = "OO_March18th_NoEmbedded"; 

    TLatex* cms = new TLatex(0.10,0.92, "#bf{PYTHIA8 OO} (5.36 TeV) ");
    cms->SetNDC();
    cms->SetTextSize(0.05);
    cms->SetTextFont(42);


    TLatex* zcut = new TLatex(0.3,0.815, "#it{z}_{cut} = 0.2");
    zcut->SetNDC();
    zcut->SetTextSize(0.04);
    zcut->SetTextFont(42);

   // List your ROOT files here
    std::vector<std::string> files = { "JetPerf_OOPYTHIA_NoEmbedded_March18th.root"};
    std::vector<std::string> Labels = { "#it{p}^{gen}_{T, jet} > 50 GeV"};

    std::vector<int> colors = {kViolet-2, kAzure-2, kSpring+2 };

    TCanvas* c = new TCanvas("c", "", 600, 600);
    c->SetTickx(); 
    c->SetTicky(); 
    c->SetLogz();
    c->SetRightMargin(0.15); 
    c->SetLeftMargin(0.12); 

    TLegend* leg = new TLegend(0.15, 0.15, 0.5, 0.17);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);

    double nEvents = 725596.0; 

    bool first = true;
    c->cd(); 
   

    TFile* f = TFile::Open(files[0].c_str());
    TH2* h = (TH2*)f->Get("hJetPerf");
    TH2* hRaw = (TH2*)f->Get("hJetPerfRaw");
    TH2* h0_10 = (TH2*)f->Get("hJetPerf_cent_0_10"); 
    TH2* h10_30 = (TH2*)f->Get("hJetPerf_cent_10_30");  
    TH2* h30_50 = (TH2*)f->Get("hJetPerf_cent_30_50");
    TH2* h50_100 = (TH2*)f->Get("hJetPerf_cent_50_100");  
    TH1* hCent = (TH1*)f->Get("hCent"); 
    TH1* hHF1D = (TH1*)f->Get("hHiHF_1D");
    // -----------------------------------------------------------
    // Plot #1: 2D plot of the pTjet/pTRef vs. pT ref
    TH2* hClone = (TH2*)h->Clone(Form("hClone_%d", 0));
    hClone->SetDirectory(0);
    //hClone->GetYaxis()->SetRangeUser(0.5, 1.5); 
    hClone->GetXaxis()->SetTitle("#it{p}^{ref}_{T} (GeV/#it{c})");
    hClone->GetXaxis()->SetTitleOffset(1.2);
    hClone->GetYaxis()->SetTitle("#it{p}^{jet}_{T}/#it{p}^{ref}_{T}");

    if (first) {
        hClone->Draw("colz");
        first = false;
        
    }

    TProfile *pfx = hClone->ProfileX(Form("pfx_%d", 0));
    pfx->SetDirectory(0);
    pfx->SetMarkerColor(kBlack);
    pfx->SetMarkerStyle(20);
    pfx->SetLineColor(kBlack);
    pfx->GetXaxis()->SetRangeUser(30.0, 500.0);

    pfx->Draw("E1 same");

    TLine *line = new TLine(0, 1, 500, 1);

    line->SetLineStyle(2);   // dotted
    line->SetLineWidth(2);
    line->SetLineColor(kBlack);

    line->Draw("same");
    std::cout << "here" << std::endl;
    leg->Draw();
    cms->Draw(); 
    // zcut->Draw(); 
    // c->Update();
    c->SaveAs(Form("JetPerf_%s.pdf", tag.c_str()));

     // -----------------------------------------------------------
    // Now loop over the bins and plot the JES and the JER

    // first create the empty histograms for the JES and JER
    // inclusive case 
    TH1D* hJES = (TH1D*)hClone->ProjectionX("JES"); 
    TH1D* hJER = (TH1D*)hClone->ProjectionX("JER");
    hJES->Reset();
    hJER->Reset(); 


    Int_t nBins = hClone->GetXaxis()->GetNbins();
    for(Int_t i=1; i <= nBins ; i++){
      TString proj = Form("px_%d", i);
      TH1D* px = hClone->ProjectionY(proj, i, i); 
      TF1 *f1 = new TF1("f1", "gaus", 0.5, 1.2);
      px->Fit(f1,"","",0.5, 1.5);
      TCanvas* cTemp = new TCanvas( Form("canvas_%d", i), "", 600, 600);
      cTemp->SetTickx(); 
      cTemp->SetTicky(); 
      cTemp->SetRightMargin(0.15); 
      cTemp->SetLeftMargin(0.12);
      TLatex* pt = new TLatex(0.3,0.815, Form("%0.2f < pT < %0.2f GeV",hClone->GetXaxis()->GetBinLowEdge(i), hClone->GetXaxis()->GetBinLowEdge(i) + hClone->GetXaxis()->GetBinWidth(i)));
      pt->SetNDC();
      pt->SetTextSize(0.02);
      pt->SetTextFont(42);
      px->Draw(); 
      pt->Draw(); 
      //cTemp->SaveAs(Form("projBinNumber_%d.pdf", i));  
      if(hClone->GetXaxis()->GetBinLowEdge(i) > 30){
        hJER->SetBinContent(i,  f1->GetParameter(2)/f1->GetParameter(1));
        hJER->SetBinError(i,  f1->GetParError(2));
        hJES->SetBinContent(i, f1->GetParameter(1)); 
        hJES->SetBinError(i, f1->GetParError(1)); 
      }                  
    }

    TH1D* hJES_0_10 = (TH1D*)h0_10->ProjectionX("JES_0_10"); 
    TH1D* hJER_0_10 = (TH1D*)h0_10->ProjectionX("JER_0_10");
    hJES_0_10->Reset();
    hJER_0_10->Reset(); 

    // central 
    for(Int_t i=1; i <= nBins ; i++){
      TString proj = Form("px_%d", i);
      TH1D* px = h0_10->ProjectionY(proj, i, i); 
      TF1 *f1 = new TF1("f1", "gaus", 0.5, 1.2);
      px->Fit(f1,"","",0.5, 1.5);
      if(h0_10->GetXaxis()->GetBinLowEdge(i) > 30){
        hJER_0_10->SetBinContent(i,  f1->GetParameter(2)/f1->GetParameter(1));
        hJER_0_10->SetBinError(i,  f1->GetParError(2));
        hJES_0_10->SetBinContent(i, f1->GetParameter(1)); 
        hJES_0_10->SetBinError(i, f1->GetParError(1)); 
      }                  
    }


    TH1D* hJES_10_30 = (TH1D*)h10_30->ProjectionX("JES_10_30"); 
    TH1D* hJER_10_30 = (TH1D*)h10_30->ProjectionX("JER_10_30");
    hJES_10_30->Reset();
    hJER_10_30->Reset(); 

    // 10-30 
    for(Int_t i=1; i <= nBins ; i++){
      TString proj = Form("px_%d", i);
      TH1D* px = h10_30->ProjectionY(proj, i, i); 
      TF1 *f1 = new TF1("f1", "gaus", 0.5, 1.2);
      px->Fit(f1,"","",0.5, 1.5);
      if(h0_10->GetXaxis()->GetBinLowEdge(i) > 30){
        hJER_10_30->SetBinContent(i,  f1->GetParameter(2)/f1->GetParameter(1));
        hJER_10_30->SetBinError(i,  f1->GetParError(2));
        hJES_10_30->SetBinContent(i, f1->GetParameter(1)); 
        hJES_10_30->SetBinError(i, f1->GetParError(1)); 
      }                  
    }


    TH1D* hJES_30_50 = (TH1D*)h10_30->ProjectionX("JES_30_50"); 
    TH1D* hJER_30_50 = (TH1D*)h10_30->ProjectionX("JER_30_50");
    hJES_30_50->Reset();
    hJER_30_50->Reset(); 

    // 30-50 
    for(Int_t i=1; i <= nBins ; i++){
      TString proj = Form("px_%d", i);
      TH1D* px = h30_50->ProjectionY(proj, i, i); 
      TF1 *f1 = new TF1("f1", "gaus", 0.5, 1.2);
      px->Fit(f1,"","",0.5, 1.5);
      if(h0_10->GetXaxis()->GetBinLowEdge(i) > 30){
        hJER_30_50->SetBinContent(i,  f1->GetParameter(2)/f1->GetParameter(1));
        hJER_30_50->SetBinError(i,  f1->GetParError(2));
        hJES_30_50->SetBinContent(i, f1->GetParameter(1)); 
        hJES_30_50->SetBinError(i, f1->GetParError(1)); 
      }                  
    }

    TH1D* hJES_Raw = (TH1D*)hRaw->ProjectionX("JES_Raw"); 
    TH1D* hJER_Raw = (TH1D*)hRaw->ProjectionX("JER_Raw");
    hJER_Raw->Reset();
    hJER_Raw->Reset(); 

    // 30-50 
    for(Int_t i=1; i <= nBins ; i++){
      TString proj = Form("px_%d", i);
      TH1D* px = hRaw->ProjectionY(proj, i, i); 
      TF1 *f1 = new TF1("f1", "gaus", 0.5, 1.2);
      px->Fit(f1,"","",0.5, 1.5);
      if(h0_10->GetXaxis()->GetBinLowEdge(i) > 30){
        hJER_Raw->SetBinContent(i,  f1->GetParameter(2)/f1->GetParameter(1));
        hJER_Raw->SetBinError(i,  f1->GetParError(2));
        hJES_Raw->SetBinContent(i, f1->GetParameter(1)); 
        hJES_Raw->SetBinError(i, f1->GetParError(1)); 
      }                  
    }


    TCanvas* cJES = new TCanvas("cJES", "", 600, 600);
    cJES->SetTickx(); 
    cJES->SetTicky(); 
    cJES->SetRightMargin(0.15); 
    cJES->SetLeftMargin(0.12); 

    hJES->GetYaxis()->SetTitle("#mu");    
    hJES->GetYaxis()->SetRangeUser(0.8, 1.2);
    hJES->GetXaxis()->SetTitle("#it{p}_{T, ref} (GeV/#it{c})"); 
    hJES->SetMarkerColor(kBlack); 
    hJES->SetMarkerStyle(20); 
    hJES->SetLineColor(kBlack); 
    hJES_Raw->SetMarkerColor(kBlack); 
    hJES_Raw->SetMarkerStyle(24); 
    hJES_Raw->SetLineColor(kBlack); 
    hJES_0_10->SetMarkerColor(kRed); 
    hJES_0_10->SetMarkerStyle(20); 
    hJES_0_10->SetLineColor(kRed);
    hJES_10_30->SetMarkerColor(kAzure-2); 
    hJES_10_30->SetMarkerStyle(20); 
    hJES_10_30->SetLineColor(kAzure-2);
    hJES_30_50->SetMarkerColor(kGreen+3); 
    hJES_30_50->SetMarkerStyle(20); 
    hJES_30_50->SetLineColor(kGreen+3);
    hJES->Draw(); 
    hJES_Raw->Draw("same"); 
    // hJES_0_10->Draw("same"); 
    // hJES_10_30->Draw("same"); 
    // hJES_30_50->Draw("same"); 
    TLegend* leg2 = new TLegend( 0.4, 0.75, 0.8, 0.9); 
    leg2->SetBorderSize(0); 
    leg2->SetFillStyle(0);
    leg2->AddEntry(hJES, "Inclusive (0-100) - Corrected PT"); 
    leg2->AddEntry(hJES_Raw, "Inclusive (0-100) - Raw PT"); 

    // leg2->AddEntry(hJES_0_10, "(0-10)"); 
    // leg2->AddEntry(hJES_10_30, "(10-30)"); 
    // leg2->AddEntry(hJES_30_50, "(30-50)"); 
    leg2->Draw(); 
    line->Draw(); 
    cms->Draw(); 
    cJES->SaveAs(Form("JES_%s.pdf", tag.c_str())); 


    TCanvas* cJER = new TCanvas("cJER", "", 600, 600);
    cJER->SetTickx(); 
    cJER->SetTicky(); 
    cJER->SetRightMargin(0.15); 
    cJER->SetLeftMargin(0.12); 

    hJER->GetYaxis()->SetTitle("#sigma/#mu"); 
    hJER->GetXaxis()->SetTitle("#it{p}_{T, ref} (GeV/#it{c})"); 
    hJER->SetMarkerColor(kBlack); 
    hJER->SetMarkerStyle(20); 
    hJER->SetLineColor(kBlack); 
    hJER_Raw->SetMarkerColor(kBlack); 
    hJER_Raw->SetMarkerStyle(24); 
    hJER_Raw->SetLineColor(kBlack); 
    hJER_0_10->SetMarkerColor(kRed); 
    hJER_0_10->SetMarkerStyle(20); 
    hJER_0_10->SetLineColor(kRed);
    hJER_10_30->SetMarkerColor(kAzure-2); 
    hJER_10_30->SetMarkerStyle(20); 
    hJER_10_30->SetLineColor(kAzure-2);
    hJER_30_50->SetMarkerColor(kGreen+3); 
    hJER_30_50->SetMarkerStyle(20); 
    hJER_30_50->SetLineColor(kGreen+3);
    hJER->Draw();
    hJER_Raw->Draw("same"); 
    // hJER_0_10->Draw("same"); 
    // hJER_10_30->Draw("same"); 
    // hJER_30_50->Draw("same"); 
    leg2->Draw(); 
    cms->Draw(); 
    cJER->SaveAs(Form("JER_%s.pdf", tag.c_str())); 
    



    //-------------------------------------------------------------
    // now draw the centrality distribution
    TCanvas* cCENT = new TCanvas("cCENT", "", 600, 600);
    cCENT->SetTickx(); 
    cCENT->SetTicky(); 
    cCENT->SetLogy(); 
    cCENT->SetRightMargin(0.15); 
    cCENT->SetLeftMargin(0.12); 
    hCent->SetMarkerColor(kBlack); 
    hCent->SetMarkerStyle(20); 
    hCent->SetLineColor(kBlack);
    hCent->GetXaxis()->SetTitle("Centrality (%)"); 
    hCent->Draw("same");

    cms->Draw(); 
    cCENT->SaveAs(Form("Cent_%s.pdf", tag.c_str())); 


     // now draw the centrality distribution
    TCanvas* cHF = new TCanvas("cHF", "", 600, 600);
    cHF->SetTickx(); 
    cHF->SetTicky(); 
    cHF->SetLogy(); 
    cHF->SetRightMargin(0.15); 
    cHF->SetLeftMargin(0.12); 
    hHF1D->SetMarkerColor(kBlack); 
    hHF1D->SetMarkerStyle(20); 
    hHF1D->SetLineColor(kBlack);
    hHF1D->GetXaxis()->SetTitle("hiHF_pf (GeV)"); 
    hHF1D->Draw("same");

    cms->Draw(); 
    cHF->SaveAs(Form("HiHF_%s.pdf", tag.c_str())); 

    //-------------------------------------------------------------
    f->Close();

}