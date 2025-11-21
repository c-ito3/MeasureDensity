#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include "TString.h"
#include "TDatime.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TLegend.h"
#include "TMath.h"

struct Measurement {
    int fileNum;
    const char* startTimeStr;
    int durationMin;
    bool isPinholeOn;
};

void analyze_laser_power_20251118() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    
    // 測定スケジュール (2025/11/18)
    std::vector<Measurement> schedule = {
        {1, "17:26:00", 3, false},
        {2, "17:34:00", 9, true},
        {3, "17:46:00", 3, false},
        {4, "17:51:00", 9, true},
        {5, "18:02:00", 3, false},
        {6, "18:08:00", 9, true},
        {7, "18:22:00", 3, false},
        {8, "18:26:00", 9, true},
        {9, "18:39:00", 3, false},
        {10, "18:43:00", 9, true},
        {11, "18:55:00", 3, false},
        {12, "19:01:00", 9, true},
        {13, "19:12:00", 3, false},
        {14, "19:18:00", 9, true},
        {15, "19:30:00", 3, false},
        {16, "19:37:00", 9, true},
        {17, "19:54:00", 3, false}
    };
    
    // CSVファイル読み込み
    std::ifstream infile("01.csv");
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open 01.csv" << std::endl;
        return;
    }
    
    std::vector<TDatime> timeVec;
    std::vector<double> powerVec;
    
    std::string line;
    // ヘッダー15行スキップ
    for(int i=0; i<15; ++i) std::getline(infile, line);
    
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 4) {
            std::string dateStr = tokens[1];
            std::string timeStr = tokens[2];
            std::string powerStr = tokens[3];
            
            dateStr.erase(0, dateStr.find_first_not_of(" "));
            timeStr.erase(0, timeStr.find_first_not_of(" "));
            
            int mon, day, year, hh, mm, ss_int;
            sscanf(dateStr.c_str(), "%d/%d/%d", &mon, &day, &year);
            sscanf(timeStr.c_str(), "%d:%d:%d", &hh, &mm, &ss_int);
            
            TDatime dt(year, mon, day, hh, mm, ss_int);
            timeVec.push_back(dt);
            powerVec.push_back(std::stod(powerStr));
        }
    }
    infile.close();
    
    std::cout << "Loaded " << timeVec.size() << " data points." << std::endl;
    
    // 結果格納用
    std::vector<double> on_file, on_power, on_err;
    std::vector<double> off_file, off_power, off_err;
    
    std::ofstream outfile("laser_power_summary.csv");
    outfile << "File,Type,Start,Duration[min],AvgPower[W],StdDev[W]" << std::endl;
    
    for (const auto& m : schedule) {
        int year = 2025;
        int mon = 11;
        int day = 18; // Date is fixed to 18th
        int hh, mm, ss_int;
        sscanf(m.startTimeStr, "%d:%d:%d", &hh, &mm, &ss_int);
        
        TDatime startDt(year, mon, day, hh, mm, ss_int);
        TDatime endDt(startDt.Convert() + m.durationMin * 60);
        
        double sum = 0;
        double sum2 = 0;
        int count = 0;
        
        for (size_t i = 0; i < timeVec.size(); ++i) {
            if (timeVec[i].Convert() >= startDt.Convert() && timeVec[i].Convert() < endDt.Convert()) {
                sum += powerVec[i];
                sum2 += powerVec[i] * powerVec[i];
                count++;
            }
        }
        
        double avg = 0;
        double stddev = 0;
        if (count > 0) {
            avg = sum / count;
            stddev = TMath::Sqrt(sum2 / count - avg * avg);
        }
        
        outfile << m.fileNum << "," << (m.isPinholeOn ? "ON" : "OFF") << "," 
                << m.startTimeStr << "," << m.durationMin << "," 
                << avg << "," << stddev << std::endl;
                
        printf("File %02d (%s): %.4f +/- %.4f W (N=%d)\n", m.fileNum, (m.isPinholeOn ? "ON " : "OFF"), avg, stddev, count);
        
        if (m.isPinholeOn) {
            on_file.push_back(m.fileNum);
            on_power.push_back(avg);
            on_err.push_back(stddev);
        } else {
            off_file.push_back(m.fileNum);
            off_power.push_back(avg);
            off_err.push_back(stddev);
        }
    }
    outfile.close();
    
    // グラフ作成
    TGraphErrors *grON = new TGraphErrors(on_file.size(), &on_file[0], &on_power[0], 0, &on_err[0]);
    TGraphErrors *grOFF = new TGraphErrors(off_file.size(), &off_file[0], &off_power[0], 0, &off_err[0]);
    
    TCanvas *c1 = new TCanvas("c1", "Laser Power Summary", 1200, 600);
    gPad->SetGrid();
    
    TH1F *frame = c1->DrawFrame(0, 0, 18, 2.5);
    frame->SetTitle("Laser Power Stability (2025/11/18);File Number;Average Power [W]");
    
    grON->SetMarkerStyle(20);
    grON->SetMarkerSize(1.5);
    grON->SetMarkerColor(kRed);
    grON->SetLineColor(kRed);
    
    grOFF->SetMarkerStyle(21);
    grOFF->SetMarkerSize(1.5);
    grOFF->SetMarkerColor(kBlue);
    grOFF->SetLineColor(kBlue);
    
    grON->Draw("P SAME");
    grOFF->Draw("P SAME");
    
    TLegend *leg = new TLegend(0.7, 0.15, 0.88, 0.3);
    leg->AddEntry(grON, "Pinhole ON (9min)", "lp");
    leg->AddEntry(grOFF, "Pinhole OFF (3min)", "lp");
    leg->Draw();
    
    c1->SaveAs("laser_power_summary.png");
}

