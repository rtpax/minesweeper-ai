#include "solver.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "debug.h"

namespace ms {


	/**\internal
	 * 
	 * Namespace for organizing globals used by class implementations.
	 * 
	 **/
	namespace util {

		/****/
		typedef std::list<region>::iterator literator;
		typedef std::vector<literator>::iterator vliterator;
		typedef std::vector<region>::iterator viterator;

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
		void debug_print_rc_coord(const rc_coord& arg) {
			debug_printf("(%u,%u)",arg.row,arg.col);
		}
		
		/**Print information about region iff debug printing is enabled**/
		void debug_print_region(const region& arg) {
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
		std::vector<region> one_chain(std::vector<region>& arg) {
			bool found = true;
			std::vector<region> chain;

			while(found) {
				found = false;
				for(viterator iter = arg.begin(); iter != arg.end();) {
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
		for(literator ri = regions.begin(); std::next(ri, 1) != regions.end(); ++ri) {\
			for(literator rj = std::next(ri, 1); rj != regions.end(); ++rj) {\
				assert((*ri) != *rj);\
			}\
		}\
	}while(0)
#else
	#define assert_each_trim()
	#define assert_norepeat()
#endif

	int solver::init_cell_keys() {
		cell_keys = new std::vector<literator>*[g.height()];
		for(unsigned i = 0; i < g.height(); ++i) {
			cell_keys[i] = new std::vector<literator>[g.width()]();
		}
		return 0;
	}

	/**
	 *
	 * Copies grid, all other members default initialize
	 *
	 **/
	solver::solver(const grid& start) : g(start, grid::FULL_COPY) {
		cell_keys = new std::vector<literator>*[g.height()];
		for(unsigned i = 0; i < g.height(); ++i) {
			cell_keys[i] = new std::vector<literator>[g.width()]();
		}
	}

	/**
	 * 
	 * Initializes the internal grid with the given parameters
	 * 
	 **/
	solver::solver(unsigned int height, unsigned int width, unsigned int bombs) : g(height,width,bombs) {
		cell_keys = new std::vector<literator>*[g.height()];
		for(unsigned i = 0; i < g.height(); ++i) {
			cell_keys[i] = new std::vector<literator>[g.width()]();
		}
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
		region remaining;

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				remaining.addcell(rc_coord(r,c));
			}
		}
		remaining.set_count(g.bombs());

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				grid::cell gotten = g.get(r,c);
				if(gotten == grid::ms_flag) {
					remaining.remove_bomb(rc_coord(r,c));					
				}
				else if(gotten <= 8 && gotten >= 0) {//we could exclude zero to save time, grid should deal with those automatically
					remaining.remove_safe(rc_coord(r,c));
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
						assert(gotten >= num_flags);
						reg.set_count(gotten - num_flags);
						add_region(reg);
					}
				}
			}
		}
		add_region(remaining);

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
	 * Complexity of the inner loop \f$\O(N^2)\f$.
	 * Outer loop's complexity is nontrivial, runs until it cannot make any more aux regions.
	 * 
	 **/
	int solver::find_aux_regions() {
		debug_printf("find_aux_regions...");
		trim_regions();
		int size = regions.size();


		for (bool made_change = 1; made_change;) { //loops as long as something was added
			made_change = 0;
			std::vector<region> region_queue;

			for(literator ri = regions.begin(); ri != regions.end(); ++ri) {
				std::vector<literator> overlaps = { ri };
				for(rc_coord cell : *ri) {
					for(literator over : cell_keys[cell.row][cell.col]) {
						std::vector<literator>::iterator index = std::find(overlaps.begin(),overlaps.end(),over);
						if(index == overlaps.end()) {
							assert(overlaps.end() == std::find_if(overlaps.begin(),overlaps.end(),[over](literator l){return *l == *over;}));
							overlaps.push_back(over);
						} else {
							assert(overlaps.end() != std::find_if(overlaps.begin(),overlaps.end(),[over](literator l){return *l == *over;}));
						}
					}
				}
				if(!overlaps.empty())
				for(std::vector<literator>::iterator rj = std::next(overlaps.end(),1); rj != overlaps.end(); ++rj) {
					region_queue.push_back(ri->intersect(**rj));
					region_queue.push_back(ri->subtract(**rj));
					region_queue.push_back((*rj)->subtract(*ri));
				}
			}

			for(region& to_add : region_queue) {
				if(add_region(to_add) != regions.end()) {
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
	 * Find all regions that can be deduced from the existing regions.
	 * 
	 * Calculate the intersection of each pair of regions
	 *   - if empty, discard it
	 *   - if nonempty, add the intersection and subraction to `this.aux_regions`
	 * 
	 * Returns the number of regions added (note that it does not include modified regions, only added)
	 * 
	 * Unlike solver::find_aux_regions it stops as soon as a cells can be added to the queue
	 * 
	 * Complexity of the inner loop \f$\O(N^2)\f$..
	 * Outer loop's complexity is nontrivial, runs until it has found aux regions (often just one iteration).
	 * 
	 **/
	int solver::lazy_aux_regions() {
		debug_printf("lazy_aux_regions...");
		trim_regions();
		int size = regions.size();

		std::vector<literator> regions_added;
		for(literator liter = regions.begin(); liter != regions.end(); ++liter)
			regions_added.push_back(liter);

		while (!regions_added.empty()) { //loops as long as something was added
			std::cout << "[" << regions.size() << "]";
			std::cout.flush();
			if(fill_queue())
				break;

			std::vector<region> region_queue;

			for(literator ri : regions_added) {
				std::cout << ".";
				std::cout.flush();
				std::vector<literator> overlaps = { ri };
				for(rc_coord cell : *ri) {
					for(literator over : cell_keys[cell.row][cell.col]) {
						std::vector<literator>::iterator index = std::find(overlaps.begin(),overlaps.end(),over);
						if(index == overlaps.end()) {
							assert(overlaps.end() == std::find_if(overlaps.begin(),overlaps.end(),[&over](literator l){return *l == *over;}));
							overlaps.push_back(over);
						} else {
							assert(overlaps.end() != std::find_if(overlaps.begin(),overlaps.end(),[&over](literator l){return *l == *over;}));
						}
					}
				}
				if(!overlaps.empty())
				for(std::vector<literator>::iterator rj = std::next(overlaps.begin(),1); rj != overlaps.end(); ++rj) {
					region_queue.push_back(ri->intersect(**rj));
					region_queue.push_back(ri->subtract(**rj));
					region_queue.push_back((*rj)->subtract(*ri));
				}
			}
			std::cout << "*";
			std::cout.flush();
			regions_added.clear();
			for(region& to_add : region_queue) {
				literator added = add_region(to_add);
				if(added != regions.end()) {
					if(std::find(regions_added.begin(), regions_added.end(), added) == regions_added.end())//this check might take more time than it's worth
						regions_added.push_back(added);
				}
			}

		}
		std::cout << "\n";
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
	 * TODO increase speed by using cell_keys
	 *
	 **/
	int solver::trim_regions() {
		region zero;

		size_t initial_size = regions.size();

		for (literator ri = regions.begin(); ri != regions.end();) {
			if (region_is_helpful(*ri)) {
				if(ri->trim()) {
					if (!region_is_helpful(*ri)) {
						ri = remove_region(ri);
					} else {
						++ri;
					}
				} else {
					++ri;
				}
			}
			else {
				ri = remove_region(ri);
			}
		}
		
		for(size_t row = 0; row < g.height(); ++row) {
			for(size_t col = 0; col < g.width(); ++col) {
				std::vector<literator>& key = cell_keys[row][col];
				if(!key.empty())//otherwise the conditional will never be false
				for(vliterator ki = key.begin(); std::next(ki, 1) != key.end() && ki != key.end(); ++ki) {
					for(vliterator kj = std::next(ki, 1); kj != key.end();) {
						if ((*ki)->samearea(**kj)) {
							**ki = (*ki)->merge(**kj);
							vliterator kj_next = kj;
							++kj_next;
							remove_region(*kj);
							kj = kj_next;

							assert(!(*ki)->empty());
						}
						else {
							++kj;
						}
					}
				}
			}
		}

		return initial_size - regions.size();
	}

	/**
	 *
	 * Adds a region to the list of regions, merging if a region already exists covering the same area.
	 * Skips regions that offer no information. If a new region is added, add it to the appropriate cell_keys
	 * 
	 * Returns `regions.end()` if merging occurs or region not added, otherwise returns an iterator to the region add/modified. 
	 *
	 * Complexity \f$O(N)\f$
	 * 
	 **/
	std::list<region>::iterator solver::add_region(const region& to_add) {
		debug_printf("adding region ");
		debug_print_region(to_add);
		debug_printf("...\n");
		if(!region_is_helpful(to_add)) {
			debug_printf("skipping because unhelpful\n");
			return regions.end();
		}

		std::vector<literator>& key = cell_keys[to_add.begin()->row][to_add.begin()->col];//guaranteed to have a first element, otherwise would fail early

		for (literator ri : key) {
			if (to_add.samearea(*ri)) {
				if(!(to_add.min() > ri->min() || to_add.max() < ri->max()))
					return regions.end();
				debug_printf("merging ");
				debug_print_region(*ri);
				debug_printf(" and ");
				debug_print_region(to_add);
				debug_printf("\n");
				*ri = ri->merge(to_add);
				assert_nonempty(*ri);
				return ri;
			}
		}

		debug_printf("adding region to list...\n");
		regions.push_back(to_add);
		literator added = std::next(regions.end(), -1);

		debug_printf("adding region to keys...\n");
		for(rc_coord cell : *added) {
			debug_printf("    adding region to key: ");
			debug_print_rc_coord(cell);
			debug_printf("\n");
			cell_keys[cell.row][cell.col].push_back(added);
		}
		debug_printf("done\n");

		return added;
	}

	/**
	 * 
	 * Removes the specified region from the list of regions and all associated keys.
	 * 
	 * Complexity \f$O(N )\f$
	 * 
	 **/
	literator solver::remove_region(literator to_remove) {
		for(rc_coord cell : *to_remove) {
			std::vector<literator>& key = cell_keys[cell.row][cell.col];
			vliterator remove_it = std::find(key.begin(), key.end(), to_remove);
			assert(remove_it != key.end());
			key.erase(remove_it);
		}
		return regions.erase(to_remove);
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





	int solver::find_regions() {
		trim_regions();
		find_base_regions();
		lazy_aux_regions();
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
		std::vector<literator>& key = cell_keys[cell.row][cell.col];
		int found = 0;

		while(!key.empty()) {
			int err = key.back()->remove_safe(cell);
			(void)err; //suppress unused warnings
			assert(err != 2 && "Attempted to remove safe from region with no safe spaces");
			key.pop_back();
			++found;
		}

		for(std::deque<rc_coord>::iterator ri = bomb_queue.begin(); ri != bomb_queue.end();) {
			if(*ri == cell) {
				ri = bomb_queue.erase(ri);
			} else {
				++ri;
			}
		}
		for(std::deque<rc_coord>::iterator ri = safe_queue.begin(); ri != safe_queue.end();) {
			if(*ri == cell) {
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
		debug_printf("removing bomb from cell ");
		debug_print_rc_coord(cell);
		debug_printf("...\n");

		std::vector<literator>& key = cell_keys[cell.row][cell.col];
		int found = 0;

		while(!key.empty()) {
			debug_printf("removing bomb from region ");
			debug_print_region(*key.back());
			int err = key.back()->remove_bomb(cell);
			(void) err; //suppresss unused warnings
			debug_printf("\nresulting region ");
			debug_print_region(*key.front());
			debug_printf("\n");
			assert(err != 2 && "Attempted to remove safe from region with no safe spaces");
			key.pop_back();
			++found;
		}

		for(std::deque<rc_coord>::iterator ri = bomb_queue.begin(); ri != bomb_queue.end();) {
			if(*ri == cell) {
				ri = bomb_queue.erase(ri);
			} else {
				++ri;
			}
		}
		for(std::deque<rc_coord>::iterator ri = safe_queue.begin(); ri != safe_queue.end();) {
			if(*ri == cell) {
				ri = safe_queue.erase(ri);
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
		int num_added = 0;
		for(region& check : regions) {
			if(check.size() == check.min()) { //implies size == max == min for all valid state regions
				for(rc_coord bomb : check) {
					num_added += add_to_bomb_queue(bomb);
				}
			} else if (check.max() == 0) {
				for(rc_coord safe : check) {
					num_added += add_to_safe_queue(safe);
				}				
			}
		}
		return num_added;
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


	/**
	 * 
	 * Runs until the next step may fail.
	 * 
	 * Returns the number of steps taken.
	 * 
	 **/
	int solver::solve_certain() {
		int ret = 0;
		while(step_certain() != rc_coord{0xffff,0xffff})
			++ret;
		return ret;
	}

	/**
	 * 
	 * Opens/flags a cell if it is certain it will be correct.
	 * 
	 * Returns the cell opened, or {0xffff,0xffff} if none is opened
	 * 
	 **/
	rc_coord solver::step_certain() {
		if(g.gamestate() != grid::RUNNING) {
			return rc_coord{ 0xffff,0xffff };
		}

		if(bomb_queue.empty() && safe_queue.empty()) {
			fill_queue();
			if(bomb_queue.empty() && safe_queue.empty()) {
				find_regions();
				fill_queue();
			}
		}

		if(!bomb_queue.empty()) {
			rc_coord ret = bomb_queue.front();
			int err = apply_flag(ret);
			(void) err; //suppress unused warnings
			debug_printf("bomb_queue:\n");
			for(rc_coord bomb : bomb_queue) {
				debug_printf("    ");
				debug_print_rc_coord(bomb);
				debug_printf("\n");
			}
			debug_printf("safe_queue:\n");
			for(rc_coord safe : safe_queue) {
				debug_printf("    ");
				debug_print_rc_coord(safe);
				debug_printf("\n");
			}
			assert(err == 0);
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
			debug_printf("safe_queue:\n");
			for(rc_coord safe : safe_queue) {
				debug_printf("    ");
				debug_print_rc_coord(safe);
				debug_printf("\n");
			}
			return ret;
		} else {
			return rc_coord{ 0xffff,0xffff };
		}
	}

	std::mt19937 solver::rng(time(NULL));

	/**
	 * 
	 * Runs until win or loss.
	 * 
	 * Returns gamestate
	 *  
	 **/
	int solver::solve() {
		int ret = 0;
		while(step() != rc_coord{0xffff,0xffff})
			++ret;
		return ret;
	}

	/**
	 * 
	 * Opens/flags a cell as long a game is running or ready to start running.
	 * Opens cells that are certain first before attempting to open a new cell.
	 * 
	 * Returns the cell opened, or {0xffff,0xffff} if none is opened
	 * 
	 **/
	rc_coord solver::step() {
		if(g.gamestate() != grid::RUNNING && g.gamestate() != grid::NEW) {
			return rc_coord{ 0xffff,0xffff };
		}

		rc_coord ret = step_certain();
		if(ret != rc_coord{ 0xffff,0xffff })
			return ret;

		std::vector<rc_coord> choices;
		for(unsigned row = 0; row < g.height(); ++row) {
			for(unsigned col = 0; col < g.width(); ++col) {
				if(g.get(row,col) == grid::ms_hidden || g.get(row,col) == grid::ms_question) {
					choices.push_back(rc_coord{row,col});
				}
			}
		}

		if(choices.size() > 0) {
			int open_index = rng() % choices.size();
			apply_open(choices[open_index]);
			return choices[open_index];
		} else {
			return rc_coord{ 0xffff,0xffff };
		}
	}




}

