
#include <iostream>
#include "ZZerg.h"




int main(int argc, char* argv[]) {
	sc2::Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	ZZerg bot;
	coordinator.SetParticipants({
		CreateParticipant(sc2::Race::Zerg, &bot),
		CreateComputer(sc2::Race::Terran,sc2::Difficulty::CheatInsane)
		});
	coordinator.SetMultithreaded(true);
	coordinator.SetStepSize(1);
	//coordinator.SetRealtime(true);
	coordinator.LaunchStarcraft();
	coordinator.StartGame("X:/StarCraft II/maps/InterloperLE.SC2Map");
	//coordinator.StartGame(sc2::kMapBelShirVestigeLE);

	while (coordinator.Update()) {
	}

	return 0;
}