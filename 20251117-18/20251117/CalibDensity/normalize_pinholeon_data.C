void normalize_pinholeon_data() {
    gSystem->mkdir(".", kTRUE);
    
    // フィットパラメータ（plot_pinholeoff_expo_fit.Cの結果）
    // Exponential fit: y = A + B * exp(C * t) (FILE 17除外後、t→∞でy→-∞)
    double A = 1927.94;         // オフセット
    double B = -0.350853;       // 振幅（負の値）
    double C = 0.051418;        // 増大定数 [/min]（正の値）
    
    // time=0でのフィット値
    double fit_t0 = A + B * TMath::Exp(C * 0);  // = A + B
    
    cout << "Fit function: y = " << A << " + " << B << " * exp(" << C << " * t)" << endl;
    cout << "Fit value at t=0: " << fit_t0 << endl;
    cout << "========================================" << endl;
    
    // Pinhole ONのデータ（偶数番号: 2, 4, 8, 10, 12, 14, 16）
    // File 6は測定時間が短いため除外
    const int nPoints = 7;
    
    // 測定開始時刻（15:39 = File 1の開始時刻を基準 = 0分）[分]
    double start_time[nPoints] = {
        6,    // File 2: 0.65625 × 1440 = 945.0 → 945.0 - 939.0 = 6
        23,   // File 4: 0.6680555556 × 1440 = 962.0 → 962.0 - 939.0 = 23
        59,   // File 8: 0.6930555556 × 1440 = 998.0 → 998.0 - 939.0 = 59
        76,   // File 10: 0.7048611111 × 1440 = 1015.0 → 1015.0 - 939.0 = 76
        121,  // File 12: 0.7361111111 × 1440 = 1060.0 → 1060.0 - 939.0 = 121
        137,  // File 14: 0.7472222222 × 1440 = 1076.0 → 1076.0 - 939.0 = 137
        155   // File 16: 0.7597222222 × 1440 = 1094.0 → 1094.0 - 939.0 = 155
    };
    
    // レーザー変位 [mm]（19.7mmを基準=0として変位を計算）
    double laser_position[nPoints] = {
        19.7 - 19.7,   // File 2: 19.7 → 0.0
        19.7 - 19.95,  // File 4: 19.95 → -0.25
        19.7 - 20.2,   // File 8: 20.2 → -0.5
        19.7 - 19.2,   // File 10: 19.2 → 0.5
        19.7 - 18.95,  // File 12: 18.95 → 0.75
        19.7 - 18.7,   // File 14: 18.7 → 1.0
        19.7 - 18.2    // File 16: 18.2 → 1.5
    };
    
    int data_nums[nPoints] = {2, 4, 8, 10, 12, 14, 16};
    
    // AreaCutのカウント数を読み込む
    std::ifstream countFile("areacut_counts.txt");
    if (!countFile.is_open()) {
        std::cerr << "Error: Cannot open areacut_counts.txt" << std::endl;
        return;
    }
    
    std::map<int, int> areaCutCounts;
    std::string line;
    std::getline(countFile, line);  // ヘッダーをスキップ
    
    while (std::getline(countFile, line)) {
        std::istringstream iss(line);
        int fileNum, valid, tofCut, areaCut;
        if (iss >> fileNum >> valid >> tofCut >> areaCut) {
            areaCutCounts[fileNum] = areaCut;
        }
    }
    countFile.close();
    
    // Pinhole ONのカウント数を取得
    double measured_counts[nPoints];
    for (int i = 0; i < nPoints; i++) {
        measured_counts[i] = areaCutCounts[data_nums[i]];
    }
    
    // 校正係数と校正後カウント数を計算
    double calibration_factor[nPoints];
    double normalized_counts[nPoints];
    double measured_errors[nPoints];
    double normalized_errors[nPoints];
    double zero_errors[nPoints];
    
    cout << "\nCalibration calculation:" << endl;
    cout << "Data | Start[min] | Avg Fit | Calib Factor | Measured | Normalized" << endl;
    cout << "-----|------------|---------|--------------|----------|------------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        // 15分間のフィット値の平均を計算（1分刻み）
        double sum_fit = 0;
        int n_steps = 15;
        for (int j = 0; j < n_steps; j++) {
            double t = start_time[i] + j;
            sum_fit += A + B * TMath::Exp(C * t);
        }
        double avg_fit = sum_fit / n_steps;
        
        // 校正係数
        calibration_factor[i] = fit_t0 / avg_fit;
        
        // 校正後カウント数
        normalized_counts[i] = measured_counts[i] * calibration_factor[i];
        
        // エラーバー（ポアソン統計: √N）
        measured_errors[i] = TMath::Sqrt(measured_counts[i]);
        normalized_errors[i] = measured_errors[i] * calibration_factor[i];
        zero_errors[i] = 0;
        
        printf("%4d | %10.0f | %7.1f | %12.4f | %8.0f | %10.1f\n",
               data_nums[i], start_time[i], avg_fit, calibration_factor[i],
               measured_counts[i], normalized_counts[i]);
    }
    
    // グラフを作成（エラーバー付き）
    TGraphErrors *gr_measured = new TGraphErrors(nPoints, laser_position, measured_counts, zero_errors, measured_errors);
    TGraphErrors *gr_normalized = new TGraphErrors(nPoints, laser_position, normalized_counts, zero_errors, normalized_errors);
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Ps Counts vs Laser Displacement", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    // 校正後のグラフ
    gr_normalized->SetMarkerStyle(20);
    gr_normalized->SetMarkerSize(2.0);
    gr_normalized->SetMarkerColor(kRed);
    gr_normalized->SetLineColor(kRed);
    gr_normalized->SetLineWidth(2);
    gr_normalized->SetTitle("Ps Counts vs Laser Displacement (Pinhole ON);Laser Displacement [mm];Ps Counts");
    gr_normalized->GetXaxis()->SetRangeUser(-1.0, 2.0);
    gr_normalized->GetYaxis()->SetRangeUser(0, 2500);
    gr_normalized->GetXaxis()->SetTitleSize(0.045);
    gr_normalized->GetYaxis()->SetTitleSize(0.045);
    gr_normalized->GetXaxis()->SetLabelSize(0.04);
    gr_normalized->GetYaxis()->SetLabelSize(0.04);
    gr_normalized->Draw("APE");
    
    // 元のデータも重ねて表示（比較用）
    gr_measured->SetMarkerStyle(24);
    gr_measured->SetMarkerSize(1.5);
    gr_measured->SetMarkerColor(kBlue);
    gr_measured->SetLineColor(kBlue);
    gr_measured->SetLineWidth(2);
    gr_measured->Draw("PE SAME");
    
    // 凡例を追加（左上に配置してデータと被らないように）
    TLegend *leg = new TLegend(0.15, 0.60, 0.42, 0.88);
    leg->SetTextSize(0.025);
    leg->SetHeader("Data Selection:", "C");
    leg->AddEntry((TObject*)0, "196 < TOF < 210", "");
    leg->AddEntry((TObject*)0, "-10 #leq X,Y #leq 10 mm", "");
    leg->AddEntry(gr_normalized, "Time-corrected", "p");
    leg->AddEntry(gr_measured, "Measured", "p");
    leg->Draw();
    
    // ガウシアンフィット（全範囲で実施）
    TF1 *fit_gauss = new TF1("fit_gauss", "gaus", -1.0, 2.0);
    fit_gauss->SetParameter(0, 1700);  // 振幅の初期値（ピークは約1676）
    fit_gauss->SetParameter(1, 0.4);   // 中心位置の初期値（0.25-0.5mmの間）
    fit_gauss->SetParameter(2, 0.5);   // σの初期値（FWHM~1.2mm相当）
    fit_gauss->SetParLimits(0, 1000, 3000);  // 振幅の範囲を制限
    fit_gauss->SetParLimits(1, -0.5, 1.5);   // 中心位置の範囲を制限
    fit_gauss->SetParLimits(2, 0.3, 1.0);    // σの範囲を制限（より狭く）
    fit_gauss->SetLineColor(kGreen+2);
    fit_gauss->SetLineWidth(3);
    
    TFitResultPtr fitResult = gr_normalized->Fit(fit_gauss, "RSQ");
    
    // フィットパラメータとエラーを取得
    double amplitude = fit_gauss->GetParameter(0);
    double mean = fit_gauss->GetParameter(1);
    double sigma = fit_gauss->GetParameter(2);
    double mean_error = fit_gauss->GetParError(1);
    double sigma_error = fit_gauss->GetParError(2);
    double fwhm = 2.355 * sigma;  // FWHM = 2.355 × σ
    double fwhm_error = 2.355 * sigma_error;  // FWHMのエラー
    
    // 最大値を見つける
    double max_counts = 0;
    double max_position = 0;
    for (int i = 0; i < nPoints; i++) {
        if (normalized_counts[i] > max_counts) {
            max_counts = normalized_counts[i];
            max_position = laser_position[i];
        }
    }
    
    cout << "\n========================================" << endl;
    cout << "Gaussian Fit Results:" << endl;
    cout << "  Center: " << mean << " +/- " << mean_error << " mm" << endl;
    cout << "  Sigma: " << sigma << " +/- " << sigma_error << " mm" << endl;
    cout << "  FWHM: " << fwhm << " +/- " << fwhm_error << " mm" << endl;
    cout << "========================================" << endl;
    cout << "Maximum normalized counts: " << max_counts << " at position " << max_position << " mm" << endl;
    
    // フィット曲線を描画
    fit_gauss->Draw("SAME");
    
    // FWHM情報をグラフ上に表示（右上に配置、エラー付き）
    TLegend *leg_fit = new TLegend(0.60, 0.72, 0.88, 0.88);
    leg_fit->SetTextSize(0.028);
    leg_fit->SetHeader("Gaussian Fit:", "C");
    leg_fit->AddEntry(fit_gauss, Form("Center: %.2f #pm %.2f mm", mean, mean_error), "l");
    leg_fit->AddEntry((TObject*)0, Form("FWHM: %.2f #pm %.2f mm", fwhm, fwhm_error), "");
    leg_fit->Draw();
    
    // 画像として保存
    c1->SaveAs("normalized_laser_position.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("normalized_laser_position.root", "RECREATE");
    gr_normalized->Write("gr_normalized");
    gr_measured->Write("gr_measured");
    fit_gauss->Write("fit_gauss");
    outfile->Close();
    
    cout << "\nOutput files created:" << endl;
    cout << "  - normalized_laser_position.png" << endl;
    cout << "  - normalized_laser_position.root" << endl;
    cout << "========================================" << endl;
}

