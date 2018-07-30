
#include "ZZerg.h"
#include "Utility.h"

bool Utility::IsArmy(const sc2::Unit & unit,ZZerg &bot)
{
	auto attributes = bot.Observation()->GetUnitTypeData().at(unit.unit_type).attributes;
	for (const auto& attribute : attributes) {
		if (attribute == sc2::Attribute::Structure) {
			return false;
		}
	}
	switch (unit.unit_type.ToType()) {
	case sc2::UNIT_TYPEID::ZERG_OVERLORD: return false;
	case sc2::UNIT_TYPEID::ZERG_OVERSEER: return false;
	case sc2::UNIT_TYPEID::PROTOSS_PROBE: return false;
	case sc2::UNIT_TYPEID::ZERG_DRONE: return false;
	case sc2::UNIT_TYPEID::TERRAN_SCV: return false;
	case sc2::UNIT_TYPEID::ZERG_QUEEN: return false;
	case sc2::UNIT_TYPEID::ZERG_LARVA: return false;
	case sc2::UNIT_TYPEID::ZERG_EGG: return false;
	case sc2::UNIT_TYPEID::TERRAN_MULE: return false;
	case sc2::UNIT_TYPEID::TERRAN_NUKE: return false;
	default: return true;
	}
}

bool Utility::IsStructure(const sc2::Unit & unit, ZZerg & bot)
{
	auto& attributes = bot.Observation()->GetUnitTypeData().at(unit.unit_type).attributes;
	bool is_structure = false;
	for (const auto& attribute : attributes) {
		if (attribute == sc2::Attribute::Structure) {
			is_structure = true;
		}
	}
	return is_structure;
}
bool Utility::IsTownHall(const sc2::Unit & unit)
{
	switch (unit.unit_type.ToType()) {
		case sc2::UNIT_TYPEID::ZERG_HATCHERY: return true;
		case sc2::UNIT_TYPEID::ZERG_LAIR: return true;
		case sc2::UNIT_TYPEID::ZERG_HIVE: return true;
		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER: return true;
		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND: return true;
		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING: return true;
		case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: return true;
		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
		default: return false;
	}
}



bool Utility::IsCombatUnit::operator()(const sc2::Unit& unit_) const {
	switch (unit_.unit_type.ToType()) {
	case sc2::UNIT_TYPEID::TERRAN_BANSHEE:
	case sc2::UNIT_TYPEID::TERRAN_CYCLONE:
	case sc2::UNIT_TYPEID::TERRAN_GHOST:
	case sc2::UNIT_TYPEID::TERRAN_HELLION:
	case sc2::UNIT_TYPEID::TERRAN_HELLIONTANK:
	case sc2::UNIT_TYPEID::TERRAN_LIBERATOR:
	case sc2::UNIT_TYPEID::TERRAN_LIBERATORAG:
	case sc2::UNIT_TYPEID::TERRAN_MARAUDER:
	case sc2::UNIT_TYPEID::TERRAN_MARINE:
	case sc2::UNIT_TYPEID::TERRAN_MEDIVAC:
	case sc2::UNIT_TYPEID::TERRAN_RAVEN:
	case sc2::UNIT_TYPEID::TERRAN_REAPER:
	case sc2::UNIT_TYPEID::TERRAN_SIEGETANK:
	case sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED:
	case sc2::UNIT_TYPEID::TERRAN_THOR:
	case sc2::UNIT_TYPEID::TERRAN_THORAP:
	case sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT:
	case sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER:
	case sc2::UNIT_TYPEID::TERRAN_WIDOWMINE:
	case sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED:

	case sc2::UNIT_TYPEID::ZERG_BANELING:
	case sc2::UNIT_TYPEID::ZERG_BANELINGBURROWED:
	case sc2::UNIT_TYPEID::ZERG_BROODLORD:
	case sc2::UNIT_TYPEID::ZERG_CORRUPTOR:
	case sc2::UNIT_TYPEID::ZERG_HYDRALISK:
	case sc2::UNIT_TYPEID::ZERG_HYDRALISKBURROWED:
	case sc2::UNIT_TYPEID::ZERG_INFESTOR:
	case sc2::UNIT_TYPEID::ZERG_INFESTORBURROWED:
	case sc2::UNIT_TYPEID::ZERG_INFESTORTERRAN:
	case sc2::UNIT_TYPEID::ZERG_LURKERMP:
	case sc2::UNIT_TYPEID::ZERG_LURKERMPBURROWED:
	case sc2::UNIT_TYPEID::ZERG_MUTALISK:
	case sc2::UNIT_TYPEID::ZERG_RAVAGER:
	case sc2::UNIT_TYPEID::ZERG_ROACH:
	case sc2::UNIT_TYPEID::ZERG_ROACHBURROWED:
	case sc2::UNIT_TYPEID::ZERG_ULTRALISK:
	case sc2::UNIT_TYPEID::ZERG_VIPER:
	case sc2::UNIT_TYPEID::ZERG_ZERGLING:
	case sc2::UNIT_TYPEID::ZERG_ZERGLINGBURROWED:

	case sc2::UNIT_TYPEID::PROTOSS_ADEPT:
	case sc2::UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT:
	case sc2::UNIT_TYPEID::PROTOSS_ARCHON:
	case sc2::UNIT_TYPEID::PROTOSS_CARRIER:
	case sc2::UNIT_TYPEID::PROTOSS_COLOSSUS:
	case sc2::UNIT_TYPEID::PROTOSS_DARKTEMPLAR:
	case sc2::UNIT_TYPEID::PROTOSS_DISRUPTOR:
	case sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED:
	case sc2::UNIT_TYPEID::PROTOSS_HIGHTEMPLAR:
	case sc2::UNIT_TYPEID::PROTOSS_IMMORTAL:
	case sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP:
	case sc2::UNIT_TYPEID::PROTOSS_ORACLE:
	case sc2::UNIT_TYPEID::PROTOSS_PHOENIX:
	case sc2::UNIT_TYPEID::PROTOSS_SENTRY:
	case sc2::UNIT_TYPEID::PROTOSS_STALKER:
	case sc2::UNIT_TYPEID::PROTOSS_TEMPEST:
	case sc2::UNIT_TYPEID::PROTOSS_VOIDRAY:
	case sc2::UNIT_TYPEID::PROTOSS_ZEALOT:
		return true;

	default:
		return false;
	}
}
//
//
//bool Utility::IsAttackable::operator()(const sc2::Unit& unit) {
//		switch (unit.unit_type.ToType()) {
//			case sc2::UNIT_TYPEID::ZERG_OVERLORD: return false;
//			case sc2::UNIT_TYPEID::ZERG_OVERSEER: return false;
//			case sc2::UNIT_TYPEID::PROTOSS_OBSERVER: return false;
//			default: return true;
//		}
//	}
//
//bool Utility::IsFlying::operator()(const sc2::Unit & unit)
//{
//	return unit.is_flying;
//}
//
//bool Utility::IsTownHall::operator()(const sc2::Unit & unit)
//{
//	switch (unit.unit_type.ToType()) {
//		case sc2::UNIT_TYPEID::ZERG_HATCHERY: return true;
//		case sc2::UNIT_TYPEID::ZERG_LAIR: return true;
//		case sc2::UNIT_TYPEID::ZERG_HIVE: return true;
//		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER: return true;
//		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND: return true;
//		case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING: return true;
//		case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: return true;
//		case sc2::UNIT_TYPEID::PROTOSS_NEXUS: return true;
//		default: return false;
//	}
//}
//
//bool Utility::IsVespeneGyser::operator()(const sc2::Unit& unit) {
//	switch (unit.unit_type.ToType()) {
//		case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
//		case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
//		case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
//		case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
//		case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
//		case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
//		default: return false;
//	}
//}
//
//
//
//Utility::IsStructure::IsStructure(const sc2::ObservationInterface* obs) : obs_(obs) {};
//
//bool Utility::IsStructure::operator()(const sc2::Unit& unit) {
//	auto& attributes = obs_->GetUnitTypeData().at(unit.unit_type).attributes;
//	bool is_structure = false;
//	for (const auto& attribute : attributes) {
//		if (attribute == sc2::Attribute::Structure) {
//				is_structure = true;
//		}
//	}
//	return is_structure;
//}






