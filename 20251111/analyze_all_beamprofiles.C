void analyze_all_beamprofiles() {
    // 出力ディレクトリを作成
    gSystem->mkdir("Analized", kTRUE);
    
    // 解析するファイル番号のリスト（01-14）
    const int nFiles = 14;
    int fileNums[nFiles] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    
    for (int iFile = 0; iFile < nFiles; iFile++) {
        int fileNum = fileNums[iFile];
        
        // 入力ファイル名
        TString inputFile = Form("Measured/20251111_Beamprofile50_%02d_DAn.txt", fileNum);
        
        // データファイルを開く
        std::ifstream infile(inputFile.Data());
    if (!infile.is_open()) {
            std::cerr << "Error: Cannot open file " << inputFile << std::endl;
            continue;
    }
    
        // 2Dヒストグラム作成
        TH2F *h2_profile = new TH2F(Form("h2_profile_%02d", fileNum), 
                                     "Beam Profile;positionX [mm];positionY [mm]", 
                                 200, -50, 50, 200, -50, 50);
    
        // データを読み込み
        std::string line;
        int validEvents = 0;
        
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
    int no, frag;
    double time, posX, posY, tof;
            
            // 新しいフォーマット: No time frag posX posY tof
            if (iss >> no >> time >> frag >> posX >> posY >> tof) {
        if (frag == 1) {
                    h2_profile->Fill(posX, posY);
                    validEvents++;
        }
    }
        }
    infile.close();
    
        std::cout << "File " << fileNum << ": " << validEvents << " valid events" << std::endl;
    
        // キャンバス作成
        TCanvas *c1 = new TCanvas(Form("c1_%02d", fileNum), "Beam Profile", 1000, 900);
        gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.15);
        gPad->SetBottomMargin(0.12);
        
    gStyle->SetPalette(kRainbow);
        
        h2_profile->GetXaxis()->SetTitleSize(0.045);
        h2_profile->GetYaxis()->SetTitleSize(0.045);
        h2_profile->GetZaxis()->SetTitleSize(0.045);
        h2_profile->GetXaxis()->SetLabelSize(0.04);
        h2_profile->GetYaxis()->SetLabelSize(0.04);
        h2_profile->GetZaxis()->SetLabelSize(0.04);
        h2_profile->Draw("COLZ");
    
        // グリッド線を直接描画（10mm間隔）
        c1->Update();
        
        // 垂直グリッド線（X = -40, -30, -20, -10, 0, 10, 20, 30, 40）
        for (int i = -4; i <= 4; i++) {
            TLine *line_v = new TLine(i*10, -50, i*10, 50);
            line_v->SetLineColor(kBlack);
            line_v->SetLineStyle(2);
            line_v->SetLineWidth(1);
            line_v->Draw();
    }
    
        // 水平グリッド線（Y = -40, -30, -20, -10, 0, 10, 20, 30, 40）
        for (int i = -4; i <= 4; i++) {
            TLine *line_h = new TLine(-50, i*10, 50, i*10);
            line_h->SetLineColor(kBlack);
            line_h->SetLineStyle(2);
            line_h->SetLineWidth(1);
            line_h->Draw();
    }
    
        // 保存
        TString outputFile = Form("Analized/profile%02d_2D.png", fileNum);
        c1->SaveAs(outputFile);
        
        delete h2_profile;
        delete c1;
    }
    
    std::cout << "\nAll beam profiles analyzed successfully!" << std::endl;
}
