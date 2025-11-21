void plot_pinholeoff_counts() {
    // Pinhole OFFのデータ（奇数番号）
    const int nPoints = 7;
    
    // データ番号（profile番号）
    double data_num[nPoints] = {1, 3, 5, 7, 9, 11, 13};
    
    // AreaCut後のカウント数（TOF+エリアカット適用後）
    double counts[nPoints] = {
        4876,  // profile 1
        4626,  // profile 3
        3146,  // profile 5
        4773,  // profile 7
        4509,  // profile 9
        4459,  // profile 11
        4072   // profile 13
    };
    
    // グラフを作成
    TGraph *gr_counts = new TGraph(nPoints, data_num, counts);
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Pinhole OFF Counts", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    // グラフの設定（点のみ、線なし）
    gr_counts->SetMarkerStyle(20);
    gr_counts->SetMarkerSize(2.0);
    gr_counts->SetMarkerColor(kBlue);
    gr_counts->SetTitle("Ps- Counts vs Data Number (Pinhole OFF);Data Number;Counts");
    gr_counts->GetXaxis()->SetRangeUser(0, 14);
    gr_counts->GetYaxis()->SetRangeUser(0, 5500);
    gr_counts->GetXaxis()->SetTitleSize(0.045);
    gr_counts->GetYaxis()->SetTitleSize(0.045);
    gr_counts->GetXaxis()->SetLabelSize(0.04);
    gr_counts->GetYaxis()->SetLabelSize(0.04);
    gr_counts->Draw("AP");
    
    // 平均値を計算
    double sum_counts = 0;
    for (int i = 0; i < nPoints; i++) {
        sum_counts += counts[i];
    }
    double avg_counts = sum_counts / nPoints;
    
    // 平均値のラインを引く
    TLine *line_avg = new TLine(0, avg_counts, 14, avg_counts);
    line_avg->SetLineColor(kRed);
    line_avg->SetLineStyle(2);
    line_avg->SetLineWidth(2);
    line_avg->Draw();
    
    // 平均値を表示（右寄せで見切れないように）
    TText *text_avg = new TText(10, avg_counts + 200, Form("Average: %.0f counts", avg_counts));
    text_avg->SetTextColor(kRed);
    text_avg->SetTextSize(0.04);
    text_avg->Draw();
    
    // 画像として保存
    c1->SaveAs("result/pinholeoff_counts.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("result/pinholeoff_counts.root", "RECREATE");
    gr_counts->Write("gr_pinholeoff_counts");
    outfile->Close();
    
    cout << "\n========================================" << endl;
    cout << "Pinhole OFF Counts (with TOF+Area Cut)" << endl;
    cout << "========================================" << endl;
    cout << "Data Number | Counts" << endl;
    cout << "------------|-------" << endl;
    
    for (int i = 0; i < nPoints; i++) {
        printf("%11.0f | %6.0f\n", data_num[i], counts[i]);
    }
    
    cout << "========================================" << endl;
    cout << "Average: " << avg_counts << " counts" << endl;
    cout << "Std Dev: " << TMath::RMS(nPoints, counts) << " counts" << endl;
    cout << "\nOutput files created:" << endl;
    cout << "  - result/pinholeoff_counts.png" << endl;
    cout << "  - result/pinholeoff_counts.root" << endl;
    cout << "========================================" << endl;
}

