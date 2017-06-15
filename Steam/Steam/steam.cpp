#include <curl\curl.h>
#include <errno.h>
#include <curl\multi.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <set>
#include <algorithm>
#include <regex>
#include <io.h>
#include <map>
#include <thread>
#include <mutex>


using namespace std;
set<string> profileset;

#define MAX 10
struct MemoryStruct {
  char *memory;
  size_t size;
};
 
int id = 0;
int itr = 1;
int limit = 0;
vector<string> urls;
mutex mtx;
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

bool readprofile(string profile);


bool readfriend(string profile);

void todo(int arg){
	for(int i=id+arg; i<urls.size(); i+=2){
		readprofile(urls.at(i));
		cout << i << endl;
		if(limit > 1000){
			i-=1000;
			id=i;
			break;
		}
	}
}
int main(){

	string dir = "userlist.txt";
	int dup = access(dir.c_str(), 0);
	typedef map<string, int> line_record;
	line_record lines;
	string temp;
	int idx = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	if(dup != 0){
		readfriend("76561197995580306");
		//readfriend("76561198001179367");
		//readfriend("76561198005909280");
		
		ifstream tempF1("temp.txt");
		ofstream oFile1("userlist.txt");
		while(getline(tempF1, temp)){
			line_record::iterator existing = lines.find(temp);
			if(existing != lines.end()){
				existing->second = (-1);
			}
			else{
				lines.insert(make_pair(temp, idx));
				++idx;
				getline(tempF1,temp);
				oFile1<<temp<<endl;
			
			}
			//cout << idx << endl;
		}
		tempF1.close();
		oFile1.close();
		remove("temp.txt");

		for(int i=0; i<itr; i++){
			ifstream iFile1("userlist.txt");
			while(getline(iFile1, temp)){
				readfriend(temp);
			}
			iFile1.close();
		
			ifstream tempF2("temp.txt");
			ofstream oFile2("userlist.txt");
			idx = 0;
			while(getline(tempF2, temp)){
				line_record::iterator existing = lines.find(temp);
				if(existing != lines.end()){
					existing->second = (-1);
				}
				else{
					lines.insert(make_pair(temp, idx));
					++idx;
					getline(tempF2,temp);
					oFile2<<temp<<endl;
			
				}
			}
			tempF2.close();
			oFile2.close();
			remove("temp.txt");

		}

	}
	ifstream iFile("userlist.txt");
	
	int error;
	if(iFile.is_open()){
		while(getline(iFile, temp)){
			urls.push_back(temp);
		}
	}
	while(id<urls.size()){
		thread t1(&todo,0);
		thread t2(&todo,1);
		//thread t3(&todo,2);
		//thread t4(&todo,3);
		t1.join();
		t2.join();
		limit = 0;
		cout << "sleep" << endl;
		Sleep(3600000);
	}
	//t3.join();
	//t4.join();


	curl_global_cleanup();
	return 0;
}

bool readprofile(string profile){
	
	string dir = "profile\\" + profile;
	int dup = access(dir.c_str(), 0);
	if(dup == 0){
		return false;
	}
	
	string steam = "http://api.steampowered.com/";
	string steamUser = "IPlayerService/";
	string getGame = "GetOwnedGames/v0001/";
	string key = "?key=B20DC50CD773CD98C8682EE6C50820C6";
	string steamID = "&steamid="+profile;
	string format = "&format=json";
	string url;
	

	CURL *curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;
	smatch m;
	string gamejson;


	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */ 
	
	url = steam + steamUser + getGame + key + steamID + format;
		


	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
	res = curl_easy_perform(curl_handle);
		
 
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		mtx.lock();
		ofstream err("error.txt", ios::app);
		err << profile << endl;
		err.close();
		mtx.unlock();

	}
	else {
		if(chunk.size > 22){
			ofstream oFile(dir);
			oFile << chunk.memory;
			oFile.close();
		}
			else{
			mtx.lock();
			cout << "blank!" << endl;
			limit++;
			mtx.unlock();
			
		}
	}



 
	curl_easy_cleanup(curl_handle);
 
	free(chunk.memory);
	url.clear();

	return true;
}


bool readfriend(string profile){

	CURL *curl_handle;
	CURLcode res;	

	struct MemoryStruct chunk;
	smatch m;
	regex e("\"steamid\": \"([0-9]*)");
	string gamejson;
	string url;
	string friendtab = "/friends";
	string temp;
	
	string steam = "http://api.steampowered.com/";
	string steamUser = "ISteamUser/";
	string getFriend = "GetFriendList/v0001/";
	string key = "?key=B20DC50CD773CD98C8682EE6C50820C6";
	string steamID = "&steamid="+profile;
	string relationship = "&relationship=friend";
	string str;

	//string gameprofile = "http://steamcommunity.com/profiles/76561198098036179/games/?tab=all";

	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */ 

	//url.clear();
	url = steam + steamUser + getFriend + key + steamID + relationship;
		


	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
	res = curl_easy_perform(curl_handle);
		
 
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	}
	else {
		//printf("%lu bytes retrieved\n", (long)chunk.size);
		
		ofstream oFile("temp.txt", ios::app);
		str = chunk.memory;
		for(auto it = sregex_iterator(str.begin(), str.end(), e);
			it != sregex_iterator();
			++it){
				oFile << it->str(1) << "\n";
		}
		oFile.close();
	}



 
	curl_easy_cleanup(curl_handle);
 
	free(chunk.memory);

	str.clear();
	url.clear();

	return true;

}