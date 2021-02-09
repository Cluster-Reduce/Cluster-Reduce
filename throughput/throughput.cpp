// experiment1 fix compressed memory

#include "CMSketch.h"
#include "file_reader.h"

#include <bits/stdc++.h>

using namespace std;

vector<string> items;
unordered_map<string, int> freq, freq_ip;

const int N = 4e7;

int n;
string key[N];
int main() {
  fileReader("path to dataset", 4e7, items, freq, freq_ip);

  int M = 5;
  int k = 16;
  int key_len = 13;
  int sig_width = 4;
  int mem_in_byte = 10 << 20;
  int w = mem_in_byte >> 2;
  int w_after = w / k * (1 + sig_width * k / 32);

  CMSketch cm0, cm, cm1;
  cm0.init(w, 3);
  cm.init(w, 3);
  for (auto key : items)
    cm0.insert(key, key_len), cm.insert(key, key_len);
  cm1.copy(cm);
  // cm.compressIteration(k, sig_width, true);
  for (auto pr : freq)
    key[n++] = pr.first;

  auto beg = clock();
  int answer = 0;
  uint64_t sum = 0;
  for (int N = M; N--;) {
    for (int i = 0; i < n; i++) {
      int ans = cm0.query_original(key[i], key_len);
      sum ^= ans;
    }
  }
  double T1 = (clock() - beg) / (double)CLOCKS_PER_SEC / M;

  // cm1.compressMax(k, sig_width);
  // beg = clock();
  // for (int N = M; N--;) {
  //   for (ii = 0; i < n; i++) {
  //     int ans = cm1.query(key[i], key_len, false);
  //   }
  // }

  double T2 = (clock() - beg) / (double)CLOCKS_PER_SEC / M;

  cm.compressIteration(k, sig_width, true);
  beg = clock();
  for (int N = M; N--;) {
    for (int i = 0; i < n; i++) {
      int ans = cm.query(key[i], key_len, true);
      sum ^= ans;
    }
  }
  cout << sum << "\n";
  double T3 = (clock() - beg) / (double)CLOCKS_PER_SEC / M;
  cout << T1 << "," << T3 << "\n";
  cout << (n / T1) / 1000000 << " " << (n / T3) / 1000000 << "\n";

  return 0;
}
