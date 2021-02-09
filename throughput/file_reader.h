#ifndef __FILE_H__
#define __FILE_H__
#include <bits/stdc++.h>
#include <unordered_map>

using namespace std;

ofstream areFile("are1.out", ios::out);
ofstream aaeFile("aae1.out", ios::out);
ofstream memFile("mem1.out", ios::out);

void fileReader(const char *filename, int MAX_ITEM, vector<string> &items,
                unordered_map<string, int> &freq,
                unordered_map<string, int> &freq_ip) {
  ifstream inFile(filename, ios::binary);
  ios::sync_with_stdio(false);

  char key[13];
  double timestamp;
  map<int, int> ip_count;
  for (int i = 0; i < MAX_ITEM; ++i) {
    inFile.read(key, 13);
    inFile.read((char *)&timestamp, 8);
    if (inFile.gcount() < 8)
      break;

    string str = string(key, 13);
    items.push_back(str);
    freq[str]++;
    // cout << str.substr(0, 4).size() << endl;
    string src_ip = str.substr(0, 4);
    freq_ip[src_ip]++;
  }
  cout << freq.size() << "," << freq_ip.size() << endl;
  for (auto pa : ip_count)
    cout << pa.first << "," << pa.second << endl;
  cout << freq.size() << "flows, " << items.size() << " items read." << endl;
  inFile.close();
}

void zipfReader(const char *filename, int MAX_ITEM, vector<string> &items,
                unordered_map<string, int> &freq,
                unordered_map<string, int> &freq_ip) {
  ifstream inFile(filename, ios::binary);
  ios::sync_with_stdio(false);

  char key[13];
  map<int, int> ip_count;
  for (int i = 0; i < MAX_ITEM; ++i) {
    inFile.read(key, 4);
    if (inFile.gcount() < 4)
      break;

    string str = string(key, 4);
    items.push_back(str);
    freq[str]++;
  }
  cout << freq.size() << "flows, " << items.size() << " items read." << endl;
  inFile.close();
}

void webReader(const char *filename, int MAX_ITEM, vector<string> &items,
               unordered_map<string, int> &freq,
               unordered_map<string, int> &freq_ip) {
  ifstream inFile(filename, ios::binary);
  ios::sync_with_stdio(false);

  char key[13];
  map<int, int> ip_count;
  for (int i = 0; i < MAX_ITEM; ++i) {
    inFile.read(key, 8);
    if (inFile.gcount() < 8)
      break;

    string str = string(key, 8);
    items.push_back(str);
    freq[str]++;
  }
  cout << freq.size() << "flows, " << items.size() << " items read." << endl;
  inFile.close();
}

#endif
