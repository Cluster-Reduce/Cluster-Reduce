#ifndef __MIN_MAX_SKETCH_H__
#define __MIN_MAX_SKETCH_H__

#include <cstring>
#include <algorithm>
#include "BOBHash32.h"

class MinMaxSketch
{
private:
	int w, d, k, max_value;
	int* counters, * signals;
	BOBHash32* hash;

public:
	void init(int in_w, int in_d, int in_max)
	{
		w = in_w, d = in_d, k = 1, max_value = in_max;
		counters = new int[w];
		signals = new int[w];
		for (int i = 0; i < w; ++i)
			counters[i] = max_value;
		memset(signals, 0, w * sizeof(int));
		hash = new BOBHash32[d];
		for (int i = 0; i < d; ++i)
			hash[i].initialize(750 + i);
	}

	void print()
	{
		for (int i = 0; i < w; ++i)
		{
			int idx = i / k + signals[i];
			int val = (counters[idx] == max_value ? -1 : counters[idx]);
			cout << val << " ";
		}
		cout << endl << endl;
	}

	void clear()
	{
		delete[]counters;
		delete[]signals;
		delete[]hash;
	}

	void insert(int key, int bucket_id)
	{
		for (int i = 0; i < d; ++i)
		{
			int idx = hash[i].run((char*)&key, 4) % w;
			counters[idx] = min(counters[idx], bucket_id);
		}
	}

	int query(int key)
	{
		int ret = 0;
		for (int i = 0; i < d; ++i)
		{
			int idx = hash[i].run((char*)&key, 4) % w;
			int val = counters[idx / k + signals[idx]];
			ret = max(ret, val);
		}
		return ret;
	}

	void compressIteration(int in_k, int sig_width, vector<double> quantiles)
	{
		k = in_k; int rng = 1 << sig_width - 1;
		int* new_counters = new int[w / k + 1];
		for (int i = 0; i <= w / k; ++i)
			new_counters[i] = max_value;

		for (int t = 0; t < 10; ++t)
		{
			for (int i = 0; i < w / k; ++i)
				for (int j = i * k; j < min((i + 1) * k, w); ++j)
					if (counters[j] < max_value)
					{
						double min_d = 1e9; int min_r = 0;
						for (int r = -rng + 1; r <= rng; ++r)
						{
							if (i + r < 0 || i + r >= w / k) continue;
							double dist = abs(quantiles[new_counters[i + r]] - quantiles[counters[j]]);
							if (dist < min_d)
								min_d = dist, min_r = r;
						}
						signals[j] = min_r, new_counters[i + signals[j]] = min(new_counters[i + signals[j]], counters[j]);
					}

			for (int i = 0; i <= w / k; ++i)
				new_counters[i] = max_value;
			for (int i = 0; i <= w / k; ++i)
				for (int j = i * k; j < min((i + 1) * k, w); ++j)
					if (counters[j] < max_value)
						new_counters[i + signals[j]] = min(new_counters[i + signals[j]], counters[j]);
		}
		delete[]counters;
		counters = new_counters;
	}
};

#endif
