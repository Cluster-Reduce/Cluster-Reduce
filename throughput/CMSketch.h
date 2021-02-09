#ifndef __CM_SKETCH_H__
#define __CM_SKETCH_H__

#include "BOBHash32.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

template <typename T> inline bool chkmin(T &lhs, const T &rhs) {
    return lhs > rhs ? lhs = rhs, true : false;
}

const int max_compress_level = 5;

class CMSketch {
  public:
    int w, d, *k, s, rng;
    int compress_level;
    int *counters;
    int8_t **signals;
    BOBHash32 *hash, *sign_hash;

  public:
    string description;

    void init(int in_w, int in_d, int in_s = 750) {
        w = in_w, d = in_d, s = in_s, compress_level = 0, rng = 0;
        k = new int[max_compress_level];
        counters = new int[w];
        signals = new int8_t *[max_compress_level];
        memset(counters, 0, w * sizeof(int));
        memset(k, 0, max_compress_level * sizeof(int));
        k[0] = 1;
        signals[0] = new int8_t[w];
        memset(signals[0], 0, w * sizeof(int8_t));
        hash = new BOBHash32[d];
        for (int i = 0; i < d; ++i)
            hash[i].initialize(s + i);
        description = "non-compress";
    }

    int size() {
        int u = w;
        for (int i = 0; i < compress_level; i++)
            u /= k[i];
        return u;
    }
    void clear() {
        delete[] counters;
        delete[] signals;
        delete[] hash;
    }

    void copy(const CMSketch &src) {
        w = src.w, d = src.d, s = src.s, compress_level = src.compress_level,
        rng = src.rng;
        counters = new int[w];
        signals = new int8_t *[max_compress_level];
        k = new int[max_compress_level];
        memcpy(k, src.k, max_compress_level * sizeof(int));
        memcpy(counters, src.counters, w * sizeof(int));
        for (int i = 0, cur_w = w; i < max(compress_level, 1); i++) {
            cur_w /= k[i];
            signals[i] = new int8_t[cur_w];
            memcpy(signals[i], src.signals[i], cur_w * sizeof(int8_t));
        }
        hash = new BOBHash32[d];
        for (int i = 0; i < d; ++i)
            hash[i].initialize(s + i);
        description = src.description;
    }

    void writeCounters(const char *filename) {
        ofstream outFile(filename, ios::binary);
        for (int i = 0; i < w; ++i) {
            short est_cnt = counters[i];
            // outFile.write((char *)(&est_cnt), sizeof(short));
            outFile << est_cnt << ",";
        }
    }

    double count0() {
        int cnt = 0;
        for (int i = 0; i < w; ++i)
            cnt += (counters[i] == 0);
        return (double)cnt / w;
    }

    void insert(const string &key, int key_len = 13) {
        for (int i = 0; i < d; ++i) {
            int idx = hash[i].run(key.c_str(), key_len) % w;
            ++counters[idx];
        }
    }

    int query_original(const string &key, int key_len = 13, bool o = true) {
        int ret = INT32_MAX;
        for (int i = 0; i < d; ++i) {
            int idx = hash[i].run(key.c_str(), key_len) % w;
            // for (int t = 0; t < compress_level; t++) {
            //  idx = idx / k[t] + signals[t][idx];
            //}
            // idx >>= 4;
            // idx += (o ? signals[0][idx] : 0);
            ret = min(ret, counters[idx]);
        }
        return ret;
    }
    int query(const string &key, int key_len = 13, bool o = true) {
        int ret = INT32_MAX;
        for (int i = 0; i < d; ++i) {
            int idx = hash[i].run(key.c_str(), key_len) % w;
            // for (int t = 0; t < compress_level; t++) {
            //  idx = idx / k[t] + signals[t][idx];
            //}
            idx = o ? ((idx >> 4) + signals[0][idx]) : (idx >> 4);
            ret = min(ret, counters[idx]);
            // ret = min(ret, idx);
        }
        return ret;
    }

    void compressMax(int in_k, int sig_width = 1) {
        description = "compress-max";
        int layer = compress_level++;
        int k = this->k[layer] = in_k, rng = 1 << sig_width - 1;
        int w = this->w;
        for (int t = 0; t < layer; t++)
            w /= this->k[t];
        int8_t *new_signals = new int8_t[w];
        int *new_counters = new int[w / k];
        memset(new_signals, 0, w * sizeof(int8_t));
        memset(new_counters, 0, (w / k) * sizeof(int));
        for (int i = 0; i < w / k; i++)
            for (int j = i * k; j < (i + 1) * k; j++)
                new_counters[i] = max(new_counters[i], counters[j]);
        delete[] counters;
        counters = new_counters;
        signals[layer] = new_signals;
    }

    void compressIteration(int in_k, int sig_width = 1, bool ignore0 = true
                           /* std::unordered_map<string, int> freq*/) {
        description = "compress-iteration";
        int layer = compress_level++;
        int k = this->k[layer] = in_k, rng = 1 << sig_width - 1;
        int w = this->w;
        for (int t = 0; t < layer; t++)
            w /= this->k[t];
        int8_t *new_signals = new int8_t[w];
        int *new_counters = new int[w / k];
        int *cnt = new int[w / k];
        memset(new_signals, 0, w * sizeof(int8_t));
        memset(new_counters, 0, (w / k) * sizeof(int));
        memset(cnt, 0, (w / k) * sizeof(int));
        for (int i = 0; i < w / k; ++i)
            for (int j = i * k; j < (i + 1) * k; ++j)
                if (counters[j] || !ignore0) {
                    new_counters[i] = max(new_counters[i], counters[j]);
                    ++cnt[i + new_signals[j]];
                }

        for (int t = 0; t < 4; ++t) {
            for (int i = 0; i < w / k; ++i)
                for (int j = i * k; j < (i + 1) * k; ++j)
                    if (counters[j] || !ignore0) {
                        int min_d = INT32_MAX, min_r = 0;
                        for (int r = -rng + 1; r <= rng; ++r) {
                            if (i + r < 0 || i + r >= w / k)
                                continue;
                            int dist =
                                (new_counters[i + r] > counters[j])
                                ? (new_counters[i + r] - counters[j])
                                : (counters[j] - new_counters[i + r]); // * cnt[i + r];
                            if (dist < min_d)
                                min_d = dist, min_r = r;
                        }
                        new_signals[j] = min_r;
                    }

            for (int i = 0; i < w / k; ++i)
                new_counters[i] = 0, cnt[i] = 0;
            for (int i = 0; i < w / k; ++i)
                for (int j = i * k; j < (i + 1) * k; ++j)
                    if (counters[j] || !ignore0) {
                        new_counters[i + new_signals[j]] =
                            max(new_counters[i + new_signals[j]], counters[j]);
                    }
        }
        signals[layer] = new_signals;
        delete[] counters;
        counters = new_counters;
    }
};

#endif
