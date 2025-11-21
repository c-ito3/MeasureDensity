void check_power_timeline() {
    gSystem->mkdir(".", kTRUE);
    gStyle->SetOptStat(0);
    
    // CSV読み込み
    std::ifstream infile("20251117/01.csv");
    std::vector<double> timeVec, powerVec;
    std::string line;
    for(int i=0; i<15; ++i) std::getline(infile, line);
    
    double startTime = 0;
    bool first = true;
    
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ',')) tokens.push_back(token);
        
        if (tokens.size() >= 4) {
            std::string dateStr = tokens[1];
            std::string timeStr = tokens[2];
            std::string powerStr = tokens[3];
            dateStr.erase(0, dateStr.find_first_not_of(" "));
            timeStr.erase(0, timeStr.find_first_not_of(" "));
            
            int mon, day, year, hh, mm, ss_int;
            sscanf(dateStr.c_str(), "%d/%d/%d", &mon, &day, &year);
            sscanf(timeStr.c_str(), "%d:%d:%d", &hh, &mm, &ss_int);
            TDatime dt(year, mon, day, hh, mm, ss_int);
            
            double t = dt.Convert();
            if (first) {
                startTime = t;
                first = false;
            }
            timeVec.push_back((t - startTime)/60.0); // 分単位
            powerVec.push_back(std::stod(powerStr));
        }
    }
    infile.close();
    
    TGraph *gr = new TGraph(timeVec.size(), &timeVec[0], &powerVec[0]);
    gr->SetTitle("Laser Power Timeline;Time [min];Power [W]");
    gr->SetLineWidth(1);
    gr->SetLineColor(kGray+1);
    
    TCanvas *c1 = new TCanvas("c1", "Power Timeline", 1200, 600);
    gr->Draw("AL");
    
    // スケジュール描画
    struct Measurement { int fileNum; const char* start; int dur; bool isOn; };
    std::vector<Measurement> schedule = {
        {1, "15:39:00", 10, false}, {2, "15:45:00", 3, true}, {3, "15:50:00", 10, false},
        {4, "16:02:00", 3, true}, {5, "16:14:00", 10, false}, {6, "16:20:00", 3, true},
        {7, "16:35:00", 10, false}, {8, "16:38:00", 3, true}, {9, "16:50:00", 10, false},
        {10, "16:55:00", 3, true}, {11, "17:34:00", 10, false}, {12, "17:40:00", 3, true},
        {13, "17:51:00", 10, false}, {14, "17:56:00", 3, true}, {15, "18:07:00", 10, false},
        {16, "18:14:00", 3, true}, {17, "18:24:00", 10, false}
    };
    
    // StartTime基準 (15:38:42 = 0)
    // 15:39:00 は 18秒後 = 0.3分後
    
    TDatime baseDt(2025, 11, 17, 15, 38, 42);
    double baseTime = baseDt.Convert();
    
    for (const auto& m : schedule) {
        int hh, mm, ss;
        sscanf(m.start, "%d:%d:%d", &hh, &mm, &ss);
        TDatime dt(2025, 11, 17, hh, mm, ss);
        double start = (dt.Convert() - baseTime) / 60.0;
        double end = start + m.dur;
        
        TBox *box = new TBox(start, 0, end, 2.0);
        box->SetFillStyle(3004);
        if (m.isOn) {
            box->SetFillColor(kRed); // ON
        } else {
            box->SetFillColor(kBlue); // OFF
        }
        box->Draw();
        
        TText *t = new TText(start, 2.1, Form("%d", m.fileNum));
        t->SetTextSize(0.02);
        t->Draw();
    }
    
    c1->SaveAs("check_power_timeline.png");
}

