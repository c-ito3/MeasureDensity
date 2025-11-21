void plot_pinholeoff_data() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    
    // Pinhole OFFのデータ（奇数番号）
    const int nPoints = 9;
    int data_nums[nPoints] = {1, 3, 5, 7, 9, 11, 13, 15, 17};
    
    // 測定開始時刻（推定含む）[分]
    double time_from_start[nPoints] = {
        0,    // File 1
        11,   // File 3
        35,   // File 5
        56,   // File 7
        71,   // File 9
        115,  // File 11
        132,  // File 13
        148,  // File 15
        164   // File 17 (推定: File 15 + 16min)
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
    
    double x[nPoints], y[nPoints], y_err[nPoints], x_err[nPoints];
    
    cout << "Pinhole OFF Data:" << endl;
    cout << "File | Time[min] | Counts" << endl;
    cout << "-----|-----------|-------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        int fileNum = data_nums[i];
        x[i] = time_from_start[i];
        x_err[i] = 0;
        
        if (counts.find(fileNum) != counts.end()) {
            y[i] = counts[fileNum];
            y_err[i] = TMath::Sqrt(y[i]);
        } else {
            y[i] = 0;
            y_err[i] = 0;
            std::cerr << "Warning: No count for File " << fileNum << std::endl;
        }
        
        printf("%4d | %9.0f | %6.0f\n", fileNum, x[i], y[i]);
    }
    
    TGraphErrors *gr = new TGraphErrors(nPoints, x, y, x_err, y_err);
    
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Data", 1000, 600);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    
    gr->SetTitle("Pinhole OFF Data Evolution;Time form start [min];Ps Counts (after TOF+Area cut)");
    gr->SetMarkerStyle(21);
    gr->SetMarkerSize(1.5);
    gr->SetMarkerColor(kBlue);
    gr->SetLineColor(kBlue);
    gr->SetLineWidth(2);
    gr->GetYaxis()->SetRangeUser(0, 2500);
    gr->Draw("APE");
    
    // 平均値を線で表示（参考）
    double sum = 0;
    for(int i=0; i<nPoints; i++) sum += y[i];
    double avg = sum / nPoints;
    
    TLine *lineAvg = new TLine(0, avg, 180, avg);
    lineAvg->SetLineColor(kGreen+2);
    lineAvg->SetLineStyle(2);
    lineAvg->SetLineWidth(2);
    lineAvg->Draw("SAME");
    
    TLegend *leg = new TLegend(0.15, 0.15, 0.45, 0.30);
    leg->AddEntry(gr, "Pinhole OFF Data", "lp");
    leg->AddEntry(lineAvg, Form("Average: %.1f", avg), "l");
    leg->Draw();
    
    c1->SaveAs("pinholeoff_data_plot.png");
}

