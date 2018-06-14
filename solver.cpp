#include "solver.h"

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


	/* solver::solver(grid * g)
	 *
	 * copies grid, all other members default initialize
	 *
	 */
	solver::solver(const grid& start) : g(start, FULL_COPY) {
		
	}


	/* int solver::find_base_regions()
	 *
	 * find the areas around each number where there could be bombs.
	 * such places must fit the following criteria:
	 * --on the grid
	 * --not not flagged or opened (so are hidden or questionmarked)
	 * --adjacent to the given square
	 *
	 * determine in each case how many bombs there are
	 * this will be exact, so region.min == region.max
	 *
	 * store the results in this.regions
	 *
	 */
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
							case ms_hidden:
							case ms_question:
								reg.addcell(rc_coord(r + rr, c + cc));
								break;
							case ms_flag:
								++num_flags;
								break;
							default:
								break;
							}

						}
				reg.set_count(g.get(r, c) - num_flags);
				regions.push_back(reg);
			}
		}

		return 0;
	}

	/* int solver::find_aux_regions()
	 *
	 * calculate the intersection of each pair of regions
	 * --if empty (size == 0), discard it
	 * --if nonempty, add the intersection to this.aux_regions,
	 *     add the pair to this.region_pairs
	 *
	 * returns the number of regions added
	 */
	int solver::find_aux_regions() {
		trim_regions();
		int size = regions.size();

		for (int startsize = size, endsize = size; startsize == endsize; endsize = regions.size()) { //loops as long as nothing was added
			startsize = endsize;

			if(!regions.empty())//otherwise the conditional will never be true
			for(literator ri = regions.begin(); it_add(ri, 1) != regions.end(); ++ri) {
				for(literator rj = it_add(ri, 1); rj != regions.end(); ++rj) {
					region _inter = (*rj).intersect(*rj);
					if (_inter.size() > 0) {
						//region _union = ri.unite(rj);	// can't think of any useful application for this knowledge
														// which implies that after getting subij, subji, and inter,
														// we should delete the original (TODO determine if that's a good
														// idea and implement)
														// even if  union is usesd, it would be better in a seperate loop
														// because it's potential run time is much longer than the others
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

	/* int solver::trim_regions()
	 *
	 * removes all empty regions
	 * merges regions that have the same number of cells
	 * returns the number of regions removed (two being merged is one being removed)
	 *
	 */
	int solver::trim_regions() {
		region zero;

		size_t initial_size = regions.size();

		for (literator ri = regions.begin(); ri != regions.end();) {
			if ((*ri).size() != 0) {
				(*ri).trim(); //can never make a size 0 from nonzero
				++ri;
			}
			else {
				regions.erase(ri);
			}
		}
		if(!regions.empty())//otherwise the conditional will never be true
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

	/* int solver::add_region(region arg)
	 *
	 * checks if this.regions contains a region with the same number of cells as arg
	 * if it does, merge it with that cell and return 0
	 * else add arg to this.regions and return 1
	 *
	 */
	int solver::add_region(const region& arg) {
		if(arg.size() == 0)
			return 0;
		for (literator ri = regions.begin(); ri != regions.end(); ++ri) {
			if ((*ri).size() == arg.size()) {
				*ri = (*ri).merge(arg);
				(*ri).assert_nonempty();
				return 0;
			}
		}
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

	/* std::vector<region> one_chain(std::list<region> arg)
	 * 
	 * returns a vector containing all regions in a chain that starts at arg.front()
	 * all regions in arg that are put into chains are removed from arg
	 * 
	 */
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

	/* std::vector<region> solver::find_chains() const
	 *
	 * a chain is a series of regions that have size 2 and 1 bomb that overlap eachother
	 * no two complete chains overlap
	 * 
	 */
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

	/* int solver::find_conglomerate() const
	 *
	 *
	 *
	 *
	 */
	int solver::find_conglomerate(){

		return 0;
	}

	int solver::find_regions() {
		trim_regions();
		find_base_regions();
		find_aux_regions();
		if(!safe_queue.empty())
			return 0;

		find_chains();
		find_leftover();
		
		return 1;
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