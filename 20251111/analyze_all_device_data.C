void analyze_single_device(int device_num) {
    TString filename = TString::Format("Laser/DeviceData_#%03d.csv", device_num);
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
                double x = (col - width/2.0) * pixelSize / 1000.0;
                double y = (height/2.0 - row) * pixelSize / 1000.0;
                h2_beam->Fill(x, y, intensity);
                col++;
            } catch (...) {}
        }
        row++;
    }
    
    infile.close();
    
    cout << "Device #" << device_num << ": " << row << " rows loaded" << endl;
    
    // キャンバスを作成
    TCanvas *c1 = new TCanvas("c1", "Beam Intensity", 1000, 900);
    gPad->SetRightMargin(0.15);
    gStyle->SetPalette(kRainbow);
    gStyle->SetOptStat(0);
    h2_beam->SetStats(0);
    h2_beam->Draw("AXIS");
    
    for (double x = -physWidth/2; x <= physWidth/2; x += 1.0) {
        TLine *vline = new TLine(x, -physHeight/2, x, physHeight/2);
        vline->SetLineColor(kGray);
        vline->SetLineStyle(2);
        vline->SetLineWidth(1);
        vline->Draw();
    }
    
    for (double y = -physHeight/2; y <= physHeight/2; y += 1.0) {
        TLine *hline = new TLine(-physWidth/2, y, physWidth/2, y);
        hline->SetLineColor(kGray);
        hline->SetLineStyle(2);
        hline->SetLineWidth(1);
        hline->Draw();
    }
    
    h2_beam->Draw("COLZ SAME");
    
    // 画像として保存
    TString png_name = TString::Format("LaserAnalized/DeviceData_#%03d_2D.png", device_num);
    c1->SaveAs(png_name);
    
    // ROOTファイルに保存
    TString root_name = TString::Format("LaserAnalized/DeviceData_#%03d_analysis.root", device_num);
    TFile *outfile = new TFile(root_name, "RECREATE");
    h2_beam->Write();
    outfile->Close();
    
    delete c1;
    delete h2_beam;
}

void analyze_all_device_data() {
    cout << "========================================" << endl;
    cout << "Starting Device Data analysis" << endl;
    cout << "========================================" << endl;
    
    // DeviceData #003から#014まで処理
    for (int i = 3; i <= 14; i++) {
        analyze_single_device(i);
    }
    
    cout << "========================================" << endl;
    cout << "All device data analysis completed!" << endl;
    cout << "========================================" << endl;
}

