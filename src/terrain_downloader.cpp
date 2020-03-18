#include "terrain_downloader/terrain_downloader.hpp"
#include <sstream>
#include <string>
#include <stdint.h>
#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include "curl/curl.h"
#include "curl/easy.h"
#include "zip.h"
using namespace terrainDownloader;
using namespace std;

vector<vector<u8>> dat;

size_t writeData(char *bufptr, size_t size, size_t nitems, void *userp) {
	auto& vec = dat[u64(userp)];
	vec.insert(vec.end(), bufptr, bufptr + size * nitems);
	return size * nitems;
}

inline void getFile(const string &prefix, u32 i, u32 j, u64 k, f64 x0, f64 y0, f64 dx, f64 dy) {

	stringstream start;

	start <<
		"http://terrain.party/api/export?box=" <<
		(x0 + dx * i) << ',' << (y0 + dy * j) << "," << (x0 + dx * (i + 1)) << "," << (y0 + dy * (j + 1)) <<
		"&name=" << i << "_" << j;

	string str = start.str();

	CURL *c = curl_easy_init();
	curl_easy_setopt(c, CURLOPT_URL, str.c_str());
	curl_easy_setopt(c, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(c, CURLOPT_INFILESIZE_LARGE, 6 * 1024 * 1024);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(c, CURLOPT_WRITEDATA, (void*)k); 
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writeData); 

	auto err = curl_easy_perform(c);

	if (err != CURLE_OK)
		cout << curl_easy_strerror(err) << endl;

	curl_easy_cleanup(c);

	auto &zipData = dat[k];

	zip_error_t error{};
	auto src = zip_source_buffer_create(zipData.data(), zipData.size(), 0, &error);
	auto zip = zip_open_from_source(src, 0, &error);

	auto region = std::to_string(i) + "_" + std::to_string(j);
	auto inFile = region + " Height Map (Merged).png";
	auto outFile = prefix + region + ".png";

	zip_stat_t stat{};
	zip_stat(zip, inFile.c_str(), 0, &stat);

	if (stat.name) {

		vector<u8> output(stat.size);

		zip_file *png = zip_fopen(zip, inFile.c_str(), 0);
		zip_fread(png, output.data(), output.size());
		zip_fclose(png);

		FILE *f = fopen(outFile.c_str(), "wb");
		fwrite(output.data(), output.size(), 1, f);
		fclose(f);
	}

	zip_close(zip);
	zipData.clear();
}

inline void doThreads(const string &prefix, u64 i, u64 j, u64 w, u64 h, f64 x0, f64 y0, f64 dx, f64 dy) {

	u64 tiles = w * h;

	u64 perThread = tiles / j;
	u64 missed = tiles - perThread * j;
	u64 thisThread = perThread;

	if (i < missed) ++thisThread;

	u64 beginRange = perThread * i + (i < missed ? i : missed);
	u64 endRange = beginRange + thisThread;

	for (u64 index = beginRange; index < endRange; ++index)
		getFile(prefix, u32(index % w), u32(index / w), index, x0, y0, dx, dy);

}

int terrainDownloader::save(f64 beginX, f64 endX, f64 beginY, f64 endY, u32 w, u32 h, u8 startMip, u8 endMip) {

	curl_global_init(CURL_GLOBAL_DEFAULT);

	for (u64 mip = 0; mip < endMip; ++mip) {

		f64 deltaX = (endX - beginX) / w;
		f64 deltaY = (endY - beginY) / h;

		if (mip < startMip) {
			w *= 2;
			h *= 2;
			continue;
		}

		dat.clear();
		dat.resize(w * h);

		vector<future<void>> futures;

		u64 threadCount = thread::hardware_concurrency();
		//u64 threadCount = 1;

		for (u64 i = 0, j = threadCount; i < j; ++i)
			futures.push_back(
				std::move(
					std::async(
						[mip, i, j, w, h, beginX, beginY, deltaX, deltaY]() -> void {
							doThreads(std::to_string(mip) + "_", i, j, w, h, beginX, beginY, deltaX, deltaY);
						}
					)
				)
			);

		for (auto &future : futures)
			future.get();

		w *= 2;
		h *= 2;
	}

	curl_global_cleanup();

	return 0;
}