#pragma once
class ZZerg;

class BuildingManager
{
	ZZerg &bot;
public:
	BuildingManager(ZZerg &bot);
	~BuildingManager();

	void BuildingManagerStep();

	bool TryBuildStructure(sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Point2D location, bool isExpansion = false);
	bool TryBuildStructure( sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Tag location_tag);
	bool TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type);
	bool TryBuildGas(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type, sc2::Point2D base_location);
	bool TryBuildSpawningPool();
	bool TryBuildBaneling();
	bool TryBuildSpire();

	bool OrderInProgress( sc2::AbilityID ability_id);  // add argument (Unit*), make it general
};

