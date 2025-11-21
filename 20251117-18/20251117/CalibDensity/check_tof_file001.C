void check_tof_file001() {
    TString filename = "../Measure/20251117_Beamprofile50_001_DAn.txt";
    
    std::ifstream infile(filename.Data());
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return;
    }
    
    // ヒストグラム作成
    TH1F *h_tof = new TH1F("h_tof", "TOF Distribution (File 001);TOF;Counts", 300, 0, 300);
    TH1F *h_posX = new TH1F("h_posX", "Position X (File 001);Position X [mm];Counts", 200, -100, 100);
    TH1F *h_posY = new TH1F("h_posY", "Position Y (File 001);Position Y [mm];Counts", 200, -100, 100);
    
    int no, frag;
    double time, col4, col5, posX, posY, tof;
    double col9, col10, col11, col12, col13, col14, col15, col16, col17;
    
    int totalLines = 0;
    int validEvents = 0;
    
    // 17列全て読み込む
    while (infile >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof 
                  >> col9 >> col10 >> col11 >> col12 >> col13 >> col14 >> col15 >> col16 >> col17) {
        totalLines++;
        
        if (frag == 1) {
            validEvents++;
            h_tof->Fill(tof);
            h_posX->Fill(posX);
            h_posY->Fill(posY);
        }
    }
    infile.close();
    
    cout << "========================================" << endl;
    cout << "File: " << filename << endl;
    cout << "Total lines read: " << totalLines << endl;
    cout << "Valid events (frag=1): " << validEvents << endl;
    cout << "========================================" << endl;
    
    // TOF範囲の統計
    int tof_196_210 = 0;
    for (int i = 1; i <= h_tof->GetNbinsX(); i++) {
        double binCenter = h_tof->GetBinCenter(i);
        if (binCenter > 196 && binCenter < 210) {
            tof_196_210 += h_tof->GetBinContent(i);
        }
    }
    cout << "Events with 196 < TOF < 210: " << tof_196_210 << endl;
    cout << "========================================" << endl;
    
    // キャンバス作成
    TCanvas *c1 = new TCanvas("c1", "TOF and Position Check", 1600, 500);
    c1->Divide(3, 1);
    
    c1->cd(1);
    gPad->SetLogy();
    h_tof->SetLineColor(kBlue);
    h_tof->SetLineWidth(2);
    h_tof->Draw();
    
    // TOFカット範囲を示す線
    TLine *line1 = new TLine(196, 0, 196, h_tof->GetMaximum());
    line1->SetLineColor(kRed);
    line1->SetLineStyle(2);
    line1->SetLineWidth(2);
    line1->Draw();
    
    TLine *line2 = new TLine(210, 0, 210, h_tof->GetMaximum());
    line2->SetLineColor(kRed);
    line2->SetLineStyle(2);
    line2->SetLineWidth(2);
    line2->Draw();
    
    c1->cd(2);
    h_posX->SetLineColor(kGreen+2);
    h_posX->SetLineWidth(2);
    h_posX->Draw();
    
    c1->cd(3);
    h_posY->SetLineColor(kMagenta);
    h_posY->SetLineWidth(2);
    h_posY->Draw();
    
    c1->SaveAs("check_tof_file001.png");
    
    cout << "Output saved: check_tof_file001.png" << endl;
}



