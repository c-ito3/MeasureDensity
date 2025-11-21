void normalize_pinholeon_average() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1111);
    
    // Pinhole ONのデータ（偶数番号）
    const int nPoints = 8;
    int data_nums[nPoints] = {2, 4, 6, 8, 10, 12, 14, 16};
    
    // レーザー変位 [mm]（19.7mmを基準=0として変位を計算）
    // File 6は19.45mm (+0.25mm)と仮定（以前の指示に基づく）
    double laser_position[nPoints] = {
        19.7 - 19.7,   // File 2: 19.7 → 0.0
        19.7 - 19.95,  // File 4: 19.95 → -0.25
        19.7 - 19.45,  // File 6: 19.45 → 0.25 (推定)
        19.7 - 20.2,   // File 8: 20.2 → -0.5
        19.7 - 19.2,   // File 10: 19.2 → 0.5
        19.7 - 18.95,  // File 12: 18.95 → 0.75
        19.7 - 18.7,   // File 14: 18.7 → 1.0
        19.7 - 18.2    // File 16: 18.2 → 1.5
    };
    
    // AreaCutのカウント数を読み込む
    std::ifstream countFile("areacut_counts.txt");
    if (!countFile.is_open()) {
        std::cerr << "Error: Cannot open areacut_counts.txt" << std::endl;
        return;
    }
    
    std::map<int, int> counts;
    std::string line;
    std::getline(countFile, line);  // ヘッダーをスキップ
    
    while (std::getline(countFile, line)) {
        std::istringstream iss(line);
        int fileNum, valid, tofCut, areaCut;
        if (iss >> fileNum >> valid >> tofCut >> areaCut) {
            counts[fileNum] = areaCut;
        }
    }
    countFile.close();
    
    // 基準となるカウント数（File 001）
    if (counts.find(1) == counts.end()) {
        std::cerr << "Error: File 001 count not found." << std::endl;
        return;
    }
    double ref_count = counts[1];
    
    // 配列の準備
    double x[nPoints], y_raw[nPoints], y_norm[nPoints];
    double y_raw_err[nPoints], y_norm_err[nPoints], x_err[nPoints];
    
    cout << "\nNormalization Results (Neighbor Average):" << endl;
    cout << "Ref Count (File 001): " << ref_count << endl;
    cout << "File | Pos[mm] | Prev(OFF) | Next(OFF) | Avg(OFF) | Factor | Raw | Norm" << endl;
    cout << "-----|---------|-----------|-----------|----------|--------|-----|-----" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        int fileNum = data_nums[i];
        int prevFile = fileNum - 1;
        int nextFile = fileNum + 1;
        
        if (counts.find(prevFile) == counts.end() || counts.find(nextFile) == counts.end()) {
            std::cerr << "Error: Missing neighbor files for File " << fileNum << std::endl;
            continue;
        }
        
        double prevCount = counts[prevFile];
        double nextCount = counts[nextFile];
        double avgCount = (prevCount + nextCount) / 2.0;
        double factor = ref_count / avgCount;
        
        x[i] = laser_position[i];
        x_err[i] = 0;
        
        y_raw[i] = counts[fileNum];
        y_raw_err[i] = TMath::Sqrt(y_raw[i]);
        
        y_norm[i] = y_raw[i] * factor;
        y_norm_err[i] = y_raw_err[i] * factor; // 簡易的なエラー伝播
        
        printf("%4d | %7.2f | %9.0f | %9.0f | %8.1f | %6.4f | %3.0f | %5.1f\n",
               fileNum, x[i], prevCount, nextCount, avgCount, factor, y_raw[i], y_norm[i]);
    }
    
    // グラフ作成
    TGraphErrors *gr_raw = new TGraphErrors(nPoints, x, y_raw, x_err, y_raw_err);
    TGraphErrors *gr_norm = new TGraphErrors(nPoints, x, y_norm, x_err, y_norm_err);
    
    TCanvas *c1 = new TCanvas("c1", "Normalized Counts", 1200, 800);
    gPad->SetGrid();
    
    gr_norm->SetTitle("Ps Counts vs Laser Displacement (Neighbor Average Norm);Laser Displacement [mm];Ps Counts");
    gr_norm->SetMarkerStyle(20);
    gr_norm->SetMarkerSize(1.5);
    gr_norm->SetMarkerColor(kRed);
    gr_norm->SetLineColor(kRed);
    gr_norm->GetYaxis()->SetRangeUser(0, 2500); // 範囲調整
    gr_norm->Draw("APE");
    
    gr_raw->SetMarkerStyle(24);
    gr_raw->SetMarkerColor(kBlue);
    gr_raw->SetLineColor(kBlue);
    gr_raw->Draw("P SAME");
    
    // 凡例
    TLegend *leg = new TLegend(0.15, 0.75, 0.45, 0.88);
    leg->AddEntry(gr_norm, "Normalized (Neighbor Avg)", "lp");
    leg->AddEntry(gr_raw, "Raw Counts", "lp");
    leg->Draw();
    
    // ガウシアンフィット
    TF1 *f1 = new TF1("f1", "gaus", -1.0, 2.0);
    f1->SetLineColor(kGreen+2);
    f1->SetLineWidth(3);
    // 初期値をデータに合わせて調整
    f1->SetParameters(1800, 0.5, 0.4);
    f1->SetParLimits(0, 1000, 3000);
    f1->SetParLimits(1, 0.0, 1.0);
    f1->SetParLimits(2, 0.1, 1.0);
    
    TFitResultPtr fitRes = gr_norm->Fit(f1, "RS");
    
    double mean = f1->GetParameter(1);
    double sigma = f1->GetParameter(2);
    double mean_err = f1->GetParError(1);
    double sigma_err = f1->GetParError(2);
    double fwhm = 2.355 * sigma;
    double fwhm_err = 2.355 * sigma_err;
    
    TLegend *leg2 = new TLegend(0.62, 0.45, 0.98, 0.65);
    leg2->SetHeader("Gaussian Fit", "C");
    leg2->AddEntry(f1, Form("Center: %.3f #pm %.3f mm", mean, mean_err), "l");
    leg2->AddEntry((TObject*)0, Form("FWHM: %.3f #pm %.3f mm", fwhm, fwhm_err), "");
    leg2->Draw();
    
    c1->SaveAs("normalized_laser_position.png");
    
    // 保存
    TFile *fout = new TFile("normalized_laser_position.root", "RECREATE");
    gr_norm->Write("gr_normalized");
    gr_raw->Write("gr_raw");
    f1->Write("fit_gauss");
    fout->Close();
}

