#include "ttt3d.h"
#include "MarkZach.h"

using namespace std;

int main() {
    auto length = minutes(3);
    auto players = new TTT3D*[2];
    
    players[0] = new MZ::MZ(length);
    players[0]->init_clock();
    players[1] = new MZ::MZ(length);
    players[1]->init_clock();

    return 0;
}
