void check_xy_file17() {
    int files[] = {1, 17};
    const int nFiles = 2;
    
    TCanvas *c1 = new TCanvas("c1", "XY Distribution Comparison", 1200, 600);
    c1->Divide(2, 1);
    
    for (int i = 0; i < nFiles; i++) {
        int fileNum = files[i];
        TString filename = TString::Format("../Measure/20251117_Beamprofile50_%03d_DAn.txt", fileNum);
        
        TH2F *hXY = new TH2F(TString::Format("hXY_%d", fileNum), TString::Format("File %03d Position;X [mm];Y [mm]", fileNum), 100, -20, 20, 100, -20, 20);
        
        std::ifstream infile(filename.Data());
        std::string line;
        
        int no, frag;
        double time, col4, col5, posX, posY, tof;
        
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if (iss >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof) {
                if (frag == 1) {
                    // TOF Cut only
                    if (tof > 196.0 && tof < 210.0) {
                        hXY->Fill(posX, posY);
                    }
                }
            }
        }
        
        c1->cd(i+1);
        hXY->Draw("COLZ");
        
        // Area Cut Box
        TBox *box = new TBox(-10, -10, 10, 10);
        box->SetFillStyle(0);
        box->SetLineColor(kRed);
        box->SetLineWidth(2);
        box->Draw();
    }
    
    c1->SaveAs("check_xy_file17.png");
}

