#include "solver.h"
#include <stdexcept>

#if DEBUG>1
	#define debug_printf(...) printf(__VA_ARGS__)
#else
	#define debug_printf(...)
#endif

namespace ms {


	/**\internal
	 * 
	 * Namespace for organizing globals used by class implementations.
	 * 
	 **/
	namespace util {

		/****/
		typedef std::list<region>::iterator literator;

		/**Allows adding to r-value iterators that do not support random access**/
		literator it_add(literator base, int amount) {
			for(int i = 0; i < amount; ++i)
				++base;
			for(int i = 0; i > amount; --i)
				--base;
			return base;
		}

		/**
		 * 
		 * Checks if the cell gives any useful information about the number of bombs.
		 * 
		 * Returns false if min/max can be inferred from size, true if they cannot
		 * 
		 **/
		bool region_is_helpful(const region& check) {
			return !(check.size() == check.max() && check.min() == 0);//size == 0 evaluates to false for valid regions
		}

		/**Print information about rc_coord iff debug printing is enabled**/
		void debug_print_rc_coord(rc_coord& arg) {
			debug_printf("(%u,%u)",arg.row,arg.col);
		}
		
		/**Print information about region iff debug printing is enabled**/
		void debug_print_region(region& arg) {
			debug_printf("{[%zu:%u,%u]",arg.size(),arg.min(),arg.max());
			for(rc_coord rc : arg) {
				debug_print_rc_coord(rc);
			}
			debug_printf("}");
		}

		/**Indicates a function is a pair suitable for a chain**/
		bool ispair(const region& check) {
			return check.size() == 2 && check.min() == 1 && check.max() == 1;
		}

		/**Returns true if check overlaps any of the regions in chain**/
		bool hasoverlap(const std::vector<region>& chain, const region& check) {
			for(const region& reg : chain) {
				if(check.has_intersect(reg))
					return true;
			}
			return false;
		}

		/**
		 * 
		 * Returns a vector containing all regions in a chain that starts at arg.front()
		 * 
		 * All regions in arg that are put into chains are removed from arg
		 * 
		 * \note This causes a lasting change in the input
		 * 
		 **/
		std::vector<region> one_chain(std::list<region>& arg) {
			bool found = true;
			std::vector<region> chain;

			while(found) {
				found = false;
				for(literator iter = arg.begin(); iter != arg.end();) {
					if(ispair(*iter) && (hasoverlap(chain,*iter) || chain.empty())) {
						chain.push_back(*iter);
						iter = arg.erase(iter);
						found = 1;
					} else {
						++iter;
					}
				}
			}

			return chain;
		}

		/**
		 *
		 * Adds a region to the list of regions, merging if a region already exists covering the same area.
		 * Skips regions that offer no information
		 * 
		 * Returns 0 if merging occurs, otherwise returns 1. 
		 *
		 * Complexity \f$O(N)\f$
		 * 
		 **/
		int merge_region_into_list(std::list<region>& list, const region& to_add) {
			if(!region_is_helpful(to_add))
				return 0;
			for (literator li = list.begin(); li != list.end(); ++li) {
				if (to_add.samearea(*li)) {
					*li = (*li).merge(to_add);
					assert_nonempty(*li);
					return 0;
				}
			}
			list.push_back(to_add);
			return 1;
		}


	}
	using namespace util;

#ifdef DEBUG
	#define assert_each_trim() do {\
		for(literator ri = regions.begin(); ri != regions.end(); ++ri) {\
			assert_trim(*ri);\
		}\
	}while(0)
	#define assert_norepeat() do {\
		for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {\
			for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {\
				assert((*ri) != *rj);\
			}\
		}\
	}while(0)
#else
	#define assert_each_trim()
	#define assert_norepeat()
#endif


	/**
	 *
	 * Copies grid, all other members default initialize
	 *
	 **/
	solver::solver(const grid& start) : g(start, grid::FULL_COPY) {
		
	}

	/**
	 * 
	 * Initializes the internal grid with the given parameters
	 * 
	 **/
	solver::solver(unsigned int height, unsigned int width, unsigned int bombs) : g(height,width,bombs) {
		
	}






	/**
	 *
	 * Find the areas around each number where there could be bombs.
	 * Such places must fit the following criteria:
	 *   - on the grid
	 *   - not not flagged or opened (so are hidden or questionmarked)
	 *   - adjacent to the given square
	 *
	 * Determine in each case how many bombs there are.
	 * This will be exact, so `region.min == region.max` for all regions generated.
	 *
	 * Store the results in `this.regions`
	 * 
	 * Guaranteed to create trim regions, not cause two regions of the same area to be added, and not cause empty regions to be added.
	 * 
	 * Complexity \f$O(N^2)\f$
	 *
	 **/
	int solver::find_base_regions() {
		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				grid::cell gotten = g.get(r,c);
				if(gotten <= 8 && gotten >= 0) {//we could exclude zero to save time, grid should deal with those automatically
					region reg;
					int num_flags = 0;

					for (int rr = -1; rr <= 1; ++rr)
						for (int cc = -1; cc <= 1; ++cc)
							if (!(rr == 0 && cc == 0) && g.iscontained(r + rr, c + cc)) {
								switch (g.get(r + rr, c + cc)) {
								case grid::ms_hidden:
								case grid::ms_question:
									reg.addcell(rc_coord(r + rr, c + cc));
									break;
								case grid::ms_flag:
									++num_flags;
									break;
								default:
									break;
								}

							}
					if(!reg.empty()) {
						reg.set_count(gotten - num_flags);
						add_region(reg);
					}
				}
			}
		}

		return 0;
	}

	/**
	 *
	 * Find all regions that can be deduced from the existing regions.
	 * 
	 * Calculate the intersection of each pair of regions
	 *   - if empty, discard it
	 *   - if nonempty, add the intersection and subraction to `this.aux_regions`
	 * 
	 * Returns the number of regions added (note that it does not include modified regions, only added)
	 * 
	 * Complexity of the inner loop upper bound \f$O(N^4)\f$, lower bound \f$\Omega (N^2)\f$.
	 * Outer loop's complexity is nontrivial, runs until it cannot make any more aux regions.
	 * 
	 **/
	int solver::find_aux_regions() {
		debug_printf("find_aux_regions...");
		trim_regions();
		int size = regions.size();


		for (bool made_change = 1; made_change;) { //loops as long as nothing was added
			made_change = 0;
			std::list<region> region_queue;

			if(!regions.empty())//otherwise the conditional will never be false
			for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
				for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {
					region _inter = (*rj).intersect(*ri);
					if (_inter.size() > 0) {
						region _subij = (*ri).subtract(*rj);
						region _subji = (*rj).subtract(*ri);
						region_queue.push_back(_inter);
						region_queue.push_back(_subij);
						region_queue.push_back(_subji);
					}
				}
			}

			for(region& to_add : region_queue) {
				if(add_region(to_add)) {
					made_change = 1;
				}
			}

		}
		assert_each_trim();
		assert_norepeat();

		debug_printf("done\n");
		return regions.size() - size;
	}

	/**
	 *
	 * Removes all empty regions and regions that give no information about min max beyong their size (min = 0, max = size).
	 * Merges regions that have the same number of cells.
	 * 
	 * Returns the number of regions removed (2 being merged counts as 1 being removed)
	 * 
	 * complexity \f$O(N^2)\f$
	 *
	 **/
	int solver::trim_regions() {
		region zero;

		size_t initial_size = regions.size();

		for (literator ri = regions.begin(); ri != regions.end();) {
			if (region_is_helpful(*ri)) {
				if(!(*ri).is_trim()) {

				}
				(*ri).trim(); //can never make a size 0 from nonzero
				++ri;
			}
			else {
				ri = regions.erase(ri);
			}
		}
		if(!regions.empty())//otherwise the conditional will never be false
		for(literator ri = regions.begin(); it_add(ri, 1) != regions.end() && ri != regions.end(); ++ri) {
			for(literator rj = it_add(ri, 1); rj != regions.end();) {
				if ((*ri).samearea(*rj)) {
					*ri = (*ri).merge(*rj);
					rj = regions.erase(rj);
				}
				else {
					++rj;
				}
			}
		}

		return initial_size - regions.size();
	}

	/**
	 *
	 * Adds a region to the list of regions, merging if a region already exists covering the same area.
	 * Skips regions that offer no information
	 * 
	 * Returns 0 if merging occurs, otherwise returns 1. 
	 *
	 * Complexity \f$O(N)\f$
	 * 
	 **/
	int solver::add_region(const region& arg) {
		return merge_region_into_list(regions,arg);
	}

	/**
	 *
	 * Adds a region to the list of conglomerates, merging if a region already exists covering the same area.
	 * Skips regions that offer no information
	 * 
	 * Returns 0 if merging occurs, otherwise returns 1. 
	 *
	 * Complexity \f$O(N)\f$
	 * 
	 **/
	int solver::add_conglomerate(const region& arg) {
		return merge_region_into_list(regions,arg);
	}


	/**
	 *
	 * a chain is a series of regions that both:
	 *   - have exactly 1 bomb
	 *   - have exactly one cell that does not overlap
	 * 
	 * no two complete chains overlap
	 * 
	 **/
	int solver::find_chains() {
		std::list<region> unchecked_regions;
		chains.clear();

		for(region& r : regions) {
			unchecked_regions.push_back(r);
		}

		while(!unchecked_regions.empty()) {
			if(ispair(unchecked_regions.front())) {
				chains.push_back(one_chain(unchecked_regions));
			} else {
				unchecked_regions.pop_front();
			}
		}

		return 0;
	}

	/**
	 *
	 *
	 *
	 *
	 **/
	int solver::find_conglomerate(){
		debug_printf("finding_conglomerate...");

		std::list<region> conglomerates;

		for (bool made_change = 0; made_change;) {
			made_change = 0;
			std::list<region> region_queue;

			if(!regions.empty())//otherwise the conditional will never be false
			for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
				for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {
					region _union = (*rj).unite(*ri);
					region_queue.push_back(_union);
				}
			}

			for(region& to_add : region_queue) {
				if(add_region(to_add)) {
					made_change = 1;
				}
			}

		}
		assert_each_trim();
		assert_norepeat();

		debug_printf("done\n");

		return 0;
	}

	int solver::find_regions() {
		trim_regions();
		find_base_regions();
		find_aux_regions();
		if(!(safe_queue.empty() && bomb_queue.empty()))
			return 0;

		//find_chains();
		//find_leftover();
		
		return 1;
	}


	/**
	 * 
	 * to be called when a safe cell is opened
	 * removes it from all regions, so it can be forgotten about and free up memory
	 * skips regions that do not have the cell
	 * throws an error if a region has the cell, but removal is impossible (has no safe)
	 * 
	 * all empty cells are removed
	 * 
	 * returns nonzero if the safe cell is found, otherwise returns 0
	 * 
	 **/
	int solver::remove_safe_from_all_regions(rc_coord cell) {
		int found = 0;
		for(literator ri = regions.begin(); ri != regions.end();) {
			switch((*ri).remove_safe(cell)) {
			case 0://success
				found = 1;
			case 1://not in region
				if((*ri).empty()) {
					ri = regions.erase(ri);
				} else {
					++ri;
				}
				break;

			case 2://we thought we knew this was wrong
				debug_printf("(%u,%u){\n",cell.row,cell.col);
				for(literator rj = regions.begin(); rj != regions.end(); ++rj) {
					if(ri == rj) 
						debug_printf("**");
					else
						debug_printf("  ");
					debug_print_region(*rj);
					if(ri == rj) {
						debug_printf("**\n");
					} else {
						debug_printf("  \n");
					}
					}
				debug_printf("}\n");
				throw std::logic_error("Attempted to remove a safe space from a region that has no safe space");
			default:
				throw std::logic_error("Impossible return value from remove_safe");
			}
		}


		for(std::list<rc_coord>::iterator ri = safe_queue.begin(); ri != safe_queue.end();) {
			if(*ri == cell) {
				debug_printf("WARNING: removing repeated cell (%d,%d) from safe queue.\n", (*ri).row, (*ri).col);
				ri = safe_queue.erase(ri);
			} else {
				++ri;
			}
		}
		
		return found;
	}

	/**
	 * 
	 * to be called when a bomb cell is opened
	 * removes it from all regions, so it can be forgotten about and free up memory
	 * skips regions that do not have the cell
	 * throws an error if a region has the cell, but removal is impossible (has no bomb)
	 * 
	 * all empty cells are removed
	 * 
	 * returns nonzero if the bomb cell is found, otherwise returns 0
	 * 
	 **/
	int solver::remove_bomb_from_all_regions(rc_coord cell) {
		int found = 0;
		for(literator ri = regions.begin(); ri != regions.end();) {
			switch((*ri).remove_bomb(cell)) {
			case 0://success
				found = 1;
				//fallthrough
			case 1://not in region
				if((*ri).empty()) {
					ri = regions.erase(ri);
				} else {
					++ri;
				}
				break;

			case 2://we thought we knew this was wrong
				debug_printf("(%u,%u){\n",cell.row,cell.col);
				for(literator rj = regions.begin(); rj != regions.end(); ++rj) {
					if(ri == rj) 
						debug_printf("**");
					else
						debug_printf("  ");
					debug_print_region(*rj);
					if(ri == rj) {
						debug_printf("**\n");
					} else {
						debug_printf("  \n");
					}
					}
				debug_printf("}\n");
				throw std::logic_error("Attempted to remove a bomb space from a region that has no bomb space");
			default:
				throw std::logic_error("Impossible return value from remove_bomb");
			}
		}

		for(std::list<rc_coord>::iterator ri = bomb_queue.begin(); ri != bomb_queue.end();) {
			if(*ri == cell) {
				debug_printf("WARNING: removing repeated cell (%d,%d) from bomb queue.\n", (*ri).row, (*ri).col);
				ri = bomb_queue.erase(ri);
			} else {
				++ri;
			}
		}

		return found;
	}

	int solver::find_leftover() {
		return 0;
	}

	/**
	 * 
	 * Forces solver to flag a cell, and treat it as a bomb for all future calculations. 
	 * This may result in errors and exceptions further on if it is incorrect.
	 * 
	 * Returns zero if the cell is successfully flagged, nonzero otherwise.
	 *  
	 **/
	int solver::manual_flag(rc_coord arg) {
		return apply_flag(arg);
	}

	/**
	 * 
	 * Forces solver to open a cell.
	 * 
	 * Returns the number of cells opened, or -1 on error
	 * 
	 **/
	int solver::manual_open(rc_coord arg) {
		return apply_open(arg);
	}

	/**
	 * 
	 * Flags a cell, and treat it as a bomb for all future calculations.
	 * 
	 * Returns zero if the cell is successfully flagged
	 *  
	 **/
	int solver::apply_flag(rc_coord arg) {
		g.set_flag(arg.row, arg.col, grid::ms_flag);

		if(g.get(arg.row, arg.col) == grid::ms_flag) {
			remove_bomb_from_all_regions(arg);
			return 0;
		} else {
			return 1;
		}
	}

	/**
	 * 
	 * Opens a cell and removes all instances of the cell from all regions
	 * 
	 * Returns the number of cells opened, or -1 on error
	 * 
	 **/
	int solver::apply_open(rc_coord arg) {
		switch(g.open(arg.row, arg.col)) {
		case -1:
			throw std::invalid_argument("Could not open the cell");
		case 0:
			return 0;
		case 1:
			remove_safe_from_all_regions(arg);
			return 1;
		default: //multiple cells opened, remove all open cells
			int ret = 0;
			for(unsigned int r = 0; r < g.height(); ++r) {
				for(unsigned int c = 0; c < g.width(); ++c) {
					grid::cell curcell = g.get(r,c);
					if((curcell >= grid::ms_0 && curcell <= grid::ms_8) || curcell == grid::ms_non_bomb) {
						ret += remove_safe_from_all_regions(rc_coord{r,c});
					}
				}
			}
			return ret;	
		}
	}

	/**
	 * 
	 * Finds all regions that guarantee that the contained cells are
	 * safe or bombs and adds them to the queues if they are not already.
	 * 
	 **/
	int solver::fill_queue() {
		for(region& check : regions) {
			if(check.size() == check.min()) { //implies size == max == min for all valid state regions
				for(rc_coord bomb : check) {
					add_to_bomb_queue(bomb);
				}
			} else if (check.max() == 0) {
				for(rc_coord safe : check) {
					add_to_safe_queue(safe);
				}				
			}
		}
		return 0;
	}

	/**
	 * 
	 * Adds the input cell to the back of the bomb queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 * 
	 **/
	int solver::add_to_bomb_queue(rc_coord to_add) {
		for(const rc_coord& check : bomb_queue) {
			if(to_add == check)
				return 0;
		}
		bomb_queue.push_back(to_add);
		return 1;
	}

	/**
	 * 
	 * Adds the input cell to the back of the safe queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 * 
	 **/
	int solver::add_to_safe_queue(rc_coord to_add) {
		for(const rc_coord& check : safe_queue) {
			if(to_add == check)
				return 0;
		}
		safe_queue.push_back(to_add);
		return 1;
	}



	int solver::solve_certain() {
		int ret = 0;
		while(step_certain() != rc_coord{0xffff,0xffff})
			++ret;
		return ret;
	}


	rc_coord solver::step_certain() {
		if(g.gamestate() != grid::RUNNING) {
			return rc_coord{ 0xffff,0xffff };
		}

		if(bomb_queue.empty() && safe_queue.empty()) {
			find_regions();
			fill_queue();
		}

		if(!bomb_queue.empty()) {
			rc_coord ret = bomb_queue.front();
			assert(apply_flag(ret) == 0);//removes ret
			return ret;
		} else if(!safe_queue.empty()) {
			rc_coord ret = safe_queue.front();
			if(g.get(ret.row,ret.col)!=grid::ms_hidden) {
				//TODO find bug where this code was reached
				debug_printf("cell:%d\nrow:%u\ncol:%u\n",g.get(ret.row,ret.col),ret.row,ret.col);
				throw std::logic_error("Attempting to open a non-hidden cell");
			}
			int open_status = apply_open(ret);//removes ret
			if(!(open_status > 0)) {
				debug_printf("cells opened: %hhu\nrow:%u\ncol:%u\n",open_status,ret.row,ret.col);
				throw std::logic_error("Opened the wrong number of cells");
			}
			return ret;
		} else {
			return rc_coord{ 0xffff,0xffff };
		}
	}


}

