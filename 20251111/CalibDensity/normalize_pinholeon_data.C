void normalize_pinholeon_data() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1111);
    
    // Pinhole ONのデータ（偶数番号: 2, 4, 6, 8, 10, 12, 14）
    const int nPoints = 7;
    int data_nums[nPoints] = {2, 4, 6, 8, 10, 12, 14};
    
    // レーザー変位 [mm]（19.7mmを基準=0として変位を計算、符号逆転）
    double laser_position[nPoints] = {
        19.7 - 19.7,   // データ2: 19.7 → 0.0
        19.7 - 20.7,   // データ4: 20.7 → -1.0
        19.7 - 18.7,   // データ6: 18.7 → 1.0
        19.7 - 19.2,   // データ8: 19.2 → 0.5
        19.7 - 20.2,   // データ10: 20.2 → -0.5
        19.7 - 17.7,   // データ12: 17.7 → 2.0
        19.7 - 21.7    // データ14: 21.7 → -2.0
    };
    
    // AreaCutのカウント数を読み込む
    std::ifstream countFile("../AreaCut/areacut_counts.txt");
    if (!countFile.is_open()) {
        // カレントディレクトリも探す
        countFile.open("areacut_counts.txt");
    }
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
    
    cout << "\nNormalization Results (20251111 Neighbor Average):" << endl;
    cout << "Ref Count (File 001): " << ref_count << endl;
    cout << "File | Pos[mm] | Prev(OFF) | Next(OFF) | Avg(OFF) | Factor | Raw | Norm" << endl;
    cout << "-----|---------|-----------|-----------|----------|--------|-----|-----" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        int fileNum = data_nums[i];
        int prevFile = fileNum - 1;
        int nextFile = fileNum + 1;
        
        // 前後のOFFファイルが存在するか確認
        double avgCount = 0;
        double prevCount = 0; 
        double nextCount = 0;
        
        if (counts.find(prevFile) != counts.end()) prevCount = counts[prevFile];
        if (counts.find(nextFile) != counts.end()) nextCount = counts[nextFile];
        
        if (prevCount > 0 && nextCount > 0) {
            avgCount = (prevCount + nextCount) / 2.0;
        } else if (prevCount > 0) {
            avgCount = prevCount; // 次がない場合は前のみ
        } else if (nextCount > 0) {
            avgCount = nextCount; // 前がない場合は次のみ
        } else {
            std::cerr << "Error: Missing neighbor files for File " << fileNum << std::endl;
            continue;
        }
        
        double factor = ref_count / avgCount;
        
        x[i] = laser_position[i];
        x_err[i] = 0;
        
        y_raw[i] = counts[fileNum];
        y_raw_err[i] = TMath::Sqrt(y_raw[i]);
        
        y_norm[i] = y_raw[i] * factor;
        y_norm_err[i] = y_raw_err[i] * factor;
        
        printf("%4d | %7.2f | %9.0f | %9.0f | %8.1f | %6.4f | %3.0f | %5.1f\n",
               fileNum, x[i], prevCount, nextCount, avgCount, factor, y_raw[i], y_norm[i]);
    }
    
    // グラフ作成
    TGraphErrors *gr_raw = new TGraphErrors(nPoints, x, y_raw, x_err, y_raw_err);
    TGraphErrors *gr_norm = new TGraphErrors(nPoints, x, y_norm, x_err, y_norm_err);
    
    TCanvas *c1 = new TCanvas("c1", "Normalized Ps Counts vs Laser Displacement", 1200, 800);
    gPad->SetGrid();
    gPad->SetRightMargin(0.05);
    
    gr_norm->SetTitle("Normalized Ps Counts vs Laser Displacement (20251111 Neighbor Avg);Laser Displacement [mm];Normalized Counts");
    gr_norm->SetMarkerStyle(20);
    gr_norm->SetMarkerSize(1.5);
    gr_norm->SetMarkerColor(kRed);
    gr_norm->SetLineColor(kRed);
    gr_norm->GetYaxis()->SetRangeUser(0, 5500); // 範囲調整
    gr_norm->Draw("APE");
    
    gr_raw->SetMarkerStyle(24);
    gr_raw->SetMarkerColor(kBlue);
    gr_raw->SetLineColor(kBlue);
    gr_raw->Draw("P SAME");
    
    // 凡例 (左上)
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
    
    // フィット結果表示 (右上、統計ボックスの下)
    TLegend *leg2 = new TLegend(0.62, 0.45, 0.98, 0.65);
    leg2->SetHeader("Gaussian Fit (20251111)", "C");
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
