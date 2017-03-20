#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <climits>

#include "hlt.hpp"
#include "networking.hpp"

#define SEED 42
#define MAX_STRENGTH 255

hlt::Move getMove(hlt::Location location, hlt::GameMap& presentMap, int myID) {
	hlt::Site site = presentMap.getSite(location);
	
	bool inside = true;
	unsigned char dir = 0;
	int min = INT_MAX;

	for (unsigned char d : CARDINALS) {
		if (presentMap.getSite(location, d).owner != myID) {
			inside = false;
			if (presentMap.getSite(location, d).strength < site.strength) {
				if (presentMap.getSite(location, d).strength) {
					min = presentMap.getSite(location, d).strength;
					dir = d;
				}				
			}
		}
	}

	if (!inside && min < site.strength) {
		return {location, dir};
	}

	if (site.strength <= 5 * site.production) {
		return {location, STILL};
	}
	dir = (unsigned char) (rand() % 2);
	if (dir == 0) {
		dir = NORTH;
	} else {
		dir = WEST;
	}
	return {location, dir};
}

int main() {
	srand(SEED);

	std::cout.sync_with_stdio(0);

	unsigned char myID;
	hlt::GameMap presentMap;
	getInit(myID, presentMap);
	sendInit("StupidRandomBot");

	std::set<hlt::Move> moves;
	while (true) {
		moves.clear();

		getFrame(presentMap);

		for (unsigned short a = 0; a < presentMap.height; a++) {
			for (unsigned short b = 0; b < presentMap.width; b++) {
				if (presentMap.getSite({b, a}).owner == myID) {
					moves.insert(getMove({b, a}, presentMap, myID));
				}
			}
		}

		sendFrame(moves);
	}

	return 0;
}