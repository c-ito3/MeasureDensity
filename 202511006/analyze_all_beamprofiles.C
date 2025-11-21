void analyze_single_file(const char* input_filename, const char* output_prefix, bool is_format1) {
    // ファイルを開く
    TString filepath = TString("測定データ/") + input_filename;
    ifstream infile(filepath.Data());
    
    if (!infile.is_open()) {
        cout << "Error: Cannot open " << filepath << endl;
        return;
    }
    
    // ヒストグラムを作成（ビン幅0.5mm）
    TH2F *h2_position = new TH2F("h2_position", "Beam Profile (X-Y Position);Position X [mm];Position Y [mm]",
                                 200, -50, 50, 200, -50, 50);
    TH1F *h1_pulseheight = new TH1F("h1_pulseheight", "Pulse Height Distribution;Pulse Height;Counts",
                                     200, -50000, 0);
    
    // ヘッダー行をスキップ（format1の場合のみヘッダーあり）
    if (is_format1) {
        string header;
        getline(infile, header);
    }
    
    // データを読み込む
    int no, frag;
    double time, pulseheight, ignore, posX, posY;
    int count = 0;
    int valid_events = 0;
    
    if (is_format1) {
        // Format 1: No time frag Pulseheight ignore positionX positionY
        while (infile >> no >> time >> frag >> pulseheight >> ignore >> posX >> posY) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベントのみプロット
            if (frag == 1 && posX != 0 && posY != 0) {
                h2_position->Fill(posX, posY);
                h1_pulseheight->Fill(pulseheight);
                valid_events++;
            }
        }
    } else {
        // Format 2: No time frag posX posY TOF...
        while (infile >> no >> time >> frag >> posX >> posY) {
            count++;
            
            // 残りの列を読み飛ばす
            string dummy_line;
            getline(infile, dummy_line);
            
            // frag=1の有効なイベントのみプロット
            if (frag == 1 && posX != 0 && posY != 0) {
                h2_position->Fill(posX, posY);
                h1_pulseheight->Fill(0);
                valid_events++;
            }
        }
    }
    
    infile.close();
    
    cout << "\n=== " << input_filename << " ===" << endl;
    cout << "Total events: " << count << endl;
    cout << "Valid events (frag=1): " << valid_events << endl;
    
    // キャンバスを作成して描画
    TCanvas *c1 = new TCanvas("c1", "Beam Profile Analysis", 1400, 600);
    c1->Divide(2, 1);
    
    // 2Dヒストグラム
    c1->cd(1);
    gPad->SetRightMargin(0.15);
    gStyle->SetPalette(kRainbow);
    gStyle->SetOptStat(0);
    h2_position->SetStats(0);
    
    // まずヒストグラムの枠だけを描画
    h2_position->Draw("AXIS");
    
    // 10mm単位のグリッド線を追加（黒色、データの背面）
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
    
    // 最後にヒストグラムのデータを重ねて描画
    h2_position->Draw("COLZ SAME");
    
    // パルスハイト分布
    c1->cd(2);
    h1_pulseheight->SetLineColor(kBlue);
    h1_pulseheight->SetFillColor(kBlue);
    h1_pulseheight->SetFillStyle(3001);
    h1_pulseheight->Draw();
    
    // 画像として保存（Analizedフォルダへ）
    TString png_name = TString("Analized/") + output_prefix + "_beamprofile_2D.png";
    c1->SaveAs(png_name);
    
    // ROOTファイルに保存
    TString root_name = TString("Analized/") + output_prefix + "_analysis.root";
    TFile *outfile = new TFile(root_name, "RECREATE");
    h2_position->Write();
    h1_pulseheight->Write();
    outfile->Close();
    
    cout << "Mean X: " << h2_position->GetMean(1) << " mm" << endl;
    cout << "Mean Y: " << h2_position->GetMean(2) << " mm" << endl;
    cout << "RMS X: " << h2_position->GetRMS(1) << " mm" << endl;
    cout << "RMS Y: " << h2_position->GetRMS(2) << " mm" << endl;
    
    delete c1;
    delete h2_position;
    delete h1_pulseheight;
}

void analyze_all_beamprofiles() {
    cout << "========================================" << endl;
    cout << "Starting batch analysis of all beam profile data" << endl;
    cout << "========================================" << endl;
    
    // 14個のファイルを処理
    for (int i = 1; i <= 14; i++) {
        TString input_file = TString::Format("20251106_Beamprofile50_%02d_DAn.txt", i);
        TString output_prefix = TString::Format("profile%02d", i);
        
        // 01はformat1、02以降はformat2
        bool is_format1 = (i == 1);
        
        analyze_single_file(input_file.Data(), output_prefix.Data(), is_format1);
        cout << endl;
    }
    
    cout << "========================================" << endl;
    cout << "Batch analysis completed!" << endl;
    cout << "All results saved in Analized/ folder" << endl;
    cout << "========================================" << endl;
}
