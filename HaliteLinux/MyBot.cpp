#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <climits>
#include <queue>
#include <fstream>

#include "hlt.hpp"
#include "networking.hpp"

#define SEED 42
#define MAX_STRENGTH 255

using namespace std;

void setSitesLocation(hlt::GameMap& presentMap) {

	for (unsigned short i = 0; i < presentMap.height; i++) {
		for (unsigned short j = 0; j < presentMap.width; j++) {
			hlt::Site& site = presentMap.getSite({j, i});
			site.location = {j, i};
		}
	}

}

// Clasa de compare pentru priority queue
class scoreCompare {
public:
	bool operator() (const hlt::Site& lhs, const hlt::Site& rhs) const {
		return lhs.score < rhs.score;
	}
};

// Atribuie scoruri pentru site-urile noastre si pentru cele din jur
void setScores(hlt::GameMap& presentMap, int myID) {

	priority_queue < hlt::Site, vector<hlt::Site>, scoreCompare> scoreQueue;

	// Seteaza scoruri pentru site-urile din jurul bot-ului
	for (unsigned short i = 0; i < presentMap.height; i++) {
		for (unsigned short j = 0; j < presentMap.width; j++) {
			hlt::Site& site = presentMap.getSite({j, i});
			if (site.owner != myID) {
				bool borderSite = false;
				for (unsigned char d : CARDINALS) {
					hlt::Site& neighbour = presentMap.getSite({j, i}, d);
					if (neighbour.owner == myID) {
						borderSite = true;
						break;
					}
				}
				if (borderSite) {
					site.score = site.production * 5 - (site.strength * 7 / 10);
					scoreQueue.emplace(site);
				}
			}
		}
	}

	vector<vector<bool>> scored = vector<vector<bool>>
		(presentMap.height, vector<bool>(presentMap.width, false));

	// Seteaza scoruri pentru site-urile bot-ului
	while (!scoreQueue.empty()) {
		hlt::Site site = scoreQueue.top();
		scoreQueue.pop();
		for (unsigned char d : CARDINALS) {
			hlt::Site& neighbour = presentMap.getSite(site.location, d);
			if (neighbour.owner == myID && !scored[neighbour.location.y][neighbour.location.x]) {
				scored[neighbour.location.y][neighbour.location.x ] = true;
				neighbour.score = site.score - neighbour.production - 2;
				neighbour.direction = presentMap.reverseDirection(d);
				scoreQueue.emplace(neighbour);
			}
		}
	}

}

// Verifica daca un site e la margine
bool isBorder(hlt::Location location, hlt::GameMap& presentMap, int myID) {
	for (unsigned char d : CARDINALS) {
		hlt::Site neighbour = presentMap.getSite(location, d);
		if (neighbour.owner != myID) {
			return true;
		}
	}
	return false;
}

// Returneaza cea mai buna mutare pentru o celula in frame-ul curent 
hlt::Move getMove(hlt::Location location, hlt::GameMap& presentMap, int myID) {
	hlt::Site site = presentMap.getSite(location);
	unsigned char dir = 0;

	int maxProd = INT_MIN;

	// Gaseste cel mai bun site de cucerit (daca exista)
	if (isBorder(location, presentMap, myID)) {
		for (unsigned char d : CARDINALS) {
			hlt::Site neighbour = presentMap.getSite(location, d);
			if (neighbour.owner != myID) {
				if (neighbour.strength < site.strength && neighbour.production > maxProd) {
					dir = d;
					maxProd = neighbour.production;
				}
			}
		}
	}
	if (maxProd != INT_MIN) {
		return {location, dir};
	}

	if (site.strength < 6 * site.production && site.strength < 255) {
		return {location, STILL};
	}

	hlt::Site dest = presentMap.getSite(location, site.direction);

	if (dest.owner==myID || dest.strength < site.strength) {
		return {location, site.direction};
	}
	else {
		return {location, STILL};
	}
}

int main() {
	srand(SEED);

	std::cout.sync_with_stdio(0);

	unsigned char myID;
	hlt::GameMap presentMap;
	getInit(myID, presentMap);

	sendInit("ZeroDay");

	std::set<hlt::Move> moves;
	while (true) {
		moves.clear();

		getFrame(presentMap);
		setSitesLocation(presentMap);
		setScores(presentMap, myID);

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