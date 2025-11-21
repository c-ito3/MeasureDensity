void analyze_beamprofile() {
    // ファイルを開く
    ifstream infile("測定データ/20251106_Beamprofile50_01_DAn.txt");
    
    // ヒストグラムを作成（ビン幅0.5mm）
    TH2F *h2_position = new TH2F("h2_position", "Beam Profile (X-Y Position);Position X [mm];Position Y [mm]",
                                 200, -50, 50, 200, -50, 50);
    TH1F *h1_pulseheight = new TH1F("h1_pulseheight", "Pulse Height Distribution;Pulse Height;Counts",
                                     200, -50000, 0);
    
    // ヘッダー行をスキップ
    string header;
    getline(infile, header);
    
    // データを読み込む
    int no, frag;
    double time, pulseheight, ignore, posX, posY;
    int count = 0;
    int valid_events = 0;
    
    while (infile >> no >> time >> frag >> pulseheight >> ignore >> posX >> posY) {
        count++;
        
        // 残りの列を読み飛ばす
        string dummy_line;
        getline(infile, dummy_line);
        
        // frag=1の有効なイベントのみプロット
        if (frag == 1 && posX != 0 && posY != 0) {
            h2_position->Fill(posX, posY);
            h1_pulseheight->Fill(pulseheight);
            valid_events++;
        }
        
        // 進捗表示
        if (count % 5000 == 0) {
            cout << "Processed " << count << " events, valid: " << valid_events << endl;
        }
    }
    
    infile.close();
    
    cout << "\n=== Summary ===" << endl;
    cout << "Total events: " << count << endl;
    cout << "Valid events (frag=1): " << valid_events << endl;
    
    // キャンバスを作成して描画
    TCanvas *c1 = new TCanvas("c1", "Beam Profile Analysis", 1400, 600);
    c1->Divide(2, 1);
    
    // 2Dヒストグラム
    c1->cd(1);
    gPad->SetRightMargin(0.15);
    gStyle->SetPalette(kRainbow);  // レインボーカラーパレット
    gStyle->SetOptStat(0);
    h2_position->SetStats(0);
    
    // まずヒストグラムの枠だけを描画
    h2_position->Draw("AXIS");
    
    // 10mm単位のグリッド線を追加（黒色、データの背面）
    TLine *line;
    line = new TLine();
    line->SetLineColor(kBlack);
    line->SetLineStyle(2);  // 破線
    line->SetLineWidth(1);
    
    // 縦線（X方向）
    for (int x = -50; x <= 50; x += 10) {
        TLine *vline = new TLine(x, -50, x, 50);
        vline->SetLineColor(kBlack);
        vline->SetLineStyle(2);
        vline->SetLineWidth(1);
        vline->Draw();
    }
    
    // 横線（Y方向）
    for (int y = -50; y <= 50; y += 10) {
        TLine *hline = new TLine(-50, y, 50, y);
        hline->SetLineColor(kBlack);
        hline->SetLineStyle(2);
        hline->SetLineWidth(1);
        hline->Draw();
    }
    
    // 最後にヒストグラムのデータを重ねて描画
    h2_position->Draw("COLZ SAME");
    
    // パルスハイト分布
    c1->cd(2);
    h1_pulseheight->SetLineColor(kBlue);
    h1_pulseheight->SetFillColor(kBlue);
    h1_pulseheight->SetFillStyle(3001);
    h1_pulseheight->Draw();
    
    // 画像として保存
    c1->SaveAs("beamprofile_2D.png");
    c1->SaveAs("beamprofile_2D.pdf");
    
    // ROOTファイルに保存
    TFile *outfile = new TFile("beamprofile_analysis.root", "RECREATE");
    h2_position->Write();
    h1_pulseheight->Write();
    outfile->Close();
    
    cout << "\nOutput files created:" << endl;
    cout << "  - beamprofile_2D.png" << endl;
    cout << "  - beamprofile_2D.pdf" << endl;
    cout << "  - beamprofile_analysis.root" << endl;
    
    // 統計情報を表示
    cout << "\n=== Position Statistics ===" << endl;
    cout << "Mean X: " << h2_position->GetMean(1) << " mm" << endl;
    cout << "Mean Y: " << h2_position->GetMean(2) << " mm" << endl;
    cout << "RMS X: " << h2_position->GetRMS(1) << " mm" << endl;
    cout << "RMS Y: " << h2_position->GetRMS(2) << " mm" << endl;
}

