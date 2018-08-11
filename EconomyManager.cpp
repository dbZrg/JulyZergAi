#include <iostream>


#include "ZZerg.h"
#include "EconomyManager.h"

#include "sc2lib/sc2_lib.h"



EconomyManager::EconomyManager(ZZerg &bot) : bot(bot)
	
{

}

EconomyManager::~EconomyManager()
{
}






void EconomyManager::EconomyManagerAll()
{
	
	
	QueenInjectManager();
	EconomyOptimizer();
	if (!bot.worker_rally_set) {
		SetWorkerWaypoint();
	}
	CreepSpread();
	UpgradesManager();
	
}


void EconomyManager::EconomyOptimizer()
{
	auto start = std::chrono::high_resolution_clock::now();

	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY, sc2::UNIT_TYPEID::ZERG_LAIR }));
	sc2::Units extractors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));
	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));

	for (auto &base : bases) {
		if ((base->assigned_harvesters > base->ideal_harvesters) && FindNonOptimalMiningLoc()!=nullptr) {
			for (auto &worker : workers) {
				if (UnitIsInRadius(base, worker) && sc2::Distance2D(FindNearestMineralNode(worker)->pos, worker->pos) < 1.5) {		
					bot.Actions()->UnitCommand(worker, sc2::ABILITY_ID::HARVEST_GATHER, FindNonOptimalMiningLoc());
					return;
				}
			}
		}
	}
	if (FindNonOptimalMiningLoc() != nullptr  && bot.Observation()->GetMinerals() + 1000 < bot.Observation()->GetVespene()) {
		drones_per_ext = 2;
	}
	else if (bot.Observation()->GetMinerals() > bot.Observation()->GetVespene() || bot.Observation()->GetVespene() < 300) {
		drones_per_ext = 0;
	}
	else if (workers.size() < 16) {
		drones_per_ext = 0;
	}
	for (auto &extractor : extractors) {
		int count = extractor->assigned_harvesters - (extractor->ideal_harvesters - drones_per_ext);
		if (count > 0) {
			const sc2::Unit * target = nullptr;
			target = FindNearestWorker(extractor);
			bot.Actions()->UnitCommand(target, sc2::ABILITY_ID::HARVEST_GATHER, FindNearestMineralNode(target));
			continue;
		}
		if (count < 0) {
			const sc2::Unit * target = nullptr;
			target = FindNearestMineralCarrier(extractor);
			bot.Actions()->UnitCommand(target, sc2::ABILITY_ID::HARVEST_GATHER, extractor);
			continue;
		}
	}
	/*for (auto &worker : workers) {
		if ( sc2::Distance2D(FindNearestBase(worker, bot)->pos, worker->pos) > 20) {
			if (worker->orders.size() > 0) {

				if (sc2::Distance2D(worker->orders.front().target_pos, FindNearestBase(bot.Observation()->GetUnit(worker->orders.front().target_unit_tag), bot)->pos) > 20 && ((worker->orders.front().ability_id.ToType() == sc2::ABILITY_ID::HARVEST_GATHER || worker->orders.front().ability_id.ToType() == sc2::ABILITY_ID::HARVEST_RETURN))) {
					const sc2::Unit *base_target = FindNearestBase(worker, bot);
					bot.Actions()->UnitCommand(worker, sc2::ABILITY_ID::HARVEST_GATHER, FindNearestMineralNode(base_target, bot));
				}
			}
		}

	}*/
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = stop - start;
	if (bot.frame % 30 == 0)std::cout << "Economy manager -> economy optimizer : " << elapsed.count() << " s\n";
}

void EconomyManager::QueenInjectManager()
{
	auto start = std::chrono::high_resolution_clock::now();
	

	float n;
	if (bot.upORdown == 1) n = 8;
	else if (bot.upORdown == 2) n = -8;
	sc2::Point2D creep;
	sc2::Point2D creep_start_pos;
	sc2::Point2D creep_start_pos_2;
	creep.x = 0;
	creep.y = n;
	
	const sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY));
	const sc2::Units queens = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_QUEEN));
	for (auto &queen : queens) {		//change to getUnit since its only one creep queen // better to iterate just creep_queen not all queens
		if (queen->energy > 25) {
			if (queen->tag != bot.creep_queen) {
				for (auto& BQpair : bot.base_queen) {
					if (BQpair.second == nullptr) continue;
					if (BQpair.second->tag == queen->tag) {
						bot.Actions()->UnitCommand(queen, sc2::ABILITY_ID::EFFECT_INJECTLARVA, BQpair.first);
					}
				}
			}
			 if (queen->tag == bot.creep_queen && bot.active_creep_tumors_.size() < 9 && queen->orders.size() == 0) {
				 for (auto & base : bases) {
					 creep_start_pos.x = base->pos.x;
					 creep_start_pos.y = base->pos.y;
					 creep_start_pos.x = creep_start_pos.x + creep.x;
					 creep_start_pos.y = creep_start_pos.y+ creep.y;

					if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_QUEEN,creep_start_pos)) {
						
						bot.Actions()->UnitCommand(queen, sc2::ABILITY_ID::BUILD_CREEPTUMOR_QUEEN,creep_start_pos);
						return;
					}
					
					else if(bot.active_creep_tumors_.size() < bot.GetFinishedBasesCount() && base->tag != bot.main_base->tag)
					{
						creep_start_pos.x += sc2::GetRandomInteger(-2, 2);
						creep_start_pos.y += sc2::GetRandomInteger(-2, 2);
						if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_QUEEN, creep_start_pos)) {
							bot.Actions()->UnitCommand(queen, sc2::ABILITY_ID::BUILD_CREEPTUMOR_QUEEN, creep_start_pos);
							return;
						}
					}
				 }
			 }
		}

	}
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = stop - start;
	if (bot.frame % 30 == 0)std::cout << "Economy manager -> queen inject : " << elapsed.count() << " s\n";
}

void EconomyManager::SetWorkerWaypoint()
{
	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY));
	for (auto & base : bases) {
		if (base->build_progress < 1)	return;				//if base is under construction wait until it finishes then set waypoint
		if (base->build_progress == 1) {
			bot.Actions()->UnitCommand(base, sc2::ABILITY_ID::RALLY_WORKERS, base->pos);
		}
	}
	bot.worker_rally_set = true;
}

void EconomyManager::CreepSpread()
{
	auto start = std::chrono::high_resolution_clock::now();


	sc2::Point2D right;
	sc2::Point2D left;
	float pi = 3.14;
	float circle_start_pos;
	float step_distance = pi / 20;
	float dx = bot.startLocation_.x - bot.enemy_location_.x;
	float dy = bot.startLocation_.y - bot.enemy_location_.y;
	circle_start_pos = atan2(dx, dy); 
	float radius=8;
	bool creep_tumor_ready = false;

	float angle_1 = circle_start_pos + pi/2;
	float angle_2 = circle_start_pos - pi/2;
	int count = 0;
	for (auto & creep_tumor : bot.active_creep_tumors_) {
		sc2::Point3D debug(0, 0, creep_tumor->pos.z);
		auto tumor_abilities = bot.Query()->GetAbilitiesForUnit(creep_tumor).abilities;
		
		for (auto & ability : tumor_abilities) {
			if (ability.ability_id.ToType() == sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR || ability.ability_id.ToType() == sc2::ABILITY_ID::BUILD_CREEPTUMOR || ability.ability_id.ToType() == sc2::ABILITY_ID::BUILD_CREEPTUMOR_QUEEN) {
				creep_tumor_ready = true;
			}
		}
		if (creep_tumor_ready) {
				
				for (float i = circle_start_pos, z = circle_start_pos, counter = 0; counter < 12; counter++, i = i + step_distance, z = z - step_distance) {

					right.x = creep_tumor->pos.x + (radius * cos(z));
					right.y = creep_tumor->pos.y + (radius * sin(z));
					

					debug.x = right.x;
					debug.y = right.y;
					bot.Debug()->DebugLineOut(debug, creep_tumor->pos);
					bot.Debug()->DebugSphereOut(debug, 1);
					bot.Debug()->SendDebug();
					if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, right) && !InRangeOfTumor(right)) {
						bot.Actions()->UnitCommand(creep_tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR, right);
					
						return;
					}
					
					left.x = creep_tumor->pos.x + (radius * cos(i));
					left.y = creep_tumor->pos.y + (radius * sin(i));
					debug.x = left.x;
					debug.y = left.y;
					bot.Debug()->DebugLineOut(debug, creep_tumor->pos);
					bot.Debug()->DebugSphereOut(debug, 1);
					bot.Debug()->SendDebug();
					if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, left) && !InRangeOfTumor(left)) {
						bot.Actions()->UnitCommand(creep_tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR, left);
					
						return;
					}

				}
				bot.active_creep_tumors_.erase(bot.active_creep_tumors_.begin() + count);
			
		}
	}




	//if (bot.GetActiveCreepCount() == 0) return;
	//const sc2::Unit* creep_queen_ = bot.Observation()->GetUnit(bot.creep_queen);
	//sc2::Units creep_tumors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED));
	//sc2::Point2D right;
	//sc2::Point2D left;
	//float pi = 3.14;
	//float circle_start_pos;
	//
	//
	//circle_start_pos = bot.enemy_angle;
	//bot.Debug()->DebugTextOut(std::to_string(circle_start_pos));
	///*if (bot.upORdown == 1) {
	//	 circle_start_pos = pi / 2;
	//}
	//else {
	//	 circle_start_pos = (float) 3 / (float) 2 * pi;
	//}*/

	//
	//bool spot_found = false;
	//float radius;
	//// todo: dodaj atan2 for dinamicly changing starting position for circle search // target enemy bases
	/*for (auto &tumor : creep_tumors) {*/
		//sc2::AvailableAbilities ability = bot.Query()->GetAbilitiesForUnit(tumor);
		//if (tumor->build_progress == 1 && ability.abilities.size() > 0) {
		//	for (radius = 8; radius > 1; radius = radius - 1) {
		//		for (float i = circle_start_pos, z = circle_start_pos, counter = 0; counter < 4; counter++, i = i + pi / 10, z = z - pi / 10) {

		//			right.x = tumor->pos.x + (radius * cos(z));
		//			right.y = tumor->pos.y + (radius * sin(z));
		//			const sc2::Unit* closest_right = FindNearestUnitToPos(right, sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED);
		//			/*std::cout << counter << " -> cos(angle)-> " << cos(z) << " sin(angle)-> " << sin(z) << std::endl;
		//			std::cout << counter << " -> angle-> " << i << " right smart-> " << "x,y->-------- " << right.x << "," << right.y << std::endl;*/
		//			sc2::Point3D debug_sphere;
		//			debug_sphere.x = right.x;
		//			debug_sphere.y = right.y;
		//			debug_sphere.z = tumor->pos.z;
		//			bot.Debug()->DebugSphereOut(debug_sphere, 2, sc2::Colors::Red);

		//			if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, right) && sc2::Distance2D(closest_right->pos, right) > 6) {
		//				bot.Actions()->UnitCommand(tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, right);
		//				spot_found = true;
		//				break;
		//			}

		//			left.x = tumor->pos.x + (radius * cos(i));
		//			left.y = tumor->pos.y + (radius * sin(i));
		//			const sc2::Unit* closest_left = FindNearestUnitToPos(left, sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED);

		//			/*std::cout << counter << " -> cos(angle)-> " << cos(i) << " sin(angle)-> " << sin(i) << std::endl;
		//			std::cout << counter << " -> angle-> " << z << "-> left smart-> " << " x,y->-------- " << left.x << "," << left.y << std::endl;*/


		//			debug_sphere.x = left.x;
		//			debug_sphere.y = left.y;
		//			debug_sphere.z = tumor->pos.z;
		//			bot.Debug()->DebugSphereOut(debug_sphere, 2, sc2::Colors::Red);

		//			if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, left) && sc2::Distance2D(closest_left->pos, left) > 6) {
		//				bot.Actions()->UnitCommand(tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, left);
		//				spot_found = true;
		//				break;
		//			}
		//		
		//		}
		//		if (spot_found) break;
		//	}
		//}

	//	sc2::AvailableAbilities ability2 = bot.Query()->GetAbilitiesForUnit(tumor);
	//	if (tumor->build_progress == 1 && ability2.abilities.size() > 0 ) {
	//		for (radius = 8; radius > 7; radius = radius - 1) {
	//			for (float i = circle_start_pos, z = circle_start_pos, counter = 0; counter < 10; counter++, i = i + pi / 10, z = z - pi / 10) {

	//				right.x = tumor->pos.x + (radius * cos(z));
	//				right.y = tumor->pos.y + (radius * sin(z));

	//			/*	std::cout << counter << "->angle->" << i << "->right normal->" << "x,y->" << right.x << "," << right.y << std::endl;*/
	//				if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, right)) {
	//					bot.Actions()->UnitCommand(tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR, right);
	//					spot_found = true;
	//					break;
	//				}

	//				left.x = tumor->pos.x + (radius * cos(i));
	//				left.y = tumor->pos.y + (radius * sin(i));

	//				/*std::cout << counter << "->angle->" << z << "->left normal->" << "x,y->" << left.x << "," << left.y << std::endl;*/
	//				if (bot.Query()->Placement(sc2::ABILITY_ID::BUILD_CREEPTUMOR_TUMOR, left)) {
	//					bot.Actions()->UnitCommand(tumor, sc2::ABILITY_ID::BUILD_CREEPTUMOR, left);
	//					spot_found = true;
	//					break;
	//				}

	//			}
	//			if (spot_found) break;
	//		}
	//	}
	//}
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = stop - start;
	if (bot.frame % 30 == 0)std::cout << "Economy manager -> creep spread : " << elapsed.count() << " s\n";
}

void EconomyManager::UpgradesManager()
{
	sc2::Units pools = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL));
	sc2::Units bane_nest = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_BANELINGNEST));
	sc2::Units lair = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_LAIR));
	sc2::Units spire = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_SPIRE));
	sc2::Units evos = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER));
	
	if (bot.Observation()->GetMinerals() > 150 && bot.Observation()->GetVespene() > 150 && bot.pool) {
		bot.Actions()->UnitCommand(pools.front(), sc2::ABILITY_ID::RESEARCH_ZERGLINGMETABOLICBOOST);
	}
	if (bot.Observation()->GetMinerals() > 150 && bot.Observation()->GetVespene() > 150  && bane_nest.size()>0 && lair.size()>0) {
		bot.Actions()->UnitCommand(bane_nest.front(), sc2::ABILITY_ID::RESEARCH_CENTRIFUGALHOOKS);
	}
	if (bot.Observation()->GetMinerals() > 150 && bot.Observation()->GetVespene() > 150 && bot.pool ) {
		bot.Actions()->UnitCommand(bot.main_base, sc2::ABILITY_ID::MORPH_LAIR);
	}
	if (bot.Observation()->GetMinerals() > 100 && bot.Observation()->GetVespene() > 100 && bot.pool && spire.size()>0 && spire.front()->orders.size() == 0 && spire.front()->build_progress == 1) {
		auto ab = bot.Query()->GetAbilitiesForUnit(spire.front(), true);
		if (!ab.abilities.empty()) {
			bot.Actions()->UnitCommand(spire.front(), ab.abilities.front().ability_id.ToType());
		}
	}
	for (auto &evo : evos) {
		if (evo->orders.empty() && evo->build_progress == 1) {
			auto ab = bot.Query()->GetAbilitiesForUnit(evo, true);
			if (!ab.abilities.empty()) {
				bot.Actions()->UnitCommand(evo, ab.abilities.front().ability_id.ToType());
			}
		}
	}
	
}



void EconomyManager::WorkerIdleManager(const sc2::Unit * unit)
{
	auto start = std::chrono::high_resolution_clock::now();
	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY,sc2::UNIT_TYPEID::ZERG_LAIR, sc2::UNIT_TYPEID::ZERG_HIVE }));
	sc2::Units extractors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));
	const sc2::Unit *mineral_near_base = nullptr;

	//First check for extractors in need of workers
	if (extractors.size() > 0) {
		for (const auto &extractor : extractors) {
			if (extractor->assigned_harvesters < extractor->ideal_harvesters) {					// add- find closest extractor in need of workers
				bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::HARVEST_GATHER, extractor);
				std::cout << "Gas Gather" << std::endl;
				return;
			}
		}
	}
	//Second check for bases in need of workers and find mineral node target close to base
	float d = 99999;
	const sc2::Unit * target = nullptr;
	for (const auto &base : bases) {
		if (base->assigned_harvesters < base->ideal_harvesters) {
			mineral_near_base = FindNearestMineralNode(base);
			float distance = sc2::DistanceSquared2D(mineral_near_base->pos, unit->pos);
			if (distance < d) {
				d = distance;
				target = mineral_near_base;
			}
		}
	}
	if (target != nullptr) {
		bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::HARVEST_GATHER, target);
		std::cout << "Mineral Gather" << std::endl;
		return;
	}

	//If there is no need for workers, just find closest mineral node and harvest

	mineral_near_base = FindNearestMineralNode(FindNearestBase(unit));
	bot.Actions()->UnitCommand(unit, sc2::ABILITY_ID::HARVEST_GATHER, mineral_near_base);
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = stop - start;
	if (bot.frame % 30 == 0)std::cout << "Economy manager -> worker idle : " << elapsed.count() << " s\n";
}

const sc2::Unit * EconomyManager::FindNearestUnitToPos(sc2::Point2D pos, sc2::UNIT_TYPEID unit_type)
{
	sc2::Units units_type = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(unit_type));
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &unit_t : units_type) {
		float d = sc2::DistanceSquared2D(unit_t->pos, pos);
		if (d < distance) {
			distance = d;
			target_node = unit_t;
		}
	}
	return target_node;
}


const sc2::Unit * EconomyManager::FindNearestMineralNode(const sc2::Unit* unit) const
{
	sc2::Units mineral_nodes = bot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)); 
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &mineral_node : mineral_nodes) {
		float d = sc2::DistanceSquared2D(mineral_node->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = mineral_node;
		}
	}
	return target_node;
}

const sc2::Unit * EconomyManager::FindNearestBase(const sc2::Unit* unit) const
{
	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY, sc2::UNIT_TYPEID::ZERG_LAIR, sc2::UNIT_TYPEID::ZERG_HIVE }));
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &base : bases) {
		float d = sc2::DistanceSquared2D(base->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = base;
		}
	}
	return target_node;
}

const sc2::Unit * EconomyManager::FindNearestWorker(const sc2::Unit* unit)
{
	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &worker : workers) {
		float d = sc2::DistanceSquared2D(worker->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = worker;
		}
	}
	return target_node;
}

const sc2::Unit * EconomyManager::FindNearestMineralCarrier(const sc2::Unit * unit)
{
	sc2::Units mineral_carriers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsCarryingMinerals);
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target_node = nullptr;

	for (const auto &carrier : mineral_carriers) {
		float d = sc2::DistanceSquared2D(carrier->pos, unit->pos);
		if (d < distance) {
			distance = d;
			target_node = carrier;
		}
	}
	return target_node;
}

const sc2::Unit * EconomyManager::FindNonOptimalMiningLoc()
{
	sc2::Units bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_HATCHERY));
	sc2::Units extractors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));

	for (auto & extr : extractors) {
		if (extr->assigned_harvesters < extr->ideal_harvesters) return extr;
	}
	for (auto & base : bases) {
		if (base->assigned_harvesters < base->ideal_harvesters) return FindNearestMineralNode(base);
	}
	return nullptr;
}

bool EconomyManager::InRangeOfTumor(sc2::Point2D point)
{
	sc2::Units all_tumors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED));
	sc2::Units all_tumors_hatching = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_CREEPTUMOR));
	for (auto & tumor : all_tumors) {
		if (sc2::Distance2D(tumor->pos, point) < 6) return true;
	}
	for (auto & tumor : all_tumors_hatching) {
		if (sc2::Distance2D(tumor->pos, point) < 6) return true;
	}
	return false;
}

float EconomyManager::GetFinishedBasesCount() const
{
	auto bases = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnits({ sc2::UNIT_TYPEID::ZERG_HATCHERY,sc2::UNIT_TYPEID::ZERG_LAIR, sc2::UNIT_TYPEID::ZERG_HIVE }));
	float count = 0;
	for (auto & base : bases) {
		if (base->build_progress < 1 && base->unit_type.ToType() == sc2::UNIT_TYPEID::ZERG_HATCHERY) continue;
		count++;
	}
	return count;
}

float EconomyManager::GetWorkerCount() const
{
	sc2::Units workers = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_DRONE));
	sc2::Units eggs = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EGG));


	float workers_count = workers.size() + AlreadyTrainingCount(eggs, sc2::ABILITY_ID::TRAIN_DRONE);
	return workers_count;
}

float EconomyManager::GetActiveGasCount() const
{
	int count = 0;
	sc2::Units extractors = bot.Observation()->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::ZERG_EXTRACTOR));
	for (auto & unit : extractors) {
		if (unit->vespene_contents > 0 || unit->build_progress < 1) count++;
	}
	return count;
}


bool EconomyManager::UnitIsInRadius(const sc2::Unit * unit, const sc2::Unit * unit2)
{
	float radius = unit->radius + 10;
	if (sc2::Distance2D(unit->pos, unit2->pos) < radius) return true;
	return false;
}

bool EconomyManager::AlreadyTraining(const sc2::Units &units, sc2::ABILITY_ID ability)
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

size_t EconomyManager::AlreadyTrainingCount(const sc2::Units units, sc2::ABILITY_ID ability) const
{
	size_t count = 0;
	for (auto &unit : units) {
		for (auto &order : unit->orders) {
			if (order.ability_id.ToType() == ability) count++;
		}
	}
	return count;
}

bool EconomyManager::WorkersNeeded(sc2::Units & units, sc2::Units & units2)
{
	
	for (auto &unit : units) {
		if (unit->assigned_harvesters < unit->ideal_harvesters && unit->build_progress==1) return true;
	}
	for (auto &unit2 : units2) {
		if (unit2->assigned_harvesters < unit2->ideal_harvesters && unit2->build_progress == 1) return true;
	}
	return false;
}



