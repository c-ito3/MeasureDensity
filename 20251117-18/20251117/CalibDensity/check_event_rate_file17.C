void check_event_rate_file17() {
    int files[] = {1, 17};
    const int nFiles = 2;
    
    TCanvas *c1 = new TCanvas("c1", "Event Rate Check", 1200, 600);
    c1->Divide(2, 1);
    
    for (int i = 0; i < nFiles; i++) {
        int fileNum = files[i];
        TString filename = TString::Format("../Measure/20251117_Beamprofile50_%03d_DAn.txt", fileNum);
        
        TH1F *hRate = new TH1F(TString::Format("hRate_%d", fileNum), TString::Format("File %03d Event Rate;Time [s];Events / 4s", fileNum), 50, 0, 200);
        
        std::ifstream infile(filename.Data());
        std::string line;
        
        int no, frag;
        double time, col4, col5, posX, posY, tof;
        
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if (iss >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof) {
                if (frag == 1) {
                    // TOF Cut & Area Cut
                    if (tof > 196.0 && tof < 210.0) {
                         if (posX >= -10.0 && posX <= 10.0 && posY >= -10.0 && posY <= 10.0) {
                             hRate->Fill(time);
                         }
                    }
                }
            }
        }
        
        c1->cd(i+1);
        hRate->SetMinimum(0);
        hRate->Draw();
        
        // フィットして傾き（レート）が一定か見る（目視で十分だが）
        TF1 *f1 = new TF1("f1", "pol0", 0, 200);
        f1->SetLineColor(kRed);
        hRate->Fit(f1, "Q");
        
        double avg_rate = f1->GetParameter(0);
        cout << "File " << fileNum << " Avg Rate: " << avg_rate << " events/4s" << endl;
    }
    
    c1->SaveAs("check_event_rate_file17.png");
}

