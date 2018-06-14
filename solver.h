#ifndef MS_SOLVER_H
#define MS_SOLVER_H

#include <vector>
#include <list>
#include <assert.h>
#include "grid.h"
#include "region.h"

namespace ms {

	class spinoff;

	class solver {
	public:
		solver(const grid& g);
		solver(unsigned int height, unsigned int width, unsigned int bombs);
		
		int solve();//runs until win or loss, returns gamestate
		int step();//takes one turn, returns gamestate
		int solve_certain();//runs until there is a chance it may fail (does nothing if board is not started)
		int step_certain();//steps iff it will not fail

		int manual_open();
		int manual_flag();

		const grid get_grid() { return grid(g, SURFACE_COPY); } //only on surface, so cannot be used to copy the game

	protected:
		std::list<region> regions;
		std::list<rc_coord> safe_queue;
		std::list<rc_coord> bomb_queue;

		int clear_queue();
		
		grid g;

		int add_region(const region& arg);
		int find_in_regions();

		int trim_regions();

		int find_regions();
		int find_base_regions();
		int find_aux_regions();
		int find_chains();
		int find_leftover();
		int remove_safe_from_all_regions(rc_coord cell);
		int remove_bomb_from_all_regions(rc_coord cell);

		std::vector<std::vector<region>> chains;

		std::list<region> congolomerates;
		int find_conglomerate();
		int add_solo_regions();
		std::vector<region> find_best_starting_points();
		int work_from_starting_point(region& conglomerate, const region& start);

		int find_guarantees();
		int cleanup();

		int assert_each_trim();
		int assert_norepeat();


	};



}


#endif