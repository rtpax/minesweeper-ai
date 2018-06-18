#include "solver.h"
#include <stdexcept>

namespace ms {

	typedef std::list<region>::iterator literator;
	literator it_add(literator base, int amount) {
		for(int i = 0; i < amount; ++i)
			++base;
		for(int i = 0; i > amount; --i)
			--base;
		return base;
	}

#ifdef DEBUG
	int solver::assert_each_trim() {
		for(literator ri = regions.begin(); ri != regions.end(); ++ri) {
			(*ri).assert_trim();
		}
		return 0;
	}
	int solver::assert_norepeat() {
		for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
			for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {
				assert((*ri) != *rj);
			}
		}
		return 0;
	}
#else
	int solver::assert_each_trim(){ return 0; }
	int solver::assert_norepeat(){ return 0; }
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
				region reg;
				int num_flags = 0;
				for (int rr = -1; rr <= 1; ++rr)
					for (int cc = -1; cc <= 1; ++cc)
						if (!(rr == 0 && cc == 0) && g.iscontained(r + rr, c + cc) &&
							g.get(r + rr, c + cc) <= 8 && g.get(r + rr, c + cc) > 0) {
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
				reg.set_count(g.get(r, c) - num_flags);
				if(!reg.empty())
					add_region(reg);
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
		trim_regions();
		int size = regions.size();

		for (int startsize = 0, endsize = size; startsize != endsize; endsize = regions.size()) { //loops as long as nothing was added
			startsize = endsize;

			if(!regions.empty())//otherwise the conditional will never be false
			for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
				for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {
					region _inter = (*rj).intersect(*rj);
					if (_inter.size() > 0) {
						//region _union = ri.unite(rj);
						region _subij = (*ri).subtract(*rj);
						region _subji = (*rj).subtract(*ri);
						add_region(_inter);
						//add_region(_union);
						add_region(_subij);
						add_region(_subji);
					}
				}
			}

		}
		assert_each_trim();
		assert_norepeat();

		return regions.size() - size;
	}

	/**
	 *
	 * Removes all empty regions.
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
			if ((*ri).size() != 0) {
				(*ri).trim(); //can never make a size 0 from nonzero
				++ri;
			}
			else {
				ri = regions.erase(ri);
			}
		}
		if(!regions.empty())//otherwise the conditional will never be false
		for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
			for(literator rj = it_add(rj, 1); rj != regions.end();) {
				if ((*ri).samearea(*rj)) {
					*ri = (*ri).merge(*rj);
					regions.erase(rj);
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
	 * 
	 * Returns 0 if merging occurs, otherwise returns 1. 
	 *
	 * Complexity \f$O(N)\f$
	 * 
	 **/
	int solver::add_region(const region& arg) {
		if(arg.size() == 0)
			return 0;
		for (literator ri = regions.begin(); ri != regions.end(); ++ri) {
			if (arg.samearea(*ri)) {
				*ri = (*ri).merge(arg);
				(*ri).assert_nonempty();
				return 0;
			}
		}
		regions.push_back(arg);
		return 1;
	}

	bool ispair(const region& check) {
		return check.size() == 2 && check.min() == 1 && check.max() == 1;
	}

	bool hasoverlap(const std::vector<region>& chain, const region& check) {
		for(const region& reg : chain) {
			if(check.has_intersect(reg))
				return true;
		}
		return false;
	}

	/**
	 * 
	 * returns a vector containing all regions in a chain that starts at arg.front()
	 * all regions in arg that are put into chains are removed from arg
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

		return 0;
	}

	int solver::find_regions() {
		trim_regions();
		find_base_regions();
		find_aux_regions();
		if(!(safe_queue.empty() && bomb_queue.empty()))
			return 0;

		find_chains();
		find_leftover();
		
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

			case 2://we thought we knew this was wrong
				throw std::logic_error("Attempted to remove a safe space from a region that has no safe space");
			default://impossible
				throw std::logic_error("Impossible return value from remove_safe");
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
			case 1://not in region
				if((*ri).empty()) {
					ri = regions.erase(ri);
				} else {
					++ri;
				}

			case 2://we thought we knew this was wrong
				throw std::logic_error("Attempted to remove a bomb space from a region that has no bomb space");
			default://impossible
				throw std::logic_error("Impossible return value from remove_bomb");
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
	 **/
	int solver::manual_flag(rc_coord arg) {
		g.flag(arg.row, arg.col);

		if(g.get(arg.row, arg.col)) {
			remove_bomb_from_all_regions(arg);
			return 0;
		} else {
			return 1;
		}
	}

	/**
	 * 
	 * Forces solver to open a cell.
	 * 
	 **/
	int solver::manual_open(rc_coord arg) {
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
			g.flag(ret.row, ret.col);
			bomb_queue.pop_front();
			return ret;
		} else if(!safe_queue.empty()) {
			rc_coord ret = safe_queue.front();
			g.open(ret.row, ret.col);
			safe_queue.pop_front();
			return ret;
		} else {
			return rc_coord{ 0xffff,0xffff };
		}
	}


}


/*
Basic strategy

Create regions around each number, if they overlap figure out what that means
   [A ] [A ] [AB] [AB] [B ] with A:1 and B:2
=> [# ] [# ] [C ] [C ] [* ] with C:1

remove obsolete regions:
	cells.size == 0
	duplicate





*/