void analyze_single_laser(int laser_num) {
    TString filename = TString::Format("Laser/DeviceData_#%03d.csv", laser_num);
    ifstream infile(filename.Data());
    
    if (!infile.is_open()) {
        cout << "Skip: " << filename << " (not found)" << endl;
        return;
    }
    
    // ヘッダーを読む（18行）
    string line;
    int width = 0, height = 0;
    double pixelSize = 0;
    
    for (int i = 0; i < 18; i++) {
        getline(infile, line);
        
        if (line.find("Sensor Resolution") != string::npos) {
            size_t pos1 = line.find("Width:");
            size_t pos2 = line.find("Height:");
            if (pos1 != string::npos && pos2 != string::npos) {
                sscanf(line.c_str() + pos1, "Width: %d", &width);
                sscanf(line.c_str() + pos2, "Height: %d", &height);
            }
        }
        
        if (line.find("Pixel Size") != string::npos) {
            size_t pos = line.find("Horizontal:");
            if (pos != string::npos) {
                sscanf(line.c_str() + pos, "Horizontal: %lf", &pixelSize);
            }
        }
    }
    
    double physWidth = width * pixelSize / 1000.0;
    double physHeight = height * pixelSize / 1000.0;
    
    // 2Dヒストグラムを作成
    TH2F *h2_beam = new TH2F("h2_beam", "Beam Intensity Distribution;X [mm];Y [mm]",
                             width, -physWidth/2, physWidth/2,
                             height, -physHeight/2, physHeight/2);
    
    // データを読み込む
    int row = 0;
    while (getline(infile, line) && row < height) {
        stringstream ss(line);
        string value_str;
        int col = 0;
        
        while (getline(ss, value_str, ';') && col < width) {
            try {
                int intensity = stoi(value_str);
                double x = -physWidth/2 + (col + 0.5) * pixelSize / 1000.0;
                double y = physHeight/2 - (row + 0.5) * pixelSize / 1000.0;
                h2_beam->Fill(x, y, intensity);
                col++;
            } catch (...) {
                break;
            }
        }
        row++;
    }
    infile.close();
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Laser Beam Profile", 1000, 900);
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.15);
    gPad->SetBottomMargin(0.12);
    
    gStyle->SetPalette(kRainbow);
    
    h2_beam->GetXaxis()->SetTitleSize(0.045);
    h2_beam->GetYaxis()->SetTitleSize(0.045);
    h2_beam->GetZaxis()->SetTitleSize(0.045);
    h2_beam->GetXaxis()->SetLabelSize(0.04);
    h2_beam->GetYaxis()->SetLabelSize(0.04);
    h2_beam->GetZaxis()->SetLabelSize(0.04);
    h2_beam->Draw("COLZ");
    
    // 保存
    TString outputFile = TString::Format("LaserAnalized/DeviceData_#%03d_2D.png", laser_num);
    c1->SaveAs(outputFile);
    
    cout << "Processed: " << filename << " -> " << outputFile << endl;
    
    delete h2_beam;
    delete c1;
}

void analyze_all_laser_data() {
    gSystem->mkdir("LaserAnalized", kTRUE);
    
    // レーザーデータは1-14
    for (int i = 1; i <= 14; i++) {
        analyze_single_laser(i);
    }
    
    cout << "\nAll laser data analyzed!" << endl;
}





