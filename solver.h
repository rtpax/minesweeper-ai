#ifndef MS_SOLVER_H
#define MS_SOLVER_H

#include <vector>
#include <assert.h>
#include "grid.h"
#include "region.h"

namespace ms {

	class spinoff;

	class solver {
	public:
		solver(grid * g);
		int solve();//runs until win or loss, returns gamestate
		int step();//takes one turn, returns gamestate
	protected:
		
		std::vector<region> regions;
		std::vector<rc_coord> safe_queue;
		std::vector<rc_coord> bomb_queue;
		std::vector<region> conglomerates;
		grid * g;

		int add_region(const region& arg);
		int find_in_regions();

		int find_regions();
		int find_base_regions();
		int find_aux_regions();
		int trim_regions();
		int leftover_region();
		int find_regions_cleanup();
		int remove_from_all_regions(rc_coord cell);

		std::vector<region> congolomerates;
		int find_conglomerate();
		int add_solo_regions();
		std::vector<region> find_best_starting_points(const region& conglomerate);
		int work_from_starting_point(region& conglomerate, const region& start);

		int find_guarantees();
		int cleanup();

#ifdef DEBUG
		int assert_each_trim() {
			for(unsigned int i = 0; i < regions.size(); ++i) {
				regions[i].assert_trim();
			}
			return 0;
		}
#else
		int assert_each_trim(){ return 0; }
#endif
	};



}


#endif