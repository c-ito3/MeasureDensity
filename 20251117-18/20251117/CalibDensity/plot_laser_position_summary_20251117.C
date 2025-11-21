void plot_laser_position_summary_20251117() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    
    // CSVファイル読み込み
    std::ifstream infile("laser_analysis_summary.csv");
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open laser_analysis_summary.csv" << std::endl;
        return;
    }
    
    std::vector<int> fileNum;
    std::vector<double> centerX, fwhmX, centerY, fwhmY;
    std::string line;
    std::getline(infile, line); // ヘッダーをスキップ
    
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<double> vals;
        
        while (std::getline(iss, token, ',')) {
            vals.push_back(std::stod(token));
        }
        
        if (vals.size() >= 6) {
            fileNum.push_back((int)vals[0]);
            centerX.push_back(vals[2]);
            fwhmX.push_back(vals[3]);
            centerY.push_back(vals[4]);
            fwhmY.push_back(vals[5]);
        }
    }
    infile.close();
    
    const int nPoints = fileNum.size();
    double *x_idx = new double[nPoints]; 
    double *cx = new double[nPoints];
    double *cy = new double[nPoints];
    
    for (int i = 0; i < nPoints; i++) {
        x_idx[i] = fileNum[i];
        cx[i] = centerX[i];
        cy[i] = centerY[i];
    }
    
    TGraph *grX = new TGraph(nPoints, x_idx, cx);
    TGraph *grY = new TGraph(nPoints, x_idx, cy);
    
    TCanvas *c1 = new TCanvas("c1", "Laser Center Position (20251117)", 1200, 800);
    gPad->SetGrid();
    gPad->SetRightMargin(0.15);
    
    grX->SetTitle("Laser Center Position (20251117);File Number;Center Position [mm]");
    grX->SetMarkerStyle(20);
    grX->SetMarkerSize(1.5);
    grX->SetMarkerColor(kRed);
    grX->SetLineColor(kRed);
    grX->SetLineWidth(2);
    
    // Y軸範囲設定
    double min_val = 2.0; // 20251117のデータ範囲 (Y~4mm, X~5-7mm) に合わせる
    double max_val = 8.0;
    grX->GetYaxis()->SetRangeUser(min_val, max_val);
    grX->Draw("APL");
    
    grY->SetMarkerStyle(21);
    grY->SetMarkerSize(1.5);
    grY->SetMarkerColor(kBlue);
    grY->SetLineColor(kBlue);
    grY->SetLineWidth(2);
    grY->Draw("PL SAME");
    
    TLatex *tex = new TLatex();
    tex->SetTextSize(0.025);
    tex->SetTextAlign(12);
    
    for (int i = 0; i < nPoints; i++) {
        // Xデータ用ラベル
        tex->SetTextColor(kRed);
        tex->DrawLatex(fileNum[i] + 0.2, cx[i], Form("X: %.2f, W: %.2f", cx[i], fwhmX[i]));
        
        // Yデータ用ラベル (Wのみ表示、下に配置)
        tex->SetTextColor(kBlue);
        double y_offset = -0.25; 
        tex->DrawLatex(fileNum[i] + 0.2, cy[i] + y_offset, Form("W: %.2f", fwhmY[i]));
    }
    
    // 凡例を左下に
    TLegend *leg = new TLegend(0.15, 0.15, 0.35, 0.28);
    leg->AddEntry(grX, "Center X", "lp");
    leg->AddEntry(grY, "Center Y", "lp");
    leg->Draw();
    
    c1->SaveAs("laser_position_summary_plot.png");
}

