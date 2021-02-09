// experiment: fix original memory
#include "headers.h"
#include <bits/stdc++.h>

using namespace std;

vector<string> items;
unordered_map<string, int> freq, freq_ip;

const int CM_NUM = 5;
void testCMCompress(int w, int d, int k, int sig_width = 1) {
    CMSketch cm[CM_NUM];
    bool NO_DP = sig_width > 1;

    cm[0].init(w, d);
    for (auto key : items) {
        cm[0].insert(key);
    }

    cm[1].copy(cm[0]);
    cm[2].copy(cm[0]);
    cm[3].copy(cm[0]);
    cm[4].copy(cm[0]);

    cm[1].compressSum(k, sig_width);
    cm[2].compressMax(k, sig_width);
    cm[3].compressIteration(k, sig_width);
    if (!NO_DP)
        cm[4].compressDP(k, sig_width);
    cout << "CM sketch: k = " << k << ", sig_width = " << sig_width << ":"
         << endl;
    for (int i = 1; i < CM_NUM - NO_DP; i++) {
        double ARE = 0;
        double AAE = 0;
        for (auto pr : freq) {
            int est_val = cm[i].query(pr.first);
            int real_val = pr.second;
            int dist = abs(est_val - real_val);
            AAE += (double)dist; // real_val;
            ARE += (double)dist / real_val;
        }
        AAE /= freq.size();
        ARE /= freq.size();
        areFile << ARE << " ";
        aaeFile << AAE << " ";
        cout << "cm[" << i << "](" << cm[i].description << ") ARE = " << ARE
             << endl;
    }
    for (int i = 1; i < CM_NUM - NO_DP; i++) {
        double mem = w << 2;
        if (i >= 3) {
            mem = mem / k + w * sig_width / 8;
        } else
            mem /= k;
        mem /= (1 << 20);
        memFile << mem << ",";
    }
    memFile << "\n";
    areFile << "\n";
    aaeFile << "\n";
    cout << endl;
    for (int i = 1; i < CM_NUM - NO_DP; ++i)
        cm[i].clear();
}
void testCompress(int mem_in_byte) {
    int mem_in_bit = mem_in_byte << 3;
    cout << "mem = " << (double)mem_in_byte / (1 << 20) << "MB" << endl;
    for (int sig_width = 1; sig_width <= 4; sig_width++)
        for (int k = 2; k <= 10; k += 2) {
            int block_size = 32 + k * sig_width;
            int w = mem_in_byte >> 2;
            cerr << k << " " << sig_width << " : " << w << " ("
                 << (double)w / (1 << 18) << "M)" << endl;
            memFile << (double)w / (1 << 18) << "\n";
            testCMCompress(w, 3, k, sig_width);
        }
}
int main() {
    fileReader("path to dataset", 4e7, items, freq, freq_ip);
    // testNoCompress(3145728 << 2);
    testCompress(36 << 20);
    return 0;
}
