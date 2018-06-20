#ifndef MS_SOLVER_H
#define MS_SOLVER_H

#include <vector>
#include <list>
#include <assert.h>
#include "grid.h"
#include "region.h"

/**
 * 
 * All classes and functions for the sweeper application are contained in this namespace. 
 * 
 **/
namespace ms {

	class spinoff;

	/**
	 * 
	 * The `solver` class is used to solve minesweeper games.
	 * 
	 * It can use a grid of any size, and can either use a pre-made grid or generate one itself.
	 * 
	 * It may be used in tandem with user controls
	 * 
	 * 
	 * 
	 * 
	 **/
	class solver {
	public:
		solver(const grid& g);
		solver(unsigned int height, unsigned int width, unsigned int bombs);
		
		/**Runs until win or loss. \n Returns gamestate.*/
		int solve();
		/**Takes one turn.\n Returns the cell opened/flagged.*/
		rc_coord step();
		/**Runs until there is a chance it may fail (does nothing if board is not started). \n Returns the number of steps taken.*/
		int solve_certain();
		/**Steps iff the solver is certain it will not fail (does nothing if the board is not started). \n Returns the cell opened/flagged, or { 0xffff,0xffff } if it does not open a cell*/
		rc_coord step_certain();

		int manual_open(rc_coord cell);
		int manual_flag(rc_coord cell);

		/**Returns the internal `grid`'s gamestate*/
		int gamestate() { return g.gamestate(); }

		/**Returns a copy of the internal grid. Only copied on surface, so cannot be used to copy the game*/
		grid view_grid() const { return grid(g, grid::SURFACE_COPY); } 
		/**Returns a complete copy of the games internal grid.*/
		grid get_grid() const { return grid(g, grid::FULL_COPY); } 
	protected:
		std::list<region> regions;
		std::list<rc_coord> safe_queue;
		std::list<rc_coord> bomb_queue;

		int clear_queue();
		int apply_open(rc_coord cell);
		int apply_flag(rc_coord cell);
		
		grid g;

		int add_region(const region& arg);
		int find_in_regions();

		int trim_regions();

		int find_regions();
		int find_base_regions();
		int find_aux_regions();
		int find_ext_aux_regions();
		int find_chains();
		int find_leftover();

		int fill_queue();
		int add_to_safe_queue(rc_coord to_add);
		int add_to_bomb_queue(rc_coord to_add);
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


	};



}


#endif