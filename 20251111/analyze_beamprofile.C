void analyze_beamprofile() {
    // 入力ファイル名
    const char* inputFile = "Measured/20251111_Beamprofile50_01_DAn.txt";
    const char* outputDir = "Analized/";
    
    // 出力ディレクトリを作成
    gSystem->mkdir(outputDir, kTRUE);
    
    // データファイルを開く
    std::ifstream infile(inputFile);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file " << inputFile << std::endl;
        return;
    }
    
    // 2Dヒストグラム作成（-50～50 mm、ビン幅0.5mm）
    TH2F *h2_profile = new TH2F("h2_profile", "Beam Profile;positionX [mm];positionY [mm]", 
                                 200, -50, 50, 200, -50, 50);
    
    // データを読み込み
    std::string line;
    int validEvents = 0;
    
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int no, frag;
        double time, pulseheight, ignore, posX, posY, tof;
        double dummy[7];
        
        // フォーマット2: No time frag pulseheight ignore posX posY tof (+ 7個のダミー)
        if (iss >> no >> time >> frag >> pulseheight >> ignore >> posX >> posY >> tof) {
            // frag == 1のイベントのみ使用
            if (frag == 1) {
                h2_profile->Fill(posX, posY);
                validEvents++;
            }
        }
    }
    infile.close();
    
    std::cout << "Total valid events (frag==1): " << validEvents << std::endl;
    
    // キャンバス作成
    TCanvas *c1 = new TCanvas("c1", "Beam Profile", 1000, 900);
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.15);
    gPad->SetBottomMargin(0.12);
    
    // カラーパレット設定
    gStyle->SetPalette(kRainbow);
    
    // ヒストグラム描画
    h2_profile->GetXaxis()->SetTitleSize(0.045);
    h2_profile->GetYaxis()->SetTitleSize(0.045);
    h2_profile->GetZaxis()->SetTitleSize(0.045);
    h2_profile->GetXaxis()->SetLabelSize(0.04);
    h2_profile->GetYaxis()->SetLabelSize(0.04);
    h2_profile->GetZaxis()->SetLabelSize(0.04);
    h2_profile->Draw("COLZ");
    
    // グリッド線を描画（10mm間隔、データの背面）
    c1->Update();
    
    // 背面にグリッド線を引くため、透明なパッドを作成
    TPad *gridPad = new TPad("gridPad", "Grid Pad", 0, 0, 1, 1);
    gridPad->SetFillStyle(0);  // 透明
    gridPad->SetFrameFillStyle(0);
    gridPad->Draw();
    gridPad->cd();
    
    // 空のヒストグラムでグリッド範囲を設定
    TH2F *h_dummy = new TH2F("h_dummy", "", 10, -50, 50, 10, -50, 50);
    h_dummy->SetStats(0);
    h_dummy->GetXaxis()->SetLabelOffset(999);
    h_dummy->GetYaxis()->SetLabelOffset(999);
    h_dummy->GetXaxis()->SetTickLength(0);
    h_dummy->GetYaxis()->SetTickLength(0);
    h_dummy->SetLineColor(kBlack);
    h_dummy->SetLineStyle(2);  // 破線
    h_dummy->SetLineWidth(1);
    h_dummy->Draw("SAME");
    
    // 垂直線を描画
    for (int i = -4; i <= 4; i++) {
        TLine *line = new TLine(i*10, -50, i*10, 50);
        line->SetLineColor(kBlack);
        line->SetLineStyle(2);
        line->SetLineWidth(1);
        line->Draw();
    }
    
    // 水平線を描画
    for (int i = -4; i <= 4; i++) {
        TLine *line = new TLine(-50, i*10, 50, i*10);
        line->SetLineColor(kBlack);
        line->SetLineStyle(2);
        line->SetLineWidth(1);
        line->Draw();
    }
    
    // 画像として保存
    TString outputFile = TString(outputDir) + "profile01_2D.png";
    c1->SaveAs(outputFile);
    
    std::cout << "Output saved to: " << outputFile << std::endl;
}





