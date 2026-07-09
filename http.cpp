#include <iostream>
#include <string>
#include <curl/curl.h>
#include "http.h"

using namespace std;

// poziva se svaki put kada libcurl dobije chunk podataka, stavlja podatke u string
size_t writeCallback(void *contents, size_t size, size_t nmemb, string *output)
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

string httpGet(const string &url)
{
    CURL *curl = curl_easy_init();
    string response = "";

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());             // postavi URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback); // gdje pisati odgovor
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);         // u nas string
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);           // prati redirecte

        CURLcode result = curl_easy_perform(curl); // izvrsi request

        if (result != CURLE_OK)
            cerr << "Greska: " << curl_easy_strerror(result) << endl;

        curl_easy_cleanup(curl);
    }

    return response;
}

/*int main() {
    string url;
    cout << "Unesite URL: ";
    cin >> url;

    string html = httpGet(url);

    cout << "\n--- Odgovor (prvih 500 karaktera) ---\n";
    cout << html.substr(0, 500) << endl;
}*/