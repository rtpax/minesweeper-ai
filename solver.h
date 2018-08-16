#ifndef MS_SOLVER_H
#define MS_SOLVER_H

#include <vector>
#include <deque>
#include <set>
#include <list>
#include <cassert>
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
	 * The `solver` class is used to solve minesweeper games.
	 * 
	 * It can use a grid of any size, and can either use a pre-made grid or generate one itself.
	 * 
	 * AI may be used in tandem with user controls
	 **/
	class solver {
	public:
		solver(const grid& g);
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

		std::set<region> regions;
		std::deque<rc_coord> safe_queue;
		std::deque<rc_coord> bomb_queue;

		typedef std::set<region>::iterator regions_iter;

		struct key_less {
			bool operator()(regions_iter it1, regions_iter it2) { return *it1 < *it2; }
		};

		typedef std::set<regions_iter, key_less> key_type;
		typedef key_type::iterator key_iter;

		key_type ** cell_keys;

		int init_cell_keys();
		regions_iter remove_region(regions_iter to_remove);
		std::pair<regions_iter, bool> add_region(const region& to_add);
		int remove_safe_from_all_regions(rc_coord cell);
		int remove_bomb_from_all_regions(rc_coord cell);

		int clear_queue();
		int apply_open(rc_coord cell);
		int apply_flag(rc_coord cell);
		
		grid g;


		int trim_regions();

		int find_regions();
		int find_base_regions();
		int find_aux_regions(bool lazy);
		int find_ext_aux_regions();
		int find_chains();
		int find_leftover();

		int fill_queue();
		int add_to_safe_queue(rc_coord to_add);
		int add_to_bomb_queue(rc_coord to_add);

		int cleanup();
	};



}


#endif