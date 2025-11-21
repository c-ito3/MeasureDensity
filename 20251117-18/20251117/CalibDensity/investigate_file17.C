void investigate_file17() {
    int files[] = {1, 13, 15, 17};
    const int nFiles = 4;
    
    TH1F *hTOF[nFiles];
    
    cout << "File | Raw(frag=1) | TOF Cut | Area Cut | Reduction Rate" << endl;
    cout << "-----|-------------|---------|----------|---------------" << endl;
    
    TCanvas *c1 = new TCanvas("c1", "TOF Distribution Comparison", 1200, 800);
    c1->Divide(2, 2);
    
    for (int i = 0; i < nFiles; i++) {
        int fileNum = files[i];
        TString filename = TString::Format("../Measure/20251117_Beamprofile50_%03d_DAn.txt", fileNum);
        
        std::ifstream infile(filename.Data());
        
        int raw_valid = 0;
        int tof_cut = 0;
        int area_cut = 0;
        
        hTOF[i] = new TH1F(TString::Format("hTOF_%d", fileNum), TString::Format("File %03d TOF;TOF [ns];Counts", fileNum), 100, 150, 250);
        
        int no, frag;
        double time, col4, col5, posX, posY, tof;
        double trash; // その他の列
        
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            // 17列フォーマット読み込み
            if (iss >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof) {
                if (frag == 1) {
                    raw_valid++;
                    hTOF[i]->Fill(tof);
                    
                    if (tof > 196.0 && tof < 210.0) {
                        tof_cut++;
                        if (posX >= -10.0 && posX <= 10.0 && posY >= -10.0 && posY <= 10.0) {
                            area_cut++;
                        }
                    }
                }
            }
        }
        
        double reduction = 100.0 * (1.0 - (double)area_cut / raw_valid);
        printf("%4d | %11d | %7d | %8d | %12.1f%%\n", fileNum, raw_valid, tof_cut, area_cut, reduction);
        
        c1->cd(i+1);
        hTOF[i]->SetLineColor(kBlue);
        if (fileNum == 17) hTOF[i]->SetLineColor(kRed);
        hTOF[i]->Draw();
        
        // TOFカット範囲を表示
        TLine *l1 = new TLine(196, 0, 196, hTOF[i]->GetMaximum());
        TLine *l2 = new TLine(210, 0, 210, hTOF[i]->GetMaximum());
        l1->SetLineColor(kGreen+2); l1->SetLineStyle(2);
        l2->SetLineColor(kGreen+2); l2->SetLineStyle(2);
        l1->Draw(); l2->Draw();
    }
    
    c1->SaveAs("investigate_file17_tof.png");
}

