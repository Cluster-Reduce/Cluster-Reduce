#include "headers.h"
#include <bits/stdc++.h>

using namespace std;

vector<string> items;
unordered_map<string, int> freq, freq_ip;
unordered_map<string, vector<string>> freq_ip_tuple;

const int WORKER = 8;
void test_distributed_cm(int mem_in_byte, int d, int k, int sig_width) {
    CMSketch cu[4][WORKER];
    BOBHash32 h;
    h.initialize(456);
    int mem_in_bit = mem_in_byte << 3;
    int block_size = 32 + k * sig_width;
    int w = mem_in_byte >> 2;
    int w0 = w / k + w * sig_width / 32;
    int w1 = w / k;

    memFile << (double)w0 / (1 << 18) << " " << (double)w1 / (1 << 18) << "\n";
    for (int t = 0; t < 4; t++)
        for (int i = 0; i < WORKER; i++) {
            cu[t][i].init(w, d);
        }
    vector<int> cnt(WORKER, 0);

    for (int t = 0; t < 4 - (sig_width > 1); t++) {
        int n = 0;
        for (auto pr : freq) {
            string key = pr.first;
            int val = pr.second;
            int n = h.run(key.c_str(), 13) & 7;
            cnt[n] += 1;
            while (val--)
                cu[t][n].insert(key.c_str());
        }
        for (int i = 0; i < WORKER; i++)
            cnt[i] = 0;
        for (int i = 0; i < WORKER; i++) {
            switch (t) {
            case 0:
                cu[t][i].compressSum(k, sig_width);
                break;
            case 1:
                cu[t][i].compressMax(k, sig_width);
                break;
            case 2:
                cu[t][i].compressIteration(k, sig_width);
                break;
            case 3:
                cu[t][i].compressDP(k, sig_width);
                break;
            default:
                break;
            }
        }
        double ARE = 0, AAE = 0;
        for (auto pr : freq) {
            string key = pr.first;
            int n = h.run(key.c_str(), 13) & 7;
            int real_val = pr.second, est_val = cu[t][n].query(key.c_str(), 13);
            int dist = abs(est_val - real_val);
            ARE += (double)dist / real_val;
            AAE += dist;
        }
        AAE /= freq.size();
        ARE /= freq.size();
        cout << ARE << "\n";
        areFile << ARE << " ";
        aaeFile << AAE << " ";
    }
    aaeFile << endl;
    areFile << endl;
    for (int t = 0; t < 4; t++)
        for (int i = 0; i < WORKER; i++)
            cu[t][i].clear();
    cout << endl;
}
int main() {
    fileReader("path to dataset", 3e7, items, freq, freq_ip);
    int mem_in_byte = 5 << 20;
    for (int sig_width = 1; sig_width <= 4; sig_width++) {
        cout << "sig_width = " << sig_width << "\n";
        for (int k = 2; k <= 10; k += 2)
            test_distributed_cm(mem_in_byte, 3, k, sig_width);
        break;
    }
    return 0;
}
