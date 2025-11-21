void analyze_tofcut() {
    // 出力ディレクトリを作成
    gSystem->mkdir("TOFcut", kTRUE);
    
    // 解析するファイル番号のリスト
    const int nFiles = 14;
    int fileNums[nFiles] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    
    // TOFカット条件
    const double tof_min = 196.0;
    const double tof_max = 210.0;
    
    for (int iFile = 0; iFile < nFiles; iFile++) {
        int fileNum = fileNums[iFile];
        
        TString inputFile = Form("Measured/20251111_Beamprofile50_%02d_DAn.txt", fileNum);
        
        std::ifstream infile(inputFile.Data());
    if (!infile.is_open()) {
            std::cerr << "Error: Cannot open file " << inputFile << std::endl;
            continue;
    }
    
        TH2F *h2_tofcut = new TH2F(Form("h2_tofcut_%02d", fileNum), 
                                    Form("Beam Profile (TOF Cut);positionX [mm];positionY [mm]"), 
                                 200, -50, 50, 200, -50, 50);
    
        std::string line;
        int validEvents = 0;
        int tofCutEvents = 0;
        
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            int no, frag;
            double time, posX, posY, tof;
            
            // 新しいフォーマット: No time frag posX posY tof
            if (iss >> no >> time >> frag >> posX >> posY >> tof) {
                if (frag == 1) {
                    validEvents++;
                    // TOFカット適用
                    if (tof > tof_min && tof < tof_max) {
                        h2_tofcut->Fill(posX, posY);
                        tofCutEvents++;
                }
            }
        }
    }
    infile.close();
    
        std::cout << "File " << fileNum << ": " << validEvents << " valid events, " 
                  << tofCutEvents << " after TOF cut" << std::endl;
        
        // キャンバス作成
        TCanvas *c1 = new TCanvas(Form("c1_tofcut_%02d", fileNum), "TOF Cut Profile", 1000, 900);
        gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.15);
        gPad->SetBottomMargin(0.12);
        
    gStyle->SetPalette(kRainbow);
        
        h2_tofcut->GetXaxis()->SetTitleSize(0.045);
        h2_tofcut->GetYaxis()->SetTitleSize(0.045);
        h2_tofcut->GetZaxis()->SetTitleSize(0.045);
        h2_tofcut->GetXaxis()->SetLabelSize(0.04);
        h2_tofcut->GetYaxis()->SetLabelSize(0.04);
        h2_tofcut->GetZaxis()->SetLabelSize(0.04);
        h2_tofcut->Draw("COLZ");
    
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
    
        TString outputFile = Form("TOFcut/profile%02d_tofcut_2D.png", fileNum);
        c1->SaveAs(outputFile);
        
        delete h2_tofcut;
        delete c1;
    }
    
    std::cout << "\nTOF cut analysis completed!" << std::endl;
}
