
#include "ZZerg.h"


ProductionManager::ProductionManager(ZZerg &bot) : bot(bot)
{
}

ProductionManager::ProductionManager() : bot(bot)
{
}


ProductionManager::~ProductionManager()
{
}

void ProductionManager::All()
{
	sc2::Units larve = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_LARVA));
	if (larve.size() == 0) return;

	auto larva = sc2::GetRandomEntry(larve);
	ProductionState production_state = GetProductionState();

	QueenProduction();

	if (GetSupplyCap() < GetSupplyOffset()) { OviProduction(larva);  return; }
	if (production_state == army)			{ ArmyProduction(larva); return; }
	if (production_state == drone)			{ DroneProduction(larva); return; }

	



}

void ProductionManager::QueenProduction()
{
	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY,sc2::UNIT_TYPEID::ZERG_LAIR,sc2::UNIT_TYPEID::ZERG_HIVE }));
	sc2::Units queens = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_QUEEN));
	int32_t mineral_count = bot.Observation()->GetMinerals();
	size_t queen_num = queens.size() + AlreadyTrainingCount(bases, sc2::ABILITY_ID::TRAIN_QUEEN);

	//QUEEN PRODUCTION 
	if (bot.pool &&  mineral_count > 149 && queen_num <= bot.GetFinishedBasesCount() + 6) {
		for (auto & BQpair : bot.base_queen) {
			if (BQpair.first == nullptr) continue;
			if ((BQpair.first->orders.size() == 0 && BQpair.second == nullptr)) {
				bot.Actions()->UnitCommand(BQpair.first, sc2::ABILITY_ID::TRAIN_QUEEN);
			}
		}
		//check if all inject queens are trained
		bool need_inject_queen = false;
		for (auto & BQpair : bot.base_queen) {
			if (BQpair.second == nullptr) {
				need_inject_queen = true;
			}
		}
		//if there is no need for inject queen, train creep queen
		if (bot.creep_queen == 0 && need_inject_queen == false && !AlreadyTraining(bases, sc2::ABILITY_ID::TRAIN_QUEEN)) {
			bot.Actions()->UnitCommand(bot.base_queen.front().first, sc2::ABILITY_ID::TRAIN_QUEEN);
		}
		if (bot.creep_queen != 0 && queen_num < bot.GetFinishedBasesCount() + 6) {
			for (auto &base : bases) {
				if (base->orders.size() == 0) {
					bot.Actions()->UnitCommand(base, sc2::ABILITY_ID::TRAIN_QUEEN);
					break;
				}
			}
		}
	}
}

void ProductionManager::DroneProduction(const sc2::Unit *larva)
{

	float workers_count = bot.EconomyManager().GetWorkerCount();
	int32_t mineral_count = bot.Observation()->GetMinerals();

	if (mineral_count > 99 && (workers_count < (bot.GetCurrnetWorkerLimit(true) + 2))) {
		bot.Actions()->UnitCommand(larva, sc2::ABILITY_ID::TRAIN_DRONE);
		return;
	}
}

void ProductionManager::OviProduction(const sc2::Unit *larva)
{
	const sc2::Units eggs = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EGG));
	int32_t mineral_count = bot.Observation()->GetMinerals();
	
	if (mineral_count > 99) {
		bot.Actions()->UnitCommand(larva, sc2::ABILITY_ID::TRAIN_OVERLORD);
		return;
	}

}

void ProductionManager::ArmyProduction(const sc2::Unit *larva)
{
	const sc2::Units spawning_pool = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL));
	const sc2::Units spire = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPIRE));
	const sc2::Units mutalisk = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPIRE));
	int32_t mineral_count = bot.Observation()->GetMinerals();
	int32_t gas_count = bot.Observation()->GetVespene();

	if (mineral_count > 99 && gas_count > 99  && spire.size() > 0 && bot.Observation()->GetFoodUsed()<200 && GetMutaCount() < 15 ) {
		bot.Actions()->UnitCommand(larva, sc2::ABILITY_ID::TRAIN_MUTALISK);
		return;
	}
	if ( mineral_count > 99 && spawning_pool.size() > 0 ) {
		bot.Actions()->UnitCommand(larva, sc2::ABILITY_ID::TRAIN_ZERGLING);
		return;
	}
	
}

ProductionState ProductionManager::GetProductionState()
{
	float workers_count = bot.EconomyManager().GetWorkerCount();
	int32_t enemy_army_supp = bot.EnemyInfo().GetEnemyArmySupp();
	int32_t my_army_supp = bot.Observation()->GetFoodArmy();
	int32_t my_supp = bot.Observation()->GetFoodUsed();


	if (enemy_army_supp > my_army_supp*1.2 ||
		bot.ArmyManagment().we_are_under_attack ||
		workers_count > 76 ||
		my_supp > 38 && my_army_supp <15
		) 
	{
		return army;
	}
	else
	{
		return drone;
	}
}

float ProductionManager::GetSupplyCap()
{
	sc2::Units eggs = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EGG));
	return bot.Observation()->GetFoodCap() + (AlreadyTrainingCount(eggs, sc2::ABILITY_ID::TRAIN_OVERLORD) * 8);
}

float ProductionManager::GetSupplyOffset()
{
	float supply_offset;
	if (bot.Observation()->GetFoodUsed() > 35) {
		supply_offset = 13;
	}
	else {
		supply_offset = 5;
	}
	return bot.Observation()->GetFoodUsed() + supply_offset;
}

float ProductionManager::GetMutaCount()
{
	sc2::Units eggs = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EGG));
	sc2::Units mutas = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_MUTALISK));
	return mutas.size()+ (AlreadyTrainingCount(eggs, sc2::ABILITY_ID::TRAIN_MUTALISK));
}

bool ProductionManager::AlreadyTraining(const sc2::Units &units, sc2::ABILITY_ID ability)
{
	for (auto &unit : units) {
		for (auto &order : unit->orders) {
			if (order.ability_id.ToType() == ability) {
				return true;
			}
		}
	}
	return false;
}

size_t ProductionManager::AlreadyTrainingCount(const sc2::Units units, sc2::ABILITY_ID ability)
{
	size_t count = 0;
	for (auto &unit : units) {
		for (auto &order : unit->orders) {
			if (order.ability_id.ToType() == ability) count++;
		}
	}
	return count;
}