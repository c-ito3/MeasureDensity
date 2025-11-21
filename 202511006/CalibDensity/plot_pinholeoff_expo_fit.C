void plot_pinholeoff_expo_fit() {
    // Pinhole OFFのデータ（奇数番号、データ5を除く）
    const int nPoints = 6;
    
    // 測定時刻（開始からの経過時間 [分]）
    // 15:20を0分として計算
    double time_min[nPoints] = {
        0,      // データ1: 15:20
        40,     // データ3: 16:00
        113,    // データ7: 17:13
        148,    // データ9: 17:48
        180,    // データ11: 18:20
        216     // データ13: 18:56
    };
    
    // データ番号
    int data_nums[nPoints] = {1, 3, 7, 9, 11, 13};
    
    // AreaCut後のカウント数（データ5をスキップ）
    double counts[nPoints] = {
        4876,  // データ1
        4626,  // データ3
        4773,  // データ7
        4509,  // データ9
        4459,  // データ11
        4072   // データ13
    };
    
    // グラフを作成
    TGraph *gr_counts = new TGraph(nPoints, time_min, counts);
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Counts vs Time (Exponential Fit)", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    // グラフの設定（点のみ、線なし）
    gr_counts->SetMarkerStyle(20);
    gr_counts->SetMarkerSize(2.0);
    gr_counts->SetMarkerColor(kBlue);
    gr_counts->SetTitle("Ps Counts vs Time (Pinhole OFF);Time [min];Counts");
    gr_counts->GetXaxis()->SetRangeUser(-10, 230);
    gr_counts->GetYaxis()->SetRangeUser(3500, 5500);
    gr_counts->GetXaxis()->SetTitleSize(0.045);
    gr_counts->GetYaxis()->SetTitleSize(0.045);
    gr_counts->GetXaxis()->SetLabelSize(0.04);
    gr_counts->GetYaxis()->SetLabelSize(0.04);
    gr_counts->Draw("AP");
    
    // 指数関数でフィッティング: y = A * exp(B*t)
    TF1 *fit_expo = new TF1("fit_expo", "[0]*TMath::Exp([1]*x)", 0, 220);
    fit_expo->SetParameter(0, 4900);   // 初期値 A
    fit_expo->SetParameter(1, -0.001); // 初期値 B（減少なので負）
    fit_expo->SetLineColor(kRed);
    fit_expo->SetLineWidth(3);
    
    gr_counts->Fit(fit_expo, "R");
    
    // フィット結果を取得
    double A = fit_expo->GetParameter(0);
    double B = fit_expo->GetParameter(1);
    double chi2 = fit_expo->GetChisquare();
    int ndf = fit_expo->GetNDF();
    
    // 半減期を計算: t_half = ln(2) / |B|
    double half_life_min = TMath::Log(2) / TMath::Abs(B);
    double half_life_hour = half_life_min / 60.0;
    
    // 1時間あたりの減少率
    double decay_rate_per_hour = (1 - TMath::Exp(B * 60)) * 100;
    
    // フィット情報をテキストボックスで表示
    TPaveText *pt = new TPaveText(0.15, 0.68, 0.50, 0.88, "NDC");
    pt->SetFillColor(kWhite);
    pt->SetTextAlign(12);
    pt->AddText(Form("Fit: y = %.1f #times e^{%.6f t}", A, B));
    pt->AddText(Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2/ndf));
    pt->AddText(Form("Decay constant: %.6f min^{-1}", B));
    pt->AddText(Form("Half-life: %.1f min (%.2f hours)", half_life_min, half_life_hour));
    pt->AddText(Form("Decay rate: %.2f%%/hour", decay_rate_per_hour));
    pt->Draw();
    
    // 画像として保存
    c1->SaveAs("pinholeoff_expo_fit.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("pinholeoff_expo_fit.root", "RECREATE");
    gr_counts->Write("gr_pinholeoff_counts");
    fit_expo->Write("fit_expo");
    outfile->Close();
    
    cout << "\n========================================" << endl;
    cout << "Pinhole OFF Counts vs Time - Exponential Fit" << endl;
    cout << "Data 5 skipped (measurement error)" << endl;
    cout << "========================================" << endl;
    cout << "Time [min] | Counts | Data Number" << endl;
    cout << "-----------|--------|------------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        printf("%10.0f | %6.0f | %11d\n", time_min[i], counts[i], data_nums[i]);
    }
    
    cout << "========================================" << endl;
    cout << "Exponential Fit: y = " << A << " * exp(" << B << " * t)" << endl;
    cout << "Chi2/ndf: " << chi2 << "/" << ndf << " = " << chi2/ndf << endl;
    cout << "Decay constant: " << B << " /min" << endl;
    cout << "Half-life: " << half_life_min << " min (" << half_life_hour << " hours)" << endl;
    cout << "Decay rate: " << decay_rate_per_hour << " %/hour" << endl;
    cout << "\nOutput files created:" << endl;
    cout << "  - pinholeoff_expo_fit.png" << endl;
    cout << "  - pinholeoff_expo_fit.root" << endl;
    cout << "========================================" << endl;
}

