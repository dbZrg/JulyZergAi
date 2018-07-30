#pragma once
class ZZerg;
enum ProductionState { army, drone };
class ProductionManager
{
	ZZerg &bot;
public:
	ProductionManager(ZZerg &bot);
	ProductionManager();
	~ProductionManager();

	void All();
	void QueenProduction();
	void DroneProduction(const sc2::Unit *larva);
	void OviProduction(const sc2::Unit *larva);
	void ArmyProduction(const sc2::Unit *larva);


	ProductionState GetProductionState();
	//!Supply cap with overlords already in production
	float GetSupplyCap();
	float GetSupplyOffset();
	bool AlreadyTraining(const sc2::Units &units, sc2::ABILITY_ID ability);
	size_t ProductionManager::AlreadyTrainingCount(const sc2::Units units, sc2::ABILITY_ID ability);
};

