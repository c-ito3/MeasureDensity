void plot_laser_position_vs_counts() {
    // Pinhole ONのデータ（偶数番号）
    const int nPoints = 7;
    
    // レーザー位置 [mm]
    double laser_position[nPoints] = {
        0.0,   // profile 2
        -1.0,  // profile 4
        -2.0,  // profile 6
        1.0,   // profile 8
        2.0,   // profile 10
        -0.5,  // profile 12
        0.5    // profile 14
    };
    
    // AreaCut後のカウント数
    double counts[nPoints] = {
        3676,  // profile 2
        314,   // profile 4
        107,   // profile 6
        1152,  // profile 8
        120,   // profile 10
        873,   // profile 12
        3466   // profile 14
    };
    
    // グラフを作成
    TGraph *gr_counts = new TGraph(nPoints, laser_position, counts);
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Laser Position vs Counts", 1200, 800);
    gPad->SetGrid();
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.12);
    
    // グラフの設定（線なし、点のみ）
    gr_counts->SetMarkerStyle(20);
    gr_counts->SetMarkerSize(2.0);
    gr_counts->SetMarkerColor(kBlue);
    gr_counts->SetTitle("Ps- Counts vs Laser Position (Pinhole ON);Laser Position [mm];Counts");
    gr_counts->GetXaxis()->SetRangeUser(-2.5, 2.5);
    gr_counts->GetYaxis()->SetRangeUser(0, 4000);
    gr_counts->GetXaxis()->SetTitleSize(0.045);
    gr_counts->GetYaxis()->SetTitleSize(0.045);
    gr_counts->GetXaxis()->SetLabelSize(0.04);
    gr_counts->GetYaxis()->SetLabelSize(0.04);
    gr_counts->Draw("AP");
    
    // 最大値の位置にマーカー
    double max_counts = 0;
    double max_position = 0;
    for (int i = 0; i < nPoints; i++) {
        if (counts[i] > max_counts) {
            max_counts = counts[i];
            max_position = laser_position[i];
        }
    }
    
    // 最大値の位置に縦線を引く
    TLine *line_max = new TLine(max_position, 0, max_position, max_counts);
    line_max->SetLineColor(kRed);
    line_max->SetLineStyle(2);
    line_max->SetLineWidth(2);
    line_max->Draw();
    
    // 最大値を表示
    TText *text_max = new TText(max_position + 0.1, max_counts + 200, 
                                Form("Max: %.0f counts at %.1f mm", max_counts, max_position));
    text_max->SetTextColor(kRed);
    text_max->SetTextSize(0.04);
    text_max->Draw();
    
    // 画像として保存
    c1->SaveAs("laser_position_vs_counts.png");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("laser_position_vs_counts.root", "RECREATE");
    gr_counts->Write("gr_laser_position_vs_counts");
    outfile->Close();
    
    cout << "\n========================================" << endl;
    cout << "Laser Position vs Counts (Pinhole ON)" << endl;
    cout << "========================================" << endl;
    cout << "Position [mm] | Counts | Profile" << endl;
    cout << "--------------|--------|--------" << endl;
    
    int profile_nums[nPoints] = {2, 4, 6, 8, 10, 12, 14};
    for (int i = 0; i < nPoints; i++) {
        printf("%13.1f | %6.0f | %7d\n", laser_position[i], counts[i], profile_nums[i]);
    }
    
    cout << "========================================" << endl;
    cout << "Maximum counts: " << max_counts << " at position " << max_position << " mm" << endl;
    cout << "\nOutput files created:" << endl;
    cout << "  - laser_position_vs_counts.png" << endl;
    cout << "  - laser_position_vs_counts.root" << endl;
    cout << "========================================" << endl;
}

