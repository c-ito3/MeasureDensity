void plot_pinholeoff_counts_fit() {
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
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Counts vs Time", 1200, 800);
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
    
    // 一次関数でフィッティング
    TF1 *fit_linear = new TF1("fit_linear", "[0] + [1]*x", 0, 220);
    fit_linear->SetParameter(0, 4800);  // 初期値
    fit_linear->SetParameter(1, -2);    // 初期値（減少傾向）
    fit_linear->SetLineColor(kRed);
    fit_linear->SetLineWidth(3);
    
    gr_counts->Fit(fit_linear, "R");
    
    // フィット結果を表示
    double p0 = fit_linear->GetParameter(0);
    double p1 = fit_linear->GetParameter(1);
    double chi2 = fit_linear->GetChisquare();
    int ndf = fit_linear->GetNDF();
    
    // フィット情報をテキストボックスで表示
    TPaveText *pt = new TPaveText(0.15, 0.70, 0.45, 0.88, "NDC");
    pt->SetFillColor(kWhite);
    pt->SetTextAlign(12);
    pt->AddText(Form("Fit: y = %.1f + %.3f #times t", p0, p1));
    pt->AddText(Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2/ndf));
    pt->AddText(Form("Slope: %.3f counts/min", p1));
    pt->AddText(Form("Decay rate: %.2f%%/hour", -p1*60/p0*100));
    pt->Draw();
    
    // 画像として保存
    c1->SaveAs("result/pinholeoff_counts_fit.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("result/pinholeoff_counts_fit.root", "RECREATE");
    gr_counts->Write("gr_pinholeoff_counts");
    fit_linear->Write("fit_linear");
    outfile->Close();
    
    cout << "\n========================================" << endl;
    cout << "Pinhole OFF Counts vs Time (Data 5 skipped)" << endl;
    cout << "========================================" << endl;
    cout << "Time [min] | Counts | Data Number" << endl;
    cout << "-----------|--------|------------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        printf("%10.0f | %6.0f | %11d\n", time_min[i], counts[i], data_nums[i]);
    }
    
    cout << "========================================" << endl;
    cout << "Linear Fit: y = " << p0 << " + " << p1 << " * t" << endl;
    cout << "Chi2/ndf: " << chi2 << "/" << ndf << " = " << chi2/ndf << endl;
    cout << "Decay rate: " << -p1*60/p0*100 << " %/hour" << endl;
    cout << "\nOutput files created:" << endl;
    cout << "  - result/pinholeoff_counts_fit.png" << endl;
    cout << "  - result/pinholeoff_counts_fit.root" << endl;
    cout << "========================================" << endl;
}

