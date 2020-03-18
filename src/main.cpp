#include "terrain_downloader/terrain_downloader.hpp"

int main(){

	f64 beginX = 139.4062546509583, endX = 145.84600172581386;
	f64 beginY = 41.06404108487939, endY = 45.86941734559742;

	u32 w = 1, h = 1;
	u8 startMip = 0, endMip = 1;

	return terrainDownloader::save(beginX, endX, beginY, endY, w, h, startMip, endMip);
}