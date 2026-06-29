#include <iostream>
#include <curl/curl.h>
#include<fstream>
#include <set>
#include <pthread.h>
#include <string.h>
#include <regex>

using namespace std;

typedef struct MemoryStruct
{
    char *memory;
    size_t size;
} MemoryStruct;


static size_t 
WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                    void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr){
        cout << "not enough memory (realloc returned NULL)\n";
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

string codigo_fonte(const string site){
    CURL *curl;
    CURLcode result;

    MemoryStruct chunk;
    chunk.memory = (char *)malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl == NULL){
        cout << "HTTP request failed";
        exit(-1);
    }

    curl_easy_setopt(curl, CURLOPT_URL, &(site[0]));

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    result = curl_easy_perform(curl);

    if(result != CURLE_OK){
        cout << "Error: " << curl_easy_strerror(result) << endl;
        exit(-1);
    }

    return chunk.memory;
}

void varreSite(string strUrl, set<string>* linksUsados 
,set<string>* linksNaoUsados, set<string> pdfs){
    regex reg1 ("href=\"https://www.ifb.edu.br/([^ ]+)");

    sregex_iterator currentMatch(strUrl.begin(), strUrl.end(), reg1);
    sregex_iterator lastMatch;
    while(currentMatch != lastMatch){
        smatch match = *currentMatch;

        if((*linksUsados).count(match.str())){
            continue;
        }
        
        (*linksNaoUsados).insert(match.str());
        currentMatch++;
    }
}


int main(void){
    set<string> linksUsados; 
    set<string> linksNaoUsados;
    set<string> pdfs;

    linksNaoUsados.insert("https://www.ifb.edu.br/");
    varreSite(codigo_fonte("https://www.ifb.edu.br/"), &linksUsados, &linksNaoUsados, pdfs);

    set<string>::iterator itr;
    for(itr = linksNaoUsados.begin(); itr != linksNaoUsados.end(); itr++){
        cout << *itr << endl;
    }

    return 0;
}