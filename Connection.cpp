#include "Connection.h"
#include <iostream>
#include <fstream>
#include <cstring>
string ret_data;
MemoryStruct * Connection::DownloadedData;
size_t Connection::writeFunction(void *data, size_t size, size_t nmemb, void *buffer_in){
    //cout << "in writeFunction" << "\n";
    ofstream rx_file;
    MemoryStruct *mem;
    size_t realsize = size * nmemb;
    if (buffer_in != NULL)  {
        mem = (MemoryStruct *)buffer_in;
        mem->memory.append((char*)data, realsize);
        //(Connection::DownloadedData)->memory.append((char*)data, realsize);
        Connection::DownloadedData = mem;
        Connection::DownloadedData->size += realsize;
        //cout << realsize << "\n";
        //cout << Connection::DownloadedData->size << "\n";
        rx_file.open("downloaded.json", ios::out | ios::trunc);
        rx_file << (Connection::DownloadedData)->memory;
        rx_file.close();
        return realsize;
    }
    return 0;
}

Connection::Connection()
{
    //cout << "Connection constructor\n";
    headers = nullptr;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
}
void Connection::SetUrl(string url){
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}
void Connection::SetHeader(){
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_slist_free_all(headers);
    headers = nullptr;
}
Connection::~Connection()
{
    //cout << "Connection destructor\n";
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
CURLcode Connection::Get() {
    cout << "Connection::Get \n";
    string response_string;
    MemoryStruct response;
    response.memory = response_string;
    response.size = 0;
    //string url ("https://raw.githubusercontent.com/gurpartap-s/test_json/master/sample.json");
    //string url ("https://api.github.com/users/hadley/orgs");
    //SetUrl(url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET,1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    if (CURLE_OK == res)
    {
        char *ct;
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
        if((CURLE_OK == res) && ct)
            return res;
    }
    return res;
}
static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    //cout << "in read_callback \n";
    WriteBuf *wt = (WriteBuf *)userp;
    size_t buffer_size = size*nmemb;
    if(wt->sizeleft) {
        /* copy as much as possible from the source to the destination */
        size_t copy_this_much = wt->sizeleft;
        if(copy_this_much > buffer_size)
            copy_this_much = buffer_size;
        memcpy(dest, wt->readptr, copy_this_much);
        //cout << wt->readptr << endl;
        wt->readptr += copy_this_much;
        wt->sizeleft -= copy_this_much;
        //cout << "out read_callback " << copy_this_much <<endl;
        return copy_this_much; /* we copied this many bytes */
    }
  return 0; /* no more data left to deliver */
}
WriteBuf wt;
CURLcode Connection::Post() {
    cout << "Connection::Post \n";
    //string url ("http://httpbin.org/post");
    //string url ("https://api.github.com/users/hadley/orgs");
    string buf;
    string line;
    try{
        ifstream rx_file ("downloaded.json");
        //WriteBuf wt;
        while (std::getline(rx_file, line)) {
            buf += line + "\n";
        }
        wt.readptr = buf.c_str();
        wt.sizeleft = buf.size();
        //SetUrl(url);
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &wt);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        //cout << "(long)wt.sizeleft " << (long)wt.sizeleft << endl;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
    }
    catch(...){
        //curl_easy_cleanup(curl);
        //curl_global_cleanup();
    }
    return res;
}
