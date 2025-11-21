void analyze_areacut() {
    // 出力ディレクトリを作成
    gSystem->mkdir("AreaCut", kTRUE);
    
    const int nFiles = 14;
    int fileNums[nFiles] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    
    // カット条件
    const double tof_min = 196.0;
    const double tof_max = 210.0;
    const double x_min = -10.0;
    const double x_max = 10.0;
    const double y_min = -10.0;
    const double y_max = 10.0;
    
    // カウント数を保存
    std::ofstream countFile("AreaCut/areacut_counts.txt");
    countFile << "FileNum\tValid\tTOFcut\tAreaCut\n";
    
    for (int iFile = 0; iFile < nFiles; iFile++) {
        int fileNum = fileNums[iFile];
        
        TString inputFile = Form("Measured/20251111_Beamprofile50_%02d_DAn.txt", fileNum);
        
        std::ifstream infile(inputFile.Data());
    if (!infile.is_open()) {
            std::cerr << "Error: Cannot open file " << inputFile << std::endl;
            continue;
    }
    
        TH2F *h2_areacut = new TH2F(Form("h2_areacut_%02d", fileNum), 
                                     "Beam Profile (TOF+Area Cut);positionX [mm];positionY [mm]", 
                                 200, -50, 50, 200, -50, 50);
    
        std::string line;
        int validEvents = 0;
        int tofCutEvents = 0;
        int areaCutEvents = 0;
        
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
    int no, frag;
    double time, posX, posY, tof;
            
            // 新しいフォーマット: No time frag posX posY tof
            if (iss >> no >> time >> frag >> posX >> posY >> tof) {
        if (frag == 1) {
                    validEvents++;
                    if (tof > tof_min && tof < tof_max) {
                        tofCutEvents++;
                        // エリアカット適用
                        if (posX >= x_min && posX <= x_max && posY >= y_min && posY <= y_max) {
                            h2_areacut->Fill(posX, posY);
                            areaCutEvents++;
                }
            }
        }
    }
        }
    infile.close();
    
        countFile << fileNum << "\t" << validEvents << "\t" << tofCutEvents << "\t" << areaCutEvents << "\n";
        
        std::cout << "File " << fileNum << ": Valid=" << validEvents 
                  << ", TOFcut=" << tofCutEvents 
                  << ", AreaCut=" << areaCutEvents << std::endl;
    
        // キャンバス作成
        TCanvas *c1 = new TCanvas(Form("c1_areacut_%02d", fileNum), "Area Cut Profile", 1000, 900);
        gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.15);
        gPad->SetBottomMargin(0.12);
        
    gStyle->SetPalette(kRainbow);
        
        h2_areacut->GetXaxis()->SetTitleSize(0.045);
        h2_areacut->GetYaxis()->SetTitleSize(0.045);
        h2_areacut->GetZaxis()->SetTitleSize(0.045);
        h2_areacut->GetXaxis()->SetLabelSize(0.04);
        h2_areacut->GetYaxis()->SetLabelSize(0.04);
        h2_areacut->GetZaxis()->SetLabelSize(0.04);
        h2_areacut->Draw("COLZ");
        
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
        
        TString outputFile = Form("AreaCut/profile%02d_areacut_2D.png", fileNum);
        c1->SaveAs(outputFile);
        
        delete h2_areacut;
        delete c1;
    }
    
    countFile.close();
    std::cout << "\nArea cut analysis completed!" << std::endl;
    std::cout << "Count data saved to: AreaCut/areacut_counts.txt" << std::endl;
}
