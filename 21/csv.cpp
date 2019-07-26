#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "csv.hpp"

using namespace std;

Csv::Csv(string csv_file)
{
    this->csv_file = csv_file;
}

bool Csv::getCsv(vector<vector<string>>& data, const char delim)
{
    ifstream ifs(csv_file);
    if (!ifs.is_open()) return false; 

    // ファイルREAD
    string buf;             
    while (getline(ifs, buf)) {
        vector<string> rec;  
        istringstream iss(buf);
        string field;        

        while (getline(iss, field, delim)) rec.push_back(field);

        if (rec.size() != 0) data.push_back(rec);
    }

    return true;  
}
