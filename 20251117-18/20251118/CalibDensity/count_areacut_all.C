void count_areacut_all() {
    // TOFカット範囲
    const double tof_min = 196.0;
    const double tof_max = 210.0;
    
    // エリアカット範囲 [mm]
    const double x_min = -10.0;
    const double x_max = 10.0;
    const double y_min = -10.0;
    const double y_max = 10.0;
    
    // ファイル番号（001から017まで）
    std::vector<int> file_nums;
    for (int i = 1; i <= 17; i++) {
        file_nums.push_back(i);
    }
    
    // 結果を格納
    std::map<int, int> validCounts;
    std::map<int, int> tofCutCounts;
    std::map<int, int> areaCutCounts;
    
    cout << "Processing files..." << endl;
    cout << "========================================" << endl;
    
    for (size_t idx = 0; idx < file_nums.size(); idx++) {
        int fileNum = file_nums[idx];
        TString filename = TString::Format("../Measure/20251118_Beamprofile50_%03d_DAn.txt", fileNum);
        
        std::ifstream infile(filename.Data());
        if (!infile.is_open()) {
            std::cerr << "Warning: Cannot open " << filename << ", skipping." << endl;
            continue;
        }
        
        int no, frag;
        double time, col4, col5, posX, posY, tof;
        double col9, col10, col11, col12, col13, col14, col15, col16, col17;
        
        int validEvents = 0;
        int tofCutEvents = 0;
        int areaCutEvents = 0;
        
        // データフォーマット: 17列 (No time frag col4 col5 posX posY TOF + 9列)
        while (infile >> no >> time >> frag >> col4 >> col5 >> posX >> posY >> tof
                      >> col9 >> col10 >> col11 >> col12 >> col13 >> col14 >> col15 >> col16 >> col17) {
            if (frag == 1) {
                validEvents++;
                if (tof > tof_min && tof < tof_max) {
                    tofCutEvents++;
                    if (posX >= x_min && posX <= x_max && posY >= y_min && posY <= y_max) {
                        areaCutEvents++;
                    }
                }
            }
        }
        infile.close();
        
        validCounts[fileNum] = validEvents;
        tofCutCounts[fileNum] = tofCutEvents;
        areaCutCounts[fileNum] = areaCutEvents;
        
        printf("File %03d: Valid=%6d, TOFcut=%6d, Areacut=%6d\n",
               fileNum, validEvents, tofCutEvents, areaCutEvents);
    }
    
    cout << "========================================" << endl;
    
    // 結果をファイルに保存
    std::ofstream outfile("areacut_counts.txt");
    outfile << "FileNum ValidEvents TOFcutEvents AreacutEvents" << endl;
    
    for (size_t idx = 0; idx < file_nums.size(); idx++) {
        int fileNum = file_nums[idx];
        if (validCounts.find(fileNum) != validCounts.end()) {
            outfile << fileNum << " " 
                    << validCounts[fileNum] << " " 
                    << tofCutCounts[fileNum] << " " 
                    << areaCutCounts[fileNum] << endl;
        }
    }
    outfile.close();
    
    cout << "Results saved to areacut_counts.txt" << endl;
}

