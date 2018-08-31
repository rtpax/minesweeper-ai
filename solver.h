#ifndef MS_SOLVER_H
#define MS_SOLVER_H

#include <vector>
#include <set>
#include <list>
#include <cassert>
#include "grid.h"
#include "region.h"
#include "region_set.h"

/**
 * 
 * All classes and functions for the sweeper application are contained in this namespace. 
 * 
 **/
namespace ms {

	class spinoff;

	/**
	 * The `solver` class is used to solve minesweeper games.
	 * 
	 * It can use a grid of any size, and can either use a pre-made grid or generate one itself.
	 * 
	 * AI may be used in tandem with user controls
	 **/
	class solver {
	public:
		solver(const solver& copy, grid::copy_type gct);
		solver(const grid& g, grid::copy_type gct = grid::FULL_COPY);
		solver(unsigned int height, unsigned int width, unsigned int bombs);
		
		int solve();
		rc_coord step();
		int solve_certain();
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
		static std::mt19937 rng;

		grid g;
		region_set regions;
		std::unordered_set<rc_coord, rc_coord_hash> safe_queue;
		std::unordered_set<rc_coord, rc_coord_hash> bomb_queue;

		int remove_safe(rc_coord cell);
		int remove_bomb(rc_coord cell);
		rc_coord get_safe_from_queue() const;
		rc_coord get_bomb_from_queue() const;

		int clear_queue();
		int apply_open(rc_coord cell);
		int apply_flag(rc_coord cell);
		
		region approx_remain() const;
		float expected_payout(rc_coord cell) const;

		int trim_regions();

		int find_regions();
		int find_base_regions();
		int find_aux_regions(bool lazy);

		int fill_queue();
		int add_to_safe_queue(rc_coord to_add);
		int add_to_bomb_queue(rc_coord to_add);

		int cleanup();
	};



}


#endif