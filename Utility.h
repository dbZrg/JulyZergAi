#pragma once

class ZZerg;

namespace Utility {
	
	bool IsArmy(const sc2::Unit &unit,ZZerg &bot);
	bool IsStructure(const sc2::Unit &unit, ZZerg &bot);
	bool IsTownHall(const sc2::Unit & unit);

	struct IsCombatUnit
	{
		bool operator()(const sc2::Unit &unit) const;
		
	};
	/*
	struct IsAttackable {
		bool operator()(const sc2::Unit& unit);
	};

	struct IsFlying {
		bool operator() (const sc2::Unit & unit);
	};

	struct IsTownHall {
		bool operator()(const sc2::Unit& unit);
	};

	struct IsVespeneGyser {
		bool operator() (const sc2::Unit &unit);
	};

	struct IsStructure {
		IsStructure(const sc2::ObservationInterface * obs);
		bool operator()(const sc2::Unit &unit);
		const sc2::ObservationInterface * obs_;
	};
*/
};


