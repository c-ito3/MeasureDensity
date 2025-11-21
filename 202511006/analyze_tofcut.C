void analyze_single_tofcut(const char* input_filename, const char* output_prefix, bool is_format1) {
    // ファイルを開く
    TString filepath = TString("測定データ/") + input_filename;
    ifstream infile(filepath.Data());
    
    if (!infile.is_open()) {
        cout << "Error: Cannot open " << filepath << endl;
        return;
    }
    
    // ヒストグラムを作成（ビン幅0.5mm）
    TH2F *h2_position = new TH2F("h2_position", "Beam Profile with TOF cut 196<TOF<210;Position X [mm];Position Y [mm]",
                                 200, -50, 50, 200, -50, 50);
    
    // ヘッダー行をスキップ（format1の場合のみヘッダーあり）
    if (is_format1) {
        string header;
        getline(infile, header);
    }
    
    // データを読み込む
    int no, frag;
    double time, pulseheight, ignore, posX, posY, tof;
    int count = 0;
    int valid_events = 0;
    int tof_cut_events = 0;
    
    if (is_format1) {
        // Format 1: No time frag Pulseheight ignore positionX positionY TOF
        while (infile >> no >> time >> frag >> pulseheight >> ignore >> posX >> posY >> tof) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベント
            if (frag == 1 && posX != 0 && posY != 0) {
                valid_events++;
                // TOFカット: 196 < TOF < 210
                if (tof > 196 && tof < 210) {
                    h2_position->Fill(posX, posY);
                    tof_cut_events++;
                }
            }
        }
    } else {
        // Format 2: No time frag posX posY TOF...
        while (infile >> no >> time >> frag >> posX >> posY >> tof) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベント
            if (frag == 1 && posX != 0 && posY != 0) {
                valid_events++;
                // TOFカット: 196 < TOF < 210
                if (tof > 196 && tof < 210) {
                    h2_position->Fill(posX, posY);
                    tof_cut_events++;
                }
            }
        }
    }
    
    infile.close();
    
    cout << "\n=== " << input_filename << " ===" << endl;
    cout << "Valid events (frag=1): " << valid_events << endl;
    cout << "Events passing TOF cut (196<TOF<210): " << tof_cut_events << endl;
    
    if (tof_cut_events > 0) {
        cout << "Mean X: " << h2_position->GetMean(1) << " mm" << endl;
        cout << "Mean Y: " << h2_position->GetMean(2) << " mm" << endl;
        cout << "RMS X: " << h2_position->GetRMS(1) << " mm" << endl;
        cout << "RMS Y: " << h2_position->GetRMS(2) << " mm" << endl;
    }
    
    // キャンバスを作成して描画
    TCanvas *c1 = new TCanvas("c1", "Beam Profile with TOF cut", 1000, 900);
    gPad->SetRightMargin(0.15);
    gStyle->SetPalette(kRainbow);
    gStyle->SetOptStat(0);
    h2_position->SetStats(0);
    
    h2_position->Draw("AXIS");
    
    for (int x = -50; x <= 50; x += 10) {
        TLine *vline = new TLine(x, -50, x, 50);
        vline->SetLineColor(kGray);
        vline->SetLineStyle(2);
        vline->SetLineWidth(1);
        vline->Draw();
    }
    
    for (int y = -50; y <= 50; y += 10) {
        TLine *hline = new TLine(-50, y, 50, y);
        hline->SetLineColor(kGray);
        hline->SetLineStyle(2);
        hline->SetLineWidth(1);
        hline->Draw();
    }
    
    h2_position->Draw("COLZ SAME");
    
    // 画像として保存
    TString png_name = TString("TOFcut/") + output_prefix + "_tofcut_2D.png";
    c1->SaveAs(png_name);
    
    // ROOTファイルに保存
    TString root_name = TString("TOFcut/") + output_prefix + "_tofcut_analysis.root";
    TFile *outfile = new TFile(root_name, "RECREATE");
    h2_position->Write();
    outfile->Close();
    
    delete c1;
    delete h2_position;
}

void analyze_tofcut() {
    cout << "========================================" << endl;
    cout << "Starting TOF cut analysis (196 < TOF < 210)" << endl;
    cout << "========================================" << endl;
    
    for (int i = 1; i <= 14; i++) {
        TString input_file = TString::Format("20251106_Beamprofile50_%02d_DAn.txt", i);
        TString output_prefix = TString::Format("profile%02d", i);
        bool is_format1 = (i == 1);
        
        analyze_single_tofcut(input_file.Data(), output_prefix.Data(), is_format1);
    }
    
    cout << "========================================" << endl;
    cout << "TOF cut analysis completed!" << endl;
    cout << "========================================" << endl;
}
