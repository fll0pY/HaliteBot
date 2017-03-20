#include <ctime>
#include <cstdlib>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>

#include "hlt.hpp"
#include "networking.hpp"

unsigned char myID;
hlt::GameMap presentMap;

/* returneaza prima directie in care se afla o locatie straina, altfel
   returneaza -1
*/
int isBorder(unsigned short line, unsigned short column) {
    for (int dir : CARDINALS) {
        if (presentMap.getSite({line, column}, dir).owner != myID) {
            return dir;
        }
    }

    return -1;
}

/* returnez directia in care se afla cea mai apropiata margine */
hlt::Move getNearestBorder(unsigned short line, unsigned short column) {
    hlt::Move newMove;
    hlt::Location newLocation = {line, column};

    unsigned char direction;
    unsigned short crtDist;
    unsigned short production;
    unsigned short crtProduction;
    unsigned short maxProduction = 0;
    unsigned short minDist = max(presentMap.height, presentMap.width) >> 1;

    /* determin distanta minima pana la o margine */
    for (int dir : CARDINALS) {
        crtDist = 0;
        crtProduction = 0;
        newLocation = {line, column};

        /* ma extind cat pot de mult pe directia curenta */
        while (crtDist < minDist && presentMap.getSite(newLocation, dir).owner == myID) {
            newLocation = presentMap.getLocation(newLocation, dir);
            ++crtDist;
        }

        /* actualizez distanta minima */
        if (crtDist < minDist) {
            minDist = crtDist;
            direction = dir;
        } else if (crtDist == minDist && crtProduction > maxProduction) {
            maxProduction = crtProduction;
            direction = dir;
        }
    }

    newMove = {{line, column}, direction};
    return newMove;
}

/* returneaza "cea mai buna" mutare */
hlt::Move getMove(unsigned short line, unsigned short column) {
    hlt::Move newMove;
    newMove.dir = STILL;
    newMove.loc = {line, column};

    /* daca sunt la marginea regiunii mele */
    if (isBorder(line, column) != -1) {
        int production = 0;
        for (int dir : CARDINALS) {
            hlt::Site nextSite = presentMap.getSite({line, column}, dir);
            hlt::Site crtSite = presentMap.getSite({line, column});

            /* verific daca pot sa cuceresc ceva */
            if (nextSite.owner != myID && nextSite.strength < crtSite.strength
                && nextSite.production > production) {
                production = nextSite.production;
                newMove.dir = dir;
            }
        }

        if (production > 0) {
            return newMove;
        }
    }

    int strength = presentMap.getSite({line, column}).strength;
    int production = presentMap.getSite({line, column}).production;

    /* prefer sa stau sa produc, decat sa mut aiurea */
    if (strength == 0 || strength < 5 * production) {
        return newMove;
    }

    return getNearestBorder(line, column);
}

int main() {
    std::cout.sync_with_stdio(0);

    getInit(myID, presentMap);
    sendInit("The Dark Byte Rises");

    std::set<hlt::Move> moves;
    while(true) {
        moves.clear();
        getFrame(presentMap);

        for(unsigned short i = 0; i < presentMap.height; ++i) {
            for(unsigned short j = 0; j < presentMap.width; ++j) {
                if (presentMap.getSite({j, i}).owner == myID) {
                    moves.insert(getMove(j, i));
                }
            }
        }

        sendFrame(moves);
    }

    return 0;
}
