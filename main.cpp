
#include <iostream>
#include "ZZerg.h"
using namespace sc2;



int main(int argc, char* argv[]) {
    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);
	ZZerg bot;
    coordinator.SetParticipants({
        CreateParticipant(Race::Zerg, &bot),
        CreateComputer(Race::Terran,sc2::Difficulty::HardVeryHard)
    });
	coordinator.SetMultithreaded(true);
	coordinator.SetStepSize(1);
	//coordinator.SetRealtime(true);
    coordinator.LaunchStarcraft();
    coordinator.StartGame("X:/StarCraft II/maps/AbyssalReefLE.SC2Map");
	//coordinator.StartGame(sc2::kMapBelShirVestigeLE);
	
    while (coordinator.Update()) {
    }
	
    return 0;
}