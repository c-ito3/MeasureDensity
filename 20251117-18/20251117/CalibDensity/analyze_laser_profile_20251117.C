#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "TString.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TLine.h"
#include "TEllipse.h"
#include "TMarker.h"

void analyze_laser_profile_20251117() {
    // gSystem->mkdir("laser_analysis", kTRUE); // 出力フォルダ廃止
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kRainBow); // 256色 Rainbow
    
    const int width = 4096;
    const int height = 2992;
    const double pixel_size = 0.00345; // mm
    const double x_max = width * pixel_size;
    const double y_max = height * pixel_size;
    
    std::vector<int> file_nums = {2, 4, 6, 8, 10, 12, 14, 16};
    
    double global_max_val = 0;
    int brightest_file_num = -1;
    int brightest_x_bin = 0;
    int brightest_y_bin = 0;
    TH2F* h2_brightest = nullptr; // 最も明るいヒストグラムを保持
    
    // 結果格納用
    struct ProfileResult {
        int fileNum;
        double maxIntensity;
        double centerX;
        double fwhmX;
        double centerY;
        double fwhmY;
    };
    std::vector<ProfileResult> results;
    
    // 各ファイルを処理
    for (int fileNum : file_nums) {
        TString filename = TString::Format("../profile/20251117/DeviceData_#%03d.csv", fileNum);
        std::ifstream infile(filename.Data());
        
        if (!infile.is_open()) {
            std::cerr << "Error: Cannot open " << filename << std::endl;
            continue;
        }
        
        std::cout << "Processing " << filename << "..." << std::endl;
        
        TH2F *h2 = new TH2F(TString::Format("h2_%03d", fileNum), 
                            TString::Format("Laser Profile File #%03d;X [mm];Y [mm]", fileNum), 
                            width/4, 0, x_max, height/4, 0, y_max);
        
        std::string line;
        for (int i = 0; i < 18; ++i) std::getline(infile, line);
        
        int y_idx = 0;
        while (std::getline(infile, line) && y_idx < height) {
            std::istringstream iss(line);
            std::string token;
            int x_idx = 0;
            while (std::getline(iss, token, ';') && x_idx < width) {
                try {
                    double val = std::stod(token);
                    h2->Fill(x_idx * pixel_size, (height - 1 - y_idx) * pixel_size, val);
                } catch (...) {}
                x_idx++;
            }
            y_idx++;
        }
        infile.close();
        
        TCanvas *c1 = new TCanvas(TString::Format("c_%03d", fileNum), "Laser Profile", 1000, 800);
        c1->SetRightMargin(0.15);
        h2->Draw("COLZ");
        c1->SaveAs(TString::Format("laser_profile_%03d.png", fileNum));
        
        // 最大値と位置取得
        int binx, biny, binz;
        h2->GetMaximumBin(binx, biny, binz);
        double max_val = h2->GetBinContent(binx, biny);
        double center_x_est = h2->GetXaxis()->GetBinCenter(binx);
        double center_y_est = h2->GetYaxis()->GetBinCenter(biny);
        
        // X Profile Fit
        TH1D *projX = h2->ProjectionX(TString::Format("projX_%03d", fileNum), biny, biny);
        TF1 *gx = new TF1(TString::Format("gx_%03d", fileNum), "gaus + pol0(3)", center_x_est - 3.0, center_x_est + 3.0);
        gx->SetParameters(max_val, center_x_est, 0.5, 200);
        projX->Fit(gx, "Q0R");
        double fit_center_x = gx->GetParameter(1);
        double fit_fwhm_x = 2.355 * gx->GetParameter(2);
        delete gx;
        delete projX;

        // Y Profile Fit
        TH1D *projY = h2->ProjectionY(TString::Format("projY_%03d", fileNum), binx, binx);
        TF1 *gy = new TF1(TString::Format("gy_%03d", fileNum), "gaus + pol0(3)", center_y_est - 3.0, center_y_est + 3.0);
        gy->SetParameters(max_val, center_y_est, 0.5, 200);
        projY->Fit(gy, "Q0R");
        double fit_center_y = gy->GetParameter(1);
        double fit_fwhm_y = 2.355 * gy->GetParameter(2);
        delete gy;
        delete projY;
        
        results.push_back({fileNum, max_val, fit_center_x, fit_fwhm_x, fit_center_y, fit_fwhm_y});
        
        if (max_val > global_max_val) {
            global_max_val = max_val;
            brightest_file_num = fileNum;
            brightest_x_bin = binx;
            brightest_y_bin = biny;
            if (h2_brightest) delete h2_brightest;
            h2_brightest = (TH2F*)h2->Clone("h2_brightest");
        }
        
        if (h2 != h2_brightest) delete h2;
        delete c1;
    }
    
    // 結果の出力
    std::ofstream outfile("laser_analysis_summary.csv");
    outfile << "File,Max_Intensity,Center_X[mm],FWHM_X[mm],Center_Y[mm],FWHM_Y[mm]" << std::endl;
    
    std::cout << "\nAnalysis Summary (20251117):" << std::endl;
    std::cout << "File | Max Int | Center X | FWHM X | Center Y | FWHM Y" << std::endl;
    std::cout << "-----|---------|----------|--------|----------|-------" << std::endl;
    
    for (const auto& r : results) {
        outfile << r.fileNum << "," << r.maxIntensity << "," 
                << r.centerX << "," << r.fwhmX << "," 
                << r.centerY << "," << r.fwhmY << std::endl;
                
        printf(" %03d | %7.0f | %8.3f | %6.3f | %8.3f | %6.3f\n",
               r.fileNum, r.maxIntensity, r.centerX, r.fwhmX, r.centerY, r.fwhmY);
    }
    outfile.close();
    std::cout << "Results saved to laser_analysis_summary.csv" << std::endl;
    
    if (brightest_file_num == -1 || !h2_brightest) {
        std::cerr << "No valid data found." << std::endl;
        return;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Brightest Profile: File #" <<  TString::Format("%03d", brightest_file_num) << std::endl;
    std::cout << "Max Intensity: " << global_max_val << std::endl;
    
    // 中心位置（最大強度のビン中心）
    double center_x = h2_brightest->GetXaxis()->GetBinCenter(brightest_x_bin);
    double center_y = h2_brightest->GetYaxis()->GetBinCenter(brightest_y_bin);
    
    // プロファイル取得とガウスフィット
    TCanvas *c_fit = new TCanvas("c_fit", "Gaussian Fit", 1200, 600);
    c_fit->Divide(2, 1);
    
    // X Profile
    c_fit->cd(1);
    TH1D *projX = h2_brightest->ProjectionX("projX", brightest_y_bin, brightest_y_bin);
    projX->SetTitle("X Profile (at Peak Y);X [mm];Intensity");
    projX->Draw();
    
    TF1 *gx = new TF1("gx", "gaus + pol0(3)", center_x - 3.0, center_x + 3.0);
    gx->SetLineColor(kRed);
    gx->SetParameters(global_max_val, center_x, 0.5, 200);
    projX->Fit(gx, "R");
    
    double mean_x = gx->GetParameter(1);
    double sigma_x = gx->GetParameter(2);
    double fwhm_x = 2.355 * sigma_x;
    
    TLegend *legX = new TLegend(0.55, 0.7, 0.9, 0.9);
    legX->AddEntry(gx, Form("Center: %.3f mm", mean_x), "l");
    legX->AddEntry((TObject*)0, Form("FWHM: %.3f mm", fwhm_x), "");
    legX->Draw();
    
    // Y Profile
    c_fit->cd(2);
    TH1D *projY = h2_brightest->ProjectionY("projY", brightest_x_bin, brightest_x_bin);
    projY->SetTitle("Y Profile (at Peak X);Y [mm];Intensity");
    projY->Draw();
    
    TF1 *gy = new TF1("gy", "gaus + pol0(3)", center_y - 3.0, center_y + 3.0);
    gy->SetLineColor(kRed);
    gy->SetParameters(global_max_val, center_y, 0.5, 200);
    projY->Fit(gy, "R");
    
    double mean_y = gy->GetParameter(1);
    double sigma_y = gy->GetParameter(2);
    double fwhm_y = 2.355 * sigma_y;
    
    TLegend *legY = new TLegend(0.55, 0.7, 0.9, 0.9);
    legY->AddEntry(gy, Form("Center: %.3f mm", mean_y), "l");
    legY->AddEntry((TObject*)0, Form("FWHM: %.3f mm", fwhm_y), "");
    legY->Draw();
    
    c_fit->SaveAs("laser_brightest_profile_fit.png");
    
    std::cout << "Gaussian Fit Results:" << std::endl;
    std::cout << "X: Center=" << mean_x << " mm, FWHM=" << fwhm_x << " mm" << std::endl;
    std::cout << "Y: Center=" << mean_y << " mm, FWHM=" << fwhm_y << " mm" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // ヒートマップ上にフィット中心とビーム径（FWHM円）を描画
    TCanvas *c_final = new TCanvas("c_final", "Brightest Profile with Fit", 1000, 800);
    c_final->SetRightMargin(0.15);
    h2_brightest->Draw("COLZ");
    
    // 中心点
    TMarker *m = new TMarker(mean_x, mean_y, 3); // + shape
    m->SetMarkerSize(2);
    m->SetMarkerColor(kWhite);
    m->Draw();
    
    // FWHM円（楕円）
    TEllipse *el = new TEllipse(mean_x, mean_y, fwhm_x/2.0, fwhm_y/2.0);
    el->SetFillStyle(0);
    el->SetLineColor(kWhite);
    el->SetLineStyle(2);
    el->SetLineWidth(2);
    el->Draw();
    
    c_final->SaveAs("laser_brightest_profile_map.png");
}

