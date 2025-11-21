void plot_pinholeoff_expo_fit() {
    gSystem->mkdir(".", kTRUE);
    
    // Pinhole OFFのデータ（奇数番号: 1, 3, 5, 7, 9, 11, 13, 15, 17）
    const int nPoints = 9;
    
    // 測定開始時刻（仮の値、後でBook1.xlsxから更新）[分]
    double time_from_start[nPoints] = {
        0,    // File 1
        20,   // File 3
        40,   // File 5
        60,   // File 7
        80,   // File 9
        100,  // File 11
        120,  // File 13
        140,  // File 15
        160   // File 17
    };
    
    int data_nums[nPoints] = {1, 3, 5, 7, 9, 11, 13, 15, 17};
    
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
    
    // Pinhole OFFのカウント数を取得
    double ps_counts[nPoints];
    double ps_errors[nPoints];
    double zero_errors[nPoints];
    
    cout << "\nPinhole OFF data:" << endl;
    cout << "File | Time[min] | Counts" << endl;
    cout << "-----|-----------|-------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        ps_counts[i] = areaCutCounts[data_nums[i]];
        ps_errors[i] = TMath::Sqrt(ps_counts[i]);  // ポアソン統計
        zero_errors[i] = 0;
        
        printf("%4d | %9.0f | %6.0f\n", data_nums[i], time_from_start[i], ps_counts[i]);
    }
    cout << "========================================" << endl;
    
    // グラフを作成（エラーバー付き）
    TGraphErrors *gr = new TGraphErrors(nPoints, time_from_start, ps_counts, zero_errors, ps_errors);
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(2.0);
    gr->SetMarkerColor(kBlue);
    gr->SetLineColor(kBlue);
    gr->SetLineWidth(2);
    gr->SetTitle("Pinhole OFF Data - Exponential Fit;Time from start [min];Ps Counts (after TOF+Area cut)");
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Exponential Fit", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    gr->Draw("APE");
    gr->GetXaxis()->SetTitleSize(0.045);
    gr->GetYaxis()->SetTitleSize(0.045);
    gr->GetXaxis()->SetLabelSize(0.04);
    gr->GetYaxis()->SetLabelSize(0.04);
    
    // 指数関数フィット: y = A + B * exp(C * t)（Bは負、Cは正、t→∞で y→-∞）
    TF1 *fit_expo = new TF1("fit_expo", "[0] + [1]*TMath::Exp([2]*x)", 0, 180);
    fit_expo->SetParameter(0, 2000);   // 初期値 A（オフセット）
    fit_expo->SetParameter(1, -2000);  // 初期値 B（負の値、t→∞で発散方向）
    fit_expo->SetParameter(2, 0.005);  // 初期値 C（正の値、増大）
    fit_expo->SetParLimits(0, -5000, 5000);   // A の範囲を広く
    fit_expo->SetParLimits(1, -5000, 0);      // B の範囲を制限（負の値のみ）
    fit_expo->SetParLimits(2, 0, 0.1);        // C の範囲を制限（正の値のみ）
    fit_expo->SetLineColor(kRed);
    fit_expo->SetLineWidth(3);
    
    gr->Fit(fit_expo, "R");
    
    // フィットパラメータを取得
    double A = fit_expo->GetParameter(0);
    double B = fit_expo->GetParameter(1);
    double C = fit_expo->GetParameter(2);
    
    // 凡例を左下に配置
    TLegend *leg = new TLegend(0.15, 0.15, 0.65, 0.35);
    leg->SetTextSize(0.035);
    leg->AddEntry(gr, "Pinhole OFF data", "p");
    leg->AddEntry(fit_expo, Form("Fit: %.1f + %.1f e^{%.4f t}", A, B, C), "l");
    leg->Draw();
    
    // 平均値を計算して表示
    double sum_counts = 0;
    for (int i = 0; i < nPoints; i++) {
        sum_counts += ps_counts[i];
    }
    double avg_counts = sum_counts / nPoints;
    
    TLatex *text = new TLatex();
    text->SetNDC();
    text->SetTextSize(0.04);
    text->SetTextColor(kGreen+2);
    text->DrawLatex(0.55, 0.55, Form("Average: %.0f", avg_counts));
    
    cout << "\n========================================" << endl;
    cout << "Exponential Fit Results:" << endl;
    cout << "  y = A + B * exp(C * t)  (t→∞, y→-∞)" << endl;
    cout << "  A = " << A << " (offset)" << endl;
    cout << "  B = " << B << " (amplitude, negative)" << endl;
    cout << "  C = " << C << " /min (growth constant, positive)" << endl;
    cout << "========================================" << endl;
    cout << "Average Ps counts: " << avg_counts << endl;
    cout << "========================================" << endl;
    
    // 画像として保存
    c1->SaveAs("pinholeoff_expo_fit.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("pinholeoff_expo_fit.root", "RECREATE");
    gr->Write("gr_pinholeoff");
    fit_expo->Write("fit_expo");
    outfile->Close();
    
    cout << "\nOutput files created:" << endl;
    cout << "  - pinholeoff_expo_fit.png" << endl;
    cout << "  - pinholeoff_expo_fit.root" << endl;
    cout << "========================================" << endl;
}

