#include "ZZerg.h"
#include "Utility.h"





EnemyInfo::EnemyInfo(ZZerg & bot) : bot(bot)
{
}

EnemyInfo::~EnemyInfo()
{
}



void EnemyInfo::EnemyInfoInit()
{
	auto unit_types=bot.Observation()->GetUnitTypeData();
	for (auto &type : unit_types) {
		enemy_unit_count[type.unit_type_id] = 0;
	}
}


void EnemyInfo::EnemyInfoAll()
{
	UpdateEnemyInfo();
}

void EnemyInfo::UpdateEnemyInfo()
{
	
	sc2::Units enemy_units_obs = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy);
	if (enemy_units_obs.size() == 0) return;

	for (auto &enemy_unit_obs : enemy_units_obs) {
		bool unit_seen = false;
		if (Utility::IsArmy(*enemy_unit_obs, bot) && enemy_unit_obs->display_type == sc2::Unit::DisplayType::Visible ) {
			for (auto &enemy_unit : enemy_army_) {
				if (enemy_unit.tag == enemy_unit_obs->tag) {
					unit_seen = true;
					break;
				}
			}
			if (!unit_seen) {
				enemy_army_.push_back(*enemy_unit_obs);
				enemy_unit_count[enemy_unit_obs->unit_type]++;
				continue;
			}
		}

		if (Utility::IsStructure(*enemy_unit_obs, bot) && enemy_unit_obs->display_type == sc2::Unit::DisplayType::Visible) {
			for (auto &enemy_building : enemy_buildings) {
				if (enemy_building.tag == enemy_unit_obs->tag) {
					unit_seen = true;
					break;
				}
			}
			if (!unit_seen) {
				if (Utility::IsTownHall(*enemy_unit_obs)) {
					enemy_bases_.push_back(*enemy_unit_obs);
				}
				enemy_buildings.push_back(*enemy_unit_obs);
				enemy_unit_count[enemy_unit_obs->unit_type]++;
				continue;
			}
		}

	}
	
}

void EnemyInfo::EnemyUnitDestroyed(const sc2::Unit & unit)
{
	if (unit.alliance != sc2::Unit::Alliance::Enemy) return;
	enemy_unit_count[unit.unit_type]--;

	if (Utility::IsArmy(unit, bot)) {
		enemy_army_.erase(std::remove_if(enemy_army_.begin(), enemy_army_.end(),
			[&](sc2::Unit & enemy_unit) { return unit.tag == enemy_unit.tag;}),
			enemy_army_.end());
			return;
	}
	if (Utility::IsStructure(unit, bot)) {
		if (Utility::IsTownHall(unit)) {
			enemy_bases_.erase(std::remove_if(enemy_bases_.begin(), enemy_bases_.end(),
				[&](sc2::Unit & enemy_unit) { return unit.tag == enemy_unit.tag; }), enemy_bases_.end());
			return;
		}
		enemy_buildings.erase(std::remove_if(enemy_buildings.begin(), enemy_buildings.end(),
			[&](sc2::Unit & enemy_unit) { return unit.tag == enemy_unit.tag; }), enemy_buildings.end());
			return;	
		
	}
	
}

bool EnemyInfo::UnitAlreadySeen(const sc2::Unit *unit, sc2::Units &units)
{
	for (auto & enemy_unit : units) {
		if (unit->tag == enemy_unit->tag) {
			return true;
		}
	}
	return false;
}

float EnemyInfo::StaticDefCount(sc2::Unit & unit)
{
	float count = 0;
	sc2::Units enemy_units = bot.Observation()->GetUnits(sc2::Unit::Alliance::Enemy, sc2::IsUnits({sc2::UNIT_TYPEID::TERRAN_BUNKER,sc2::UNIT_TYPEID::TERRAN_MISSILETURRET,sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON,sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY, sc2::UNIT_TYPEID::ZERG_SPINECRAWLER,sc2::UNIT_TYPEID::ZERG_SPORECRAWLER }));
	for (auto & enemy_unit : enemy_units) {
		if (sc2::Distance2D(enemy_unit->pos, unit.pos) < 10) {
			count++;
		}
	}
	return count;
}

float EnemyInfo::GetEnemySupply()
{
	return 0;
}

float EnemyInfo::GetEnemyArmySupp()const
{
	float supply = 0;
	for (auto enemy_unit : enemy_army_) {
		auto food_req = bot.Observation()->GetUnitTypeData().at(enemy_unit.unit_type).food_required;
			supply = supply + food_req;
	}
	return supply;
}
