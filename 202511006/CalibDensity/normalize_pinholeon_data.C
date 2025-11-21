void normalize_pinholeon_data() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1111);
    
    // Pinhole ONのデータ（データ4をスキップ）
    const int nPoints = 6;
    int data_nums[nPoints] = {2, 6, 8, 10, 12, 14};
    
    // レーザー位置 [mm]
    double laser_position[nPoints] = {
        0.0,   // データ2
        -2.0,  // データ6
        1.0,   // データ8
        2.0,   // データ10
        -0.5,  // データ12
        0.5    // データ14
    };
    
    // 実測カウント数（AreaCut後）
    double measured_counts[nPoints] = {
        3676,  // データ2
        107,   // データ6
        1152,  // データ8
        120,   // データ10
        873,   // データ12
        3466   // データ14
    };
    
    // OFFデータ（Neighbor Average用）
    std::map<int, double> off_counts;
    off_counts[1] = 4876;
    off_counts[3] = 4626;
    // off_counts[5] = skipped;
    off_counts[7] = 4773;
    off_counts[9] = 4509;
    off_counts[11] = 4459;
    off_counts[13] = 4072;
    
    double ref_count = off_counts[1];
    
    // 配列の準備
    double x[nPoints], y_raw[nPoints], y_norm[nPoints];
    double y_raw_err[nPoints], y_norm_err[nPoints], x_err[nPoints];
    
    cout << "\nNormalization Results (20251106 Neighbor Average):" << endl;
    cout << "Ref Count (File 001): " << ref_count << endl;
    cout << "File | Pos[mm] | Prev(OFF) | Next(OFF) | Avg(OFF) | Factor | Raw | Norm" << endl;
    cout << "-----|---------|-----------|-----------|----------|--------|-----|-----" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        int fileNum = data_nums[i];
        int prevFile = fileNum - 1;
        int nextFile = fileNum + 1;
        
        double avgCount = 0;
        double prevCount = 0;
        double nextCount = 0;
        
        if (off_counts.find(prevFile) != off_counts.end()) prevCount = off_counts[prevFile];
        if (off_counts.find(nextFile) != off_counts.end()) nextCount = off_counts[nextFile];
        
        if (prevCount > 0 && nextCount > 0) {
            avgCount = (prevCount + nextCount) / 2.0;
        } else if (prevCount > 0) {
            avgCount = prevCount;
        } else if (nextCount > 0) {
            avgCount = nextCount;
        } else {
            // File 6の場合、Prev(5)がないのでNext(7)を使うロジックに入るはずだが、
            // もし両方なければスキップ
            std::cerr << "Error: Missing neighbor files for File " << fileNum << std::endl;
            continue;
        }
        
        double factor = ref_count / avgCount;
        
        x[i] = laser_position[i];
        x_err[i] = 0;
        y_raw[i] = measured_counts[i];
        y_raw_err[i] = TMath::Sqrt(y_raw[i]);
        y_norm[i] = y_raw[i] * factor;
        y_norm_err[i] = y_raw_err[i] * factor;
        
        printf("%4d | %7.2f | %9.0f | %9.0f | %8.1f | %6.4f | %3.0f | %5.1f\n",
               fileNum, x[i], prevCount, nextCount, avgCount, factor, y_raw[i], y_norm[i]);
    }
    
    // グラフ作成
    TGraphErrors *gr_raw = new TGraphErrors(nPoints, x, y_raw, x_err, y_raw_err);
    TGraphErrors *gr_norm = new TGraphErrors(nPoints, x, y_norm, x_err, y_norm_err);
    
    TCanvas *c1 = new TCanvas("c1", "Normalized Ps Counts vs Laser Position", 1200, 800);
    gPad->SetGrid();
    gPad->SetRightMargin(0.05);
    
    gr_norm->SetTitle("Normalized Ps Counts vs Laser Displacement (20251106 Neighbor Avg);Laser Displacement [mm];Normalized Counts");
    gr_norm->SetMarkerStyle(20);
    gr_norm->SetMarkerSize(1.5);
    gr_norm->SetMarkerColor(kRed);
    gr_norm->SetLineColor(kRed);
    gr_norm->GetYaxis()->SetRangeUser(0, 5500);
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
    TF1 *f1 = new TF1("f1", "gaus", -2.5, 2.5);
    f1->SetLineColor(kGreen+2);
    f1->SetLineWidth(3);
    f1->SetParameters(4000, 0, 0.5);
    
    TFitResultPtr fitRes = gr_norm->Fit(f1, "RS");
    
    double mean = f1->GetParameter(1);
    double sigma = f1->GetParameter(2);
    double mean_err = f1->GetParError(1);
    double sigma_err = f1->GetParError(2);
    double fwhm = 2.355 * sigma;
    double fwhm_err = 2.355 * sigma_err;
    
    // フィット結果表示
    TLegend *leg2 = new TLegend(0.62, 0.45, 0.98, 0.65);
    leg2->SetHeader("Gaussian Fit (20251106)", "C");
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
