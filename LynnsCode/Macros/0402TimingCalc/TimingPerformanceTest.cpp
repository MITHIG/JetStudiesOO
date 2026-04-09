#include <vector>
#include <iostream>
#include <chrono>
#include <fstream>
#include <TCanvas.h>
#include <TGraph.h>
#include <string>
int TimingPerformanceTest(){
    TCanvas *c = new TCanvas("c", "Time Elapsed", 800, 600);

    std::ifstream in("/home/xirong/JetStudiesOO/022426ScanForest/Macros/0323QAPlotting/1PDMCTry2.log");
    std::vector<int> timeElapsed;
    std::string line;
    while (std::getline(in, line)){
        size_t pos = line.find("Time elapsed: ");
        if (pos != std::string::npos) {
            std::string timeStr = line.substr(pos + 13);
            try {
                int time = std::stoi(timeStr);
                std::cout << "Time: " << time << " seconds" << std::endl;
                timeElapsed.push_back(time);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid time value: " << timeStr << std::endl;
            }
        }
    }
    std::vector<int> diffs;
    diffs.reserve(timeElapsed.empty() ? 0 : timeElapsed.size() - 1);
    for (size_t i = 1; i < timeElapsed.size(); ++i) {
        diffs.push_back(timeElapsed[i] - timeElapsed[i - 1]);
        std::cout << "Difference between event " << i << " and " << (i - 1) << ": " << diffs.back() << " seconds" << std::endl;
    }

    TGraph *g = new TGraph(diffs.size());

    for (int i = 0; i < (int)diffs.size(); i++) {
        g->SetPoint(i, i, diffs[i]);
    }

    g->SetTitle("Time it takes to process 1M events;Number of 1M events processed;Time (s)");
    g->SetMarkerStyle(20);
    g->SetLineWidth(2);

    g->Draw("ALP");

    c->SaveAs("time_elapsed.png");
    return 0;
}