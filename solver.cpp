#include "solver.h"

namespace ms {


	/* solver::solver(grid * g)
	 *
	 * sets grid, all other members default initialize
	 *
	 */
	solver::solver(grid * g) {
		this->g = g;
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
		for (unsigned int r = 0; r < g->height(); ++r) {
			for (unsigned int c = 0; c < g->width(); ++c) {
				region reg;
				int num_flags = 0;
				for (int rr = -1; rr <= 1; ++rr)
					for (int cc = -1; cc <= 1; ++cc)
						if (!(rr == 0 && cc == 0) && g->iscontained(r + rr, c + cc) &&
							g->get(r + rr, c + cc) <= 8 && g->get(r + rr, c + cc) > 0) {
							switch (g->get(r + rr, c + cc)) {
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
				reg.min = reg.max = g->get(r, c) - num_flags;
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
		std::vector<region> old_regions;
		int size = regions.size();

		do {
			old_regions = regions;

			for (int i = 0; i < size - 1; ++i) { //first of pair
				for (int j = i + 1; j < size; ++j) { //second of pair
					region _inter = regions[i].intersect(regions[j]);
					if (_inter.size() > 0) {
						//region _union = regions[i].unite(regions[j]); // can't think of any useful application for this knowledge
																		// which implies that after getting subij, subji, and inter,
																		// we should delete the original (TODO determine if that's a good
																		// idea and implement)
																		// even if  union is usesd, it would be better in a seperate loop
																		// because it's potential run time is much longer than the others
						region _subij = regions[i].subtract(regions[j]);
						region _subji = regions[j].subtract(regions[i]);
						add_region(_inter);
						//add_region(_union);
						add_region(_subij);
						add_region(_subji);
					}
				}
			}
			trim_regions();
		} while (old_regions != regions);

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

		for (unsigned int i = 0; i < regions.size();) {
			if (regions[i].size() != 0) {
				regions[i].trim(); //can never make a size 0
				++i;
			}
			else {
				regions.erase(regions.begin() + i);
			}
		}
		for (int i = 0; i < (int) regions.size() - 1; ++i) {
			for (unsigned int j = i + 1; j < regions.size();) {
				if (regions[i].samearea(regions[j])) {
					regions[i] = regions[i].merge(regions[j]);
					regions.erase(regions.begin() + j);
				}
				else {
					++j;
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
		for (unsigned int i = 0; i < regions.size(); ++i) {
			if (regions[i].size() == arg.size()) {
				regions[i] = regions[i].merge(arg);
				return 0;
			}
		}
		return 1;
	}

	/* int solver::find_conglomerate()
	 *
	 *
	 *
	 *
	 */
	int solver::find_conglomerate() {
		conglomerates.push_back(region());
		std::vector<region> starts = find_best_starting_points(conglomerates[0]);
		for (unsigned int i = 1; i < starts.size(); ++i) {
			conglomerates.push_back(conglomerates[0]);
		}
		for (unsigned int i = 1; i < starts.size(); ++i) {
			work_from_starting_point(conglomerates[0], starts[0]);
		}
		return 0;
	}



	int solver::find_regions() {
		trim_regions();
		find_base_regions();
		find_aux_regions();
		find_conglomerate();
		leftover_region();
		return 0;
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