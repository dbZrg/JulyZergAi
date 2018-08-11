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
	bool TryBuildStructure( sc2::AbilityID ability_type_for_structure, sc2::UnitTypeID unit_type, sc2::Tag location_tag,bool check_if_building);
	bool TryExpand(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type);
	bool TryBuildGas(sc2::AbilityID build_ability, sc2::UnitTypeID worker_type, sc2::Point2D base_location,bool check_if_building);
	bool TryBuildSpawningPool();
	bool TryBuildBaneling();
	bool TryBuildSpire();
	bool TryBuildSpore(const sc2::Unit *base);
	bool TryBuildEvo();

	bool OrderInProgress( sc2::AbilityID ability_id);  
	size_t OrderCount(sc2::AbilityID ability_id);

	std::pair<size_t,size_t> GetStaticDefCount(const sc2::Unit *unit);

	bool save_minerals;
};

