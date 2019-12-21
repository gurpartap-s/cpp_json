#include <iostream>
#include <fstream>
#include "Connection.h"
#include "json11.hpp"

using namespace std;
using namespace json11;

void process_json(){
    cout << "Processing json\n";
    string buf;
    string line;
    ofstream file;
    ifstream rx_file ("downloaded.json");
    while (std::getline(rx_file, line)) {
        buf += line + "\n";
    }
    string err;
    auto json = Json::parse(buf, err);
    if (!err.empty()) {
        cout << "Failed: " << err.c_str() << endl;
    } else {
        //cout << "dump json to file" << endl;
        file.open("processed.json", ios::out | ios::trunc);
        file << json.dump().c_str();
        file.close();
    }
}
int main(int argc, char** argv){
    string url_str;

    if(argc < 2){
        cout << "Usage:\n";
        cout << "   <program_name> <URL>\n";
        cout << "   URL - the resource to be accessed\n";
        return 0;
    }else{
        url_str.assign(argv[1]);
    }
    Connection conn;
    conn.SetUrl(url_str);
    conn.SetHeader();
    conn.Get();
    process_json();
    conn.Post();

    return 0;
}
