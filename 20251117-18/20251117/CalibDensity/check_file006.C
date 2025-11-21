void check_file006() {
    TString filename = "../Measure/20251117_Beamprofile50_006_DAn.txt";
    
    std::ifstream infile(filename.Data());
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return;
    }
    
    // ヒストグラム作成
    TH1F *h_tof = new TH1F("h_tof", "TOF Distribution (File 006);TOF;Counts", 300, 0, 300);
    TH2F *h2_pos = new TH2F("h2_pos", "Position Distribution (File 006);Position X [mm];Position Y [mm]", 200, -100, 100, 200, -100, 100);
    
    int no, frag;
    double time, col4, col5, posX, posY, tof;
    double col9, col10, col11, col12, col13, col14, col15, col16, col17;
    
    int totalLines = 0;
    int validEvents = 0;
    int tofCutEvents = 0;
    int areaCutEvents = 0;
    
    const double tof_min = 196.0;
    const double tof_max = 210.0;
    const double x_min = -10.0;
    const double x_max = 10.0;
    const double y_min = -10.0;
    const double y_max = 10.0;
    
    // 17列全て読み込む
    while (infile >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof 
                  >> col9 >> col10 >> col11 >> col12 >> col13 >> col14 >> col15 >> col16 >> col17) {
        totalLines++;
        
        if (frag == 1) {
            validEvents++;
            h_tof->Fill(tof);
            h2_pos->Fill(posX, posY);
            
            if (tof > tof_min && tof < tof_max) {
                tofCutEvents++;
                if (posX >= x_min && posX <= x_max && posY >= y_min && posY <= y_max) {
                    areaCutEvents++;
                }
            }
        }
    }
    infile.close();
    
    cout << "========================================" << endl;
    cout << "File: " << filename << endl;
    cout << "Total lines read: " << totalLines << endl;
    cout << "Valid events (frag=1): " << validEvents << endl;
    cout << "Events with 196 < TOF < 210: " << tofCutEvents << endl;
    cout << "Events after Area cut: " << areaCutEvents << endl;
    cout << "========================================" << endl;
    
    // キャンバス作成
    TCanvas *c1 = new TCanvas("c1", "File 006 Check", 1200, 500);
    c1->Divide(2, 1);
    
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
    h2_pos->SetMarkerStyle(20);
    h2_pos->SetMarkerSize(0.5);
    h2_pos->Draw("COLZ");
    
    // エリアカットの境界を示す
    TBox *box = new TBox(x_min, y_min, x_max, y_max);
    box->SetFillStyle(0);
    box->SetLineColor(kRed);
    box->SetLineWidth(3);
    box->Draw();
    
    c1->SaveAs("check_file006.png");
    
    cout << "Output saved: check_file006.png" << endl;
}



