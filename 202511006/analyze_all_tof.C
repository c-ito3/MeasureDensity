void analyze_single_tof(const char* input_filename, const char* output_prefix, bool is_format1) {
    // ファイルを開く
    TString filepath = TString("測定データ/") + input_filename;
    ifstream infile(filepath.Data());
    
    if (!infile.is_open()) {
        cout << "Error: Cannot open " << filepath << endl;
        return;
    }
    
    // ヒストグラムを作成
    TH1F *h1_tof = new TH1F("h1_tof", "Time of Flight Distribution;TOF [ns];Counts",
                            200, 0, 500);
    TH2F *h2_tof_vs_pulseheight = new TH2F("h2_tof_vs_pulseheight", 
                                           "TOF vs Pulse Height;Pulse Height;TOF [ns]",
                                           200, -50000, 0, 200, 0, 500);
    
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
    
    if (is_format1) {
        // Format 1: No time frag Pulseheight ignore positionX positionY TOF
        while (infile >> no >> time >> frag >> pulseheight >> ignore >> posX >> posY >> tof) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベントのみプロット
            if (frag == 1 && tof > 0) {
                h1_tof->Fill(tof);
                h2_tof_vs_pulseheight->Fill(pulseheight, tof);
                valid_events++;
            }
        }
    } else {
        // Format 2: No time frag posX posY TOF...
        while (infile >> no >> time >> frag >> posX >> posY >> tof) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベントのみプロット
            if (frag == 1 && tof > 0) {
                h1_tof->Fill(tof);
                h2_tof_vs_pulseheight->Fill(0.0, tof);
                valid_events++;
            }
        }
    }
    
    infile.close();
    
    cout << "\n=== " << input_filename << " ===" << endl;
    cout << "Total events: " << count << endl;
    cout << "Valid TOF events (frag=1, TOF>0): " << valid_events << endl;
    
    if (valid_events > 0) {
        cout << "Mean TOF: " << h1_tof->GetMean() << " ns" << endl;
        cout << "RMS TOF: " << h1_tof->GetRMS() << " ns" << endl;
        cout << "Peak TOF: " << h1_tof->GetBinCenter(h1_tof->GetMaximumBin()) << " ns" << endl;
    }
    
    // キャンバスを作成して描画
    TCanvas *c1 = new TCanvas("c1", "TOF Analysis", 1400, 600);
    c1->Divide(2, 1);
    
    // TOFヒストグラム
    c1->cd(1);
    h1_tof->SetLineColor(kBlue);
    h1_tof->SetFillColor(kBlue);
    h1_tof->SetFillStyle(3001);
    h1_tof->Draw();
    
    // TOF vs Pulse Heightの2Dプロット
    c1->cd(2);
    gPad->SetRightMargin(0.15);
    gStyle->SetPalette(kRainbow);
    h2_tof_vs_pulseheight->SetStats(0);
    h2_tof_vs_pulseheight->Draw("COLZ");
    
    // 画像として保存
    TString png_name = TString("Analized/") + output_prefix + "_tof.png";
    c1->SaveAs(png_name);
    
    // ROOTファイルに保存
    TString root_name = TString("Analized/") + output_prefix + "_tof_analysis.root";
    TFile *outfile = new TFile(root_name, "RECREATE");
    h1_tof->Write();
    h2_tof_vs_pulseheight->Write();
    outfile->Close();
    
    delete c1;
    delete h1_tof;
    delete h2_tof_vs_pulseheight;
}

void analyze_all_tof() {
    cout << "========================================" << endl;
    cout << "Starting batch TOF analysis" << endl;
    cout << "========================================" << endl;
    
    // 14個のファイルを処理
    for (int i = 1; i <= 14; i++) {
        TString input_file = TString::Format("20251106_Beamprofile50_%02d_DAn.txt", i);
        TString output_prefix = TString::Format("profile%02d", i);
        
        // 01はformat1、02以降はformat2
        bool is_format1 = (i == 1);
        
        analyze_single_tof(input_file.Data(), output_prefix.Data(), is_format1);
        cout << endl;
    }
    
    cout << "========================================" << endl;
    cout << "Batch TOF analysis completed!" << endl;
    cout << "All results saved in Analized/ folder" << endl;
    cout << "========================================" << endl;
}
