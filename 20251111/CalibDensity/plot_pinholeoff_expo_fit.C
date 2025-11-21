void plot_pinholeoff_expo_fit() {
    gSystem->mkdir(".", kTRUE);  // 確実にディレクトリが存在するようにする
    
    // Pinhole OFFのデータ（奇数番号: 1, 3, 5, 7, 9, 11, 13）
    const int nPoints = 7;
    int dataNum[nPoints] = {1, 3, 5, 7, 9, 11, 13};
    
    // 測定開始時刻（17:17からの経過時間 [分]）
    double startTime[nPoints] = {
        0,    // 1: 17:17
        46,   // 3: 18:03
        85,   // 5: 18:42
        138,  // 7: 19:35
        178,  // 9: 20:15
        228,  // 11: 21:05
        268   // 13: 21:45
    };
    
    // AreaCutのカウント数を読み込む
    std::ifstream countFile("../AreaCut/areacut_counts.txt");
    if (!countFile.is_open()) {
        std::cerr << "Error: Cannot open ../AreaCut/areacut_counts.txt" << std::endl;
        std::cerr << "Please run analyze_areacut.C first!" << std::endl;
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
    double counts[nPoints];
    for (int i = 0; i < nPoints; i++) {
        counts[i] = areaCutCounts[dataNum[i]];
        std::cout << "Data " << dataNum[i] << " (t=" << startTime[i] << " min): " 
                  << counts[i] << " counts" << std::endl;
    }
    
    // グラフを作成
    TGraphErrors *gr_counts = new TGraphErrors(nPoints, startTime, counts);
    
    // 指数関数でフィッティング: y = A * exp(B*t)
    TF1 *fit_expo = new TF1("fit_expo", "[0]*TMath::Exp([1]*x)", 0, 300);
    fit_expo->SetParameter(0, 5000);   // 初期値 A
    fit_expo->SetParameter(1, -0.001); // 初期値 B（減少なので負）
    fit_expo->SetLineColor(kRed);
    fit_expo->SetLineWidth(3);
    
    gr_counts->Fit(fit_expo, "R");
    
    // フィットパラメータを取得
    double A = fit_expo->GetParameter(0);
    double B = fit_expo->GetParameter(1);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Fit function: y = " << A << " * exp(" << B << " * t)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Exponential Fit", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    gr_counts->SetMarkerStyle(20);
    gr_counts->SetMarkerSize(2.0);
    gr_counts->SetMarkerColor(kBlue);
    gr_counts->SetLineColor(kBlue);
    gr_counts->SetLineWidth(2);
    gr_counts->SetTitle("Pinhole OFF Data - Exponential Fit;Time from start [min];Ps Counts (after TOF+Area cut)");
    gr_counts->GetXaxis()->SetTitleSize(0.045);
    gr_counts->GetYaxis()->SetTitleSize(0.045);
    gr_counts->GetXaxis()->SetLabelSize(0.04);
    gr_counts->GetYaxis()->SetLabelSize(0.04);
    gr_counts->GetXaxis()->SetRangeUser(-10, 280);
    gr_counts->Draw("APE");
    
    // フィット曲線を描画
    fit_expo->Draw("SAME");
    
    // 凡例を追加
    TLegend *leg = new TLegend(0.55, 0.70, 0.88, 0.88);
    leg->SetTextSize(0.035);
    leg->AddEntry(gr_counts, "Pinhole OFF data", "lpe");
    leg->AddEntry(fit_expo, Form("Fit: %.1f #times e^{%.6f t}", A, B), "l");
    leg->Draw();
    
    // 平均値を計算して表示
    double sum = 0;
    for (int i = 0; i < nPoints; i++) {
        sum += counts[i];
    }
    double average = sum / nPoints;
    
    TLine *avgLine = new TLine(-10, average, 280, average);
    avgLine->SetLineColor(kGreen+2);
    avgLine->SetLineStyle(2);
    avgLine->SetLineWidth(2);
    avgLine->Draw();
    
    TText *avgText = new TText(200, average + 100, Form("Average: %.0f", average));
    avgText->SetTextSize(0.035);
    avgText->SetTextColor(kGreen+2);
    avgText->Draw();
    
    // 保存
    c1->SaveAs("pinholeoff_expo_fit.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("pinholeoff_expo_fit.root", "RECREATE");
    gr_counts->Write("gr_pinholeoff_counts");
    fit_expo->Write("fit_expo");
    outfile->Close();
    
    std::cout << "\nOutput files created:" << std::endl;
    std::cout << "  - pinholeoff_expo_fit.png" << std::endl;
    std::cout << "  - pinholeoff_expo_fit.root" << std::endl;
    std::cout << "========================================" << std::endl;
}





