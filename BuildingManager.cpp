#include <iostream>


#include "ZZerg.h"
#include "BuildingManager.h"



BuildingManager::BuildingManager(ZZerg &bot) :bot(bot)
{
}


BuildingManager::~BuildingManager()
{
}



void BuildingManager::BuildingManagerStep()
{
	int current_supply = bot.Observation()->GetFoodUsed();
	sc2::Units pool = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL));
	sc2::Units lair = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_LAIR));
	sc2::Units spire = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPIRE));
	sc2::Units drones = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));
	sc2::Units hatches = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY,sc2::UNIT_TYPEID::ZERG_LAIR,sc2::UNIT_TYPEID::ZERG_HIVE}));
	sc2::Units extractors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));
	sc2::Units baneling_nests = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_BANELINGNEST));
	int active_gas_count = bot.EconomyManager().GetActiveGasCount();
	float finished_bases_count = bot.EconomyManager().GetFinishedBasesCount();

	if (bot.Observation()->GetMinerals() > 300) save_minerals = false;
	//Build Order

	//Fast expand
	if (bot.frame % 100 == 0) {
		if (bot.EnemyInfo().GetEnemyAirCount() == 0) {

		}
		else if (bot.EnemyInfo().GetEnemyAirCount() == 1) {
			//check if all bases have at least one spore
			for (auto & base : hatches) {
				size_t spine_count = GetStaticDefCount(base).first;
				if (spine_count < 1) { TryBuildSpore(base); }
			}
		}
		else if (bot.EnemyInfo().GetEnemyAirCount() < 4) {
			//check if all bases have at least
			for (auto & base : hatches) {
				size_t spine_count = GetStaticDefCount(base).first;
				if (spine_count < 2) { TryBuildSpore(base); }
			}
		}
		else if (bot.EnemyInfo().GetEnemyAirCount() < 10) {
			//check if all bases have at least 
			for (auto & base : hatches) {
				size_t spine_count = GetStaticDefCount(base).first;
				if (spine_count < 4) { TryBuildSpore(base); }
			}
		}
	}

	if (!OrderInProgress(sc2::ABILITY_ID::BUILD_HATCHERY) && hatches.size() == 1) {
		if (!TryExpand(sc2::ABILITY_ID::BUILD_HATCHERY, sc2::UNIT_TYPEID::ZERG_DRONE) || bot.Observation()->GetMinerals() < 300) {
			std::cout << "------------------------ first base" << std::endl;
			return;
		}
	}
	
	if (current_supply > 17) {


		//build spawning pool
		if (pool.size() == 0 && !OrderInProgress(sc2::ABILITY_ID::BUILD_SPAWNINGPOOL)) {
			if ((!TryBuildSpawningPool()) || (bot.Observation()->GetMinerals() < 200)) {
				std::cout << "------------------------spawning pool" << std::endl;
				return;
			}
		}

		if (!OrderInProgress(sc2::ABILITY_ID::BUILD_EXTRACTOR) && extractors.size() == 0) {
			if (!TryBuildGas(sc2::ABILITY_ID::BUILD_EXTRACTOR, sc2::UNIT_TYPEID::ZERG_DRONE, bot.main_base->pos,true)) {
				std::cout << "---------------------extractor first" << std::endl;
				return;
			}
		}
		if (lair.size() > 0 && extractors.size()  < finished_bases_count * 2  && active_gas_count + OrderCount(sc2::ABILITY_ID::BUILD_EXTRACTOR) < 8) {
			for (auto &base : hatches) {
				if (base->build_progress == 1 && TryBuildGas(sc2::ABILITY_ID::BUILD_EXTRACTOR, sc2::UNIT_TYPEID::ZERG_DRONE, base->pos, false)) {
					return;
				}
			}
			
			
		/*	if (!TryBuildGas(sc2::ABILITY_ID::BUILD_EXTRACTOR, sc2::UNIT_TYPEID::ZERG_DRONE,sc2::GetRandomEntry(hatches)->pos, false)) {
				std::cout << "---------------------build all extractors" << std::endl;
				return;
			}*/

		}

		/*if ((!OrderInProgress(sc2::ABILITY_ID::BUILD_EXTRACTOR) && extractors.size() == 2 && hatches.size()==4) || (!OrderInProgress(sc2::ABILITY_ID::BUILD_EXTRACTOR) && hatches.size() == 4 && extractors.size() == 3) || (active_gas_count < hatches.size() && !OrderInProgress(sc2::ABILITY_ID::BUILD_EXTRACTOR) && hatches.size()>2)) {
			if (!TryBuildGas(sc2::ABILITY_ID::BUILD_EXTRACTOR, sc2::UNIT_TYPEID::ZERG_DRONE, hatches[0]->pos)) {

				return;
			}
		}*/

		if (baneling_nests.size() == 0 && !OrderInProgress(sc2::ABILITY_ID::BUILD_BANELINGNEST) && current_supply > 55) {
			if (!TryBuildBaneling() || bot.Observation()->GetMinerals() < 200 || bot.Observation()->GetVespene() < 150) {
				std::cout << "----------------------baneling nest" << std::endl;

				return; //dovrsiiii
			}
		}
		if (current_supply > 32 && drones.size() > 22 && hatches.size()+ OrderCount(sc2::ABILITY_ID::BUILD_HATCHERY) < 3) {   //need better solution
			if (bot.Observation()->GetMinerals() < 300) save_minerals = true;
			else save_minerals = false;
			if (bot.Observation()->GetMinerals() < 300 || !TryExpand(sc2::ABILITY_ID::BUILD_HATCHERY, sc2::UNIT_TYPEID::ZERG_DRONE)) {
				std::cout << "----------------------third baseee" << std::endl;
				return;
			}
		}
		
		if (current_supply > 60 && hatches.size() > 2 && lair.size() > 0 && spire.size()+ OrderCount(sc2::ABILITY_ID::BUILD_SPIRE)==0) {
			if (bot.Observation()->GetMinerals() < 200) save_minerals = true;
			else save_minerals = false;
			if (bot.Observation()->GetMinerals() < 200 || bot.Observation()->GetVespene() < 200 || !TryBuildSpire()) {
				std::cout << "-----------------------spireeee" << std::endl;
				return;
			}
		}
		
		if ((drones.size() > bot.GetCurrnetWorkerLimit(true) + 10 ) || (bot.Observation()->GetMinerals() > 2000 && hatches.size() < 8) || (current_supply > 70 && hatches.size() + OrderCount(sc2::ABILITY_ID::BUILD_HATCHERY) < 4) || (current_supply > 130 && hatches.size() + OrderCount(sc2::ABILITY_ID::BUILD_HATCHERY) < 5)) {
			if (bot.Observation()->GetMinerals() < 300) save_minerals = true;
			else save_minerals = false;
			if(bot.Observation()->GetMinerals() < 300 || !TryExpand(sc2::ABILITY_ID::BUILD_HATCHERY, sc2::UNIT_TYPEID::ZERG_DRONE));
			std::cout << "--------------------------expand if ...." << std::endl;
			return;
		}
		
		
	}

}





bool BuildingManager::TryBuildStructure(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Point2D location, bool isExpansion ) {

	const sc2::ObservationInterface* observation = bot.Observation();
	sc2::Units workers = observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));

	//if we have no workers Don't build
	if (workers.empty()) {
		return false;
	}

	// Check to see if there is already a worker heading out to build it
	for (const auto& worker : workers) {
		for (const auto& order : worker->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}
	}

	// If no worker is already building one, get a random worker to build one
	const sc2::Unit* unit = GetRandomEntry(workers);

	// Check to see if unit can make it there
	if (bot.Query()->PathingDistance(unit, location) < 0.1f) {
		return false;
	}
	if (!isExpansion) {
		for (const auto& expansion : bot.expansions_) {
			if (Distance2D(location, sc2::Point2D(expansion.x, expansion.y)) < 7) {
				return false;
			}
		}
	}
	// Check to see if unit can build there
	if (bot.Query()->Placement(ability_type_for_structure, location)) {
		bot.Actions()->UnitCommand(unit, ability_type_for_structure, location);
		return true;
	}
	return false;

}

//Try to build a structure based on tag, Used mostly for Vespene, since the pathing check will fail even though the geyser is "Pathable"
bool BuildingManager::TryBuildStructure(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Tag location_tag,bool check_if_building) {
	const sc2::ObservationInterface* observation = bot.Observation();
	sc2::Units workers = observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));
	const sc2::Unit* target = observation->GetUnit(location_tag);

	if (workers.empty()) {
		return false;
	}

	// Check to see if there is already a worker heading out to build it
	if (check_if_building) {
		for (const auto& worker : workers) {
			for (const auto& order : worker->orders) {
				if (order.ability_id == ability_type_for_structure) {
					return false;
				}
			}
		}
	}

	// If no worker is already building one, get a random worker to build one
	const sc2::Unit* unit = GetRandomEntry(workers);

	// Check to see if unit can build there
	if (bot.Query()->Placement(ability_type_for_structure, target->pos)) {
		bot.Actions()->UnitCommand(unit, ability_type_for_structure, target);
		return true;
	}
	return false;

}

//Expands to nearest location and updates the start location to be between the new location and old bases.
bool BuildingManager::TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type) {
	const sc2::ObservationInterface* observation = bot.Observation();
	float minimum_distance = std::numeric_limits<float>::max();
	sc2::Point3D closest_expansion;
	for (const auto& expansion : bot.expansions_) {
		float current_distance = sc2::Distance2D(bot.startLocation_, expansion);
		if (current_distance < .01f) {
			continue;
		}

		if (current_distance < minimum_distance) {
			if (bot.Query()->Placement(build_ability, expansion)) {
				closest_expansion = expansion;
				minimum_distance = current_distance;
			}
		}
	}
	//only update staging location up till 3 bases.
	if (TryBuildStructure(build_ability, worker_type, closest_expansion, true) && observation->GetUnits(sc2::Unit::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY)).size() < 4) {
		bot.staging_location_ = sc2::Point3D(((bot.staging_location_.x + closest_expansion.x) / 2), ((bot.staging_location_.y + closest_expansion.y) / 2),
			((bot.staging_location_.z + closest_expansion.z) / 2));
		return true;
	}
	return false;

}

//Tries to build a geyser for a base
bool BuildingManager::TryBuildGas(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type, sc2::Point2D base_location, bool check_if_building) {
	
	sc2::Units geysers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
	sc2::Units geysers2 = bot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER));
	
	geysers.insert(geysers.end(), geysers2.begin(), geysers2.end());
;
	//only search within this radius
	float minimum_distance = 15.0f;
	sc2::Tag closestGeyser = 0;
	for (const auto& geyser : geysers) {
		float current_distance = Distance2D(base_location, geyser->pos);
		if (current_distance < minimum_distance) {
			if (bot.Query()->Placement(build_ability, geyser->pos)) {
				minimum_distance = current_distance;
				closestGeyser = geyser->tag;
			}
		}
	}

	// In the case where there are no more available geysers nearby
	if (closestGeyser == 0) {
		return false;
	}
	if (check_if_building) { return TryBuildStructure(build_ability, worker_type, closestGeyser,true ); }
	else { return TryBuildStructure(build_ability, worker_type, closestGeyser, false); }
	
}

bool BuildingManager::TryBuildSpawningPool()
{
	const sc2::ObservationInterface* observation = bot.Observation();

	sc2::Point2D mineral_near_base = bot.EconomyManager().FindNearestMineralNode(bot.main_base)->pos;
	std::cout << mineral_near_base.x << " -- " << mineral_near_base.y << std::endl;
	sc2::Point2D pool_place;
	pool_place.x = 2 * bot.startLocation_.x - mineral_near_base.x;
	pool_place.y = 2 * bot.startLocation_.y - mineral_near_base.y;
	sc2::Point3D debug;
	debug.x = pool_place.x;
	debug.y = pool_place.y;
	debug.z = bot.startLocation_.z;

	return TryBuildStructure(sc2::ABILITY_ID::BUILD_SPAWNINGPOOL, sc2::UNIT_TYPEID::ZERG_DRONE, pool_place,true);
}

bool BuildingManager::TryBuildBaneling()
{
	const sc2::ObservationInterface* observation = bot.Observation();

	sc2::Point2D mineral_near_base = bot.EconomyManager().FindNearestMineralNode(bot.main_base)->pos;
	std::cout << mineral_near_base.x << " -- " << mineral_near_base.y << std::endl;
	sc2::Point2D pool_place;
	pool_place.x = 2 * bot.startLocation_.x - mineral_near_base.x;
	pool_place.y = 2 * bot.startLocation_.y - mineral_near_base.y;
	sc2::Point3D debug;
	debug.x = pool_place.x;
	debug.y = pool_place.y;
	debug.z = bot.startLocation_.z;
	pool_place.x += sc2::GetRandomInteger(-4, 4);
	pool_place.y += sc2::GetRandomInteger(-4, 4);
	
	return TryBuildStructure(sc2::ABILITY_ID::BUILD_BANELINGNEST, sc2::UNIT_TYPEID::ZERG_DRONE, pool_place, true);
}

bool BuildingManager::TryBuildSpire()
{
	const sc2::ObservationInterface* observation = bot.Observation();

	sc2::Point2D mineral_near_base = bot.EconomyManager().FindNearestMineralNode(bot.main_base)->pos;

	sc2::Point2D pool_place;
	pool_place.x = 2 * bot.startLocation_.x - mineral_near_base.x;
	pool_place.y = 2 * bot.startLocation_.y - mineral_near_base.y;
	sc2::Point3D debug;
	debug.x = pool_place.x;
	debug.y = pool_place.y;
	debug.z = bot.startLocation_.z;
	pool_place.x += sc2::GetRandomInteger(-4, 4);
	pool_place.y += sc2::GetRandomInteger(-4, 4);

	return TryBuildStructure(sc2::ABILITY_ID::BUILD_SPIRE, sc2::UNIT_TYPEID::ZERG_DRONE, pool_place, true);
}

bool BuildingManager::TryBuildSpore(const sc2::Unit *base)
{
	const float pi = 3.14;
	std::vector<sc2::Point2D> spore_locs;
	for (float z = 0, x = 0; z < 4; z++, x = x + pi / 2) {
		spore_locs.push_back(sc2::Point2D(base->pos.x + (3 * cos(x)), base->pos.y + (3 * sin(x))));
	}
	return TryBuildStructure(sc2::ABILITY_ID::BUILD_SPORECRAWLER, sc2::UNIT_TYPEID::ZERG_DRONE, sc2::GetRandomEntry(spore_locs), true);
}

bool BuildingManager::OrderInProgress(sc2::AbilityID ability_id)
{
	
	for (auto & worker : bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE))) {
		if (worker->orders.empty()) continue;
		for (auto & worker_order : worker->orders) {
			if (worker_order.ability_id == ability_id) {
				std::cout << "Order IN PROGRESS -> " << ability_id << std::endl;
				return true;
			}
		}
	}
	return false;
}

size_t BuildingManager::OrderCount(sc2::AbilityID ability_id)
{
	auto workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));
	size_t count = 0;
	for (auto & drone : workers) {
		if (drone->orders.size() > 0) {
			if (drone->orders.front().ability_id == ability_id) {
				count++;
			}
		}
	}
	return count;
}

std::pair<size_t, size_t> BuildingManager::GetStaticDefCount(const sc2::Unit * unit)
{
	std::pair<size_t, size_t> spore_spine(0, 0);
	sc2::Units static_def = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_SPORECRAWLER,sc2::UNIT_TYPEID::ZERG_SPINECRAWLER }));
	for (auto &def_unit : static_def) {
		if (sc2::Distance2D(def_unit->pos, unit->pos) < 6) {
			if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPORECRAWLER) spore_spine.first++;
			else if (unit->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_SPINECRAWLER) spore_spine.second++;
		}
	}
	return spore_spine;
}
