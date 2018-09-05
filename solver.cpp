#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <boost/math/special_functions/factorials.hpp>
#include "solver.h"
#include "debug.h"

namespace ms {

	/**
	 * Copies grid, all other members default initialize
	 **/
	solver::solver(const grid& start, grid::copy_type gct) : 
		g(start, gct), regions(g.height(), g.width()) {	}

	/**
	 * Initializes the internal grid with the given parameters
	 **/
	solver::solver(unsigned int height, unsigned int width, unsigned int bombs) : 
		g(height,width,bombs), regions(height, width) { }

	/**
	 * Copies all contents of solver, copies grid with the given copy type
	 **/
	solver::solver(const solver& copy, grid::copy_type gct) : 
		g(copy.g, gct), regions(copy.regions) {}

	/**
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
	 * Complexity \f$O(N)\f$ where \f$N\f$ is the number of cells in the grid
	 **/
	int solver::find_base_regions() {
		region remaining;

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				remaining.add_cell(rc_coord(r,c));
			}
		}
		remaining.set_count(g.bombs());

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				grid::cell gotten = g.get(r,c);
				if(gotten == grid::ms_flag) {
					remaining.remove_bomb(rc_coord(r,c));
				} else if(gotten <= 8 && gotten >= 0) {
					remaining.remove_safe(rc_coord(r,c));
					region reg;
					int num_flags = 0;

					for (int rr = -1; rr <= 1; ++rr)
						for (int cc = -1; cc <= 1; ++cc)
							if (!(rr == 0 && cc == 0) && g.iscontained(r + rr, c + cc)) {
								switch (g.get(r + rr, c + cc)) {
								case grid::ms_hidden:
								case grid::ms_question:
									reg.add_cell(rc_coord(r + rr, c + cc));
									break;
								case grid::ms_flag:
									++num_flags;
									break;
								default:
									break;
								}

							}
					if(!reg.empty()) {
						if(gotten < num_flags)
							throw std::runtime_error("number of flags surrounding the cell exceeds the number of the cell");
						reg.set_count(gotten - num_flags);
						regions.add(reg);
					}
				}
			}
		}
		//this check is added to prevent massive slowdowns from adding remaining when it intersects around 100 cells -> 2000 cells
		//the choice of 10 is somewhat arbitrary, TODO get a better system for deciding when to add remaining
		if(remaining.size() < 10)
			regions.add(remaining);

		return 0;
	}


	/**
	 * Find all regions that can be deduced from the existing regions.
	 * 
	 * Calculate the intersection of each pair of regions
	 *   - if empty, discard it
	 *   - if nonempty, add the intersection and subraction to `this.aux_regions`
	 * 
	 * Returns the number iterations, (zero if nothing to do)
	 * 
	 * If `lazy == true` it stops as soon as a cells can be added to the queue
	 * 
	 * Complexity of the inner loop \f$O(N^2)\f$ where N is the number of regions modified since last time the loop was reached
	 * Outer loop's complexity is nontrivial, runs until it has found aux regions. Often just one or zero iteration,
	 * but if there is nothing to be found, there are often around 10-20 (more when more regions).
	 **/
	int solver::find_aux_regions(bool lazy) {
		dbg::cout << "find_aux_regions [" <<  (lazy?"lazy":"nonlazy") << "]...";
		
		int iterations = 0;

		const region_set::subset_type& regions_added = regions.get_modified_regions();

		while (!regions_added.empty()) { //loops as long as something was added
			dbg::cout2 << "[" << regions_added.size() << "," << regions.size() << "]";

			if(lazy && fill_queue())
				break;

			std::vector<region> region_queue;

			for(auto ri = regions_added.begin(); ri != regions_added.end(); ++ri) {
				dbg::cout2 << ".";
				region_set::subset_type overlaps = regions.regions_intersecting(**ri);
				
				for(auto rj = overlaps.begin(); rj != overlaps.end(); ++rj) {
					if(*rj == *ri)
						continue;
					region_queue.push_back((*ri)->intersect(**rj));
					region_queue.push_back((*ri)->subtract(**rj));
					region_queue.push_back((*rj)->subtract(**ri));
				}
			}
			dbg::cout2 << "*";
			regions.reset_modified_regions();
			for(region& to_add : region_queue) {
				regions.add(to_add);
			}
			++iterations;
		}
		dbg::cout2 << "\n";
		dbg::cout << "done\n";
		return iterations;
	}


	int solver::find_regions() {
		find_base_regions();
		find_aux_regions(true);
		if(!(safe_queue.empty() && bomb_queue.empty()))
			return 0;
		
		return 1;
	}


	/**
	 * To be called when a safe cell is opened.
	 * Removes `cell` from all regions, so it can be forgotten about and free up memory 
	 * and simplify future calculations
	 * 
	 * returns nonzero if the safe cell is found, otherwise returns 0.
	 **/
	int solver::remove_safe(rc_coord cell) {
		int removed = regions.remove_safe(cell);	

		auto it = safe_queue.find(cell);
		if(it != safe_queue.end())
			safe_queue.erase(it);
		assert(bomb_queue.find(cell) == bomb_queue.end());
		
		return removed;
	}

	/**
	 * to be called when a bomb cell is opened
	 * removes it from all regions, so it can be forgotten about and free up memory
	 * and simplify future calculations
	 * 
	 * returns nonzero if the bomb cell is found, otherwise returns 0
	 **/
	int solver::remove_bomb(rc_coord cell) {
		int removed = regions.remove_bomb(cell);

		auto it = bomb_queue.find(cell);
		if(it != bomb_queue.end())
			bomb_queue.erase(it);
		assert(safe_queue.find(cell) == safe_queue.end());

		return removed;
	}

	rc_coord solver::get_safe_from_queue() const {
		return *safe_queue.begin();
	}

	rc_coord solver::get_bomb_from_queue() const {
		return *bomb_queue.begin();
	}

	/**
	 * Forces solver to flag a cell, and treat it as a bomb for all future calculations. 
	 * This may result in errors and exceptions further on if it is incorrect.
	 * 
	 * Returns zero if the cell is successfully flagged, nonzero otherwise.
	 **/
	int solver::manual_flag(rc_coord arg) {
		return apply_flag(arg);
	}

	/**
	 * Forces solver to open a cell.
	 * 
	 * Returns the number of cells opened, or -1 on error
	 **/
	int solver::manual_open(rc_coord arg) {
		return apply_open(arg);
	}

	/**
	 * Flags a cell, and treat it as a bomb for all future calculations.
	 * 
	 * Returns zero if the cell is successfully flagged
	 **/
	int solver::apply_flag(rc_coord arg) {
		g.set_flag(arg.row, arg.col, grid::ms_flag);

		if(g.get(arg.row, arg.col) == grid::ms_flag) {
			remove_bomb(arg);
			return 0;
		} else {
			return 1;
		}
	}

	/**
	 * Opens a cell and removes all instances of the cell from all regions
	 * 
	 * Returns the number of cells opened, or -1 on error
	 **/
	int solver::apply_open(rc_coord arg) {
		switch(g.open(arg.row, arg.col)) {
		case -1:
			throw bad_region_error("Could not open cell (" + std::to_string(arg.row) + "," + std::to_string(arg.col) + ")");
		case 0:
			return 0;
		case 1:
			remove_safe(arg);
			return 1;
		default: //multiple cells opened, remove all open cells
			int ret = 0;
			for(unsigned int r = 0; r < g.height(); ++r) {
				for(unsigned int c = 0; c < g.width(); ++c) {
					grid::cell curcell = g.get(r,c);
					if((curcell >= grid::ms_0 && curcell <= grid::ms_8) || curcell == grid::ms_non_bomb) {
						ret += !!remove_safe(rc_coord{r,c});
					}
				}
			}
			return ret;	
		}
	}

	/**
	 * Finds all regions that guarantee that the contained cells are
	 * safe or bombs and adds them to the queues if they are not already.
	 **/
	int solver::fill_queue() {
		int num_added = 0;
		for(const region& check : regions) {
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
	 * Adds the input cell to the back of the bomb queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 **/
	int solver::add_to_bomb_queue(rc_coord to_add) {
		for(const rc_coord& check : bomb_queue) {
			if(to_add == check)
				return 0;
		}
		bomb_queue.insert(to_add);
		return 1;
	}

	/**
	 * Adds the input cell to the back of the safe queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 **/
	int solver::add_to_safe_queue(rc_coord to_add) {
		for(const rc_coord& check : safe_queue) {
			if(to_add == check)
				return 0;
		}
		safe_queue.insert(to_add);
		return 1;
	}

	region solver::approx_remain() const {
		region ret_base;
		for(unsigned r = 0; r < g.height(); ++r) {
			for(unsigned c = 0; c < g.width(); ++c) {
				ret_base.add_cell(rc_coord(r,c));
			}
		}
		ret_base.set_count(g.bombs());
		for(unsigned r = 0; r < g.height(); ++r) {
			for(unsigned c = 0; c < g.width(); ++c) {
				switch(g.get(r,c)) {
				case grid::ms_flag:
					ret_base.remove_bomb(rc_coord(r,c));
					break;
				case grid::ms_hidden:
				case grid::ms_question:
					break;
				default:
					ret_base.remove_safe(rc_coord(r,c));
				}
			}
		}
		std::vector<region_set::const_iterator> random_regions;
		for(region_set::const_iterator it = regions.cbegin(); it != regions.cend(); ++it) {
			random_regions.push_back(it);
		}
		//number of iterations arbitrary, the more iterations the closer to actual results you get
		region ret;
		for(int i = 0; i < 10; ++i) {
			region to_merge = ret_base;
			std::shuffle(random_regions.begin(), random_regions.end(), rng);
			for(region_set::iterator it : random_regions) {
				to_merge.subtract_to(*it);
			}
			if(ret.empty()) {
				ret = to_merge;
			} else {
				ret.merge_to(to_merge);
			}
		}
		return ret;
	}

	float solver::expected_payout(rc_coord cell) const {
		using boost::math::factorial;

		float payout[9] = { 0 };
		float permutations[9] = { 0 };
		region new_base;
		for(int r = -1; r <= 1; ++r) {
			for(int c = -1; c <= 1; ++c) {
				if((c != 0 || r != 0) && g.iscontained(cell.row + r, cell.col + c)) {
					grid::cell value = g.get(cell.row + r, cell.col + c);
					if(value == grid::ms_hidden || value == grid::ms_question)
						new_base.add_cell(rc_coord(cell.row + r, cell.col + c));
				}
			}
		}
		for(unsigned count = 0; count <= 8; ++count) {
			if(new_base.size() >= count) {
				new_base.set_count(count);
				try {
					solver test(*this, grid::SURFACE_COPY);
					test.apply_open(cell);
					test.regions.add(new_base);
					test.find_aux_regions(false);
					payout[count] += test.safe_queue.size() + test.bomb_queue.size() / 1.5f; //opening safe cells is worth a bit more than flagging cells
					permutations[count] = factorial<float>(new_base.size()) / factorial<float>(new_base.size() - count);
				} catch (const bad_region_error& e) {
					goto else_bad_count;
				}
			} else {
			else_bad_count:
				payout[count] = 0;
				permutations[count] = 0;
			}
		}
		float num = 0;
		int den = 0;
		for(unsigned count = 0; count <= 8; ++count) {
			num += payout[count] * permutations[count];
			den += permutations[count];
		}
		if(den == 0) {
			return -1;
		} else {
			return num / den;
		}
	}


	/**
	 * Runs until the next step is not guaranteed to succeed.
	 * 
	 * Returns the number of steps taken.
	 **/
	int solver::solve_certain() {
		int ret = 0;
		while(step_certain() != BAD_RC_COORD)
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell if it is certain it will be correct.
	 * 
	 * Returns the cell opened, or `BAD_RC_COORD` if none is opened
	 **/
	rc_coord solver::step_certain() {
		if(g.gamestate() != grid::RUNNING) {
			return BAD_RC_COORD;
		}

		if(bomb_queue.empty() && safe_queue.empty()) {
			fill_queue();
			if(bomb_queue.empty() && safe_queue.empty()) {
				find_regions();
				fill_queue();
			}
		}

		if(!bomb_queue.empty()) {
			rc_coord ret = get_bomb_from_queue();
			apply_flag(ret);
			dbg::cout << "flagged: " << ret << "\n";
			return ret;
		} else if(!safe_queue.empty()) {
			rc_coord ret = get_safe_from_queue();
			if(g.get(ret.row,ret.col)!=grid::ms_hidden) {
				throw std::logic_error("Attempting to open a non-hidden cell");
			}
			int open_status = apply_open(ret);//removes ret
			if(!(open_status > 0)) {
				throw std::logic_error("Opened the wrong number of cells");
			}
			dbg::cout << "opened: " << ret << "\n";
			return ret;
		} else {
			return BAD_RC_COORD;
		}
	}

	std::mt19937 solver::rng(time(NULL));

	/**
	 * Runs until win or loss.
	 * 
	 * Returns gamestate
	 **/
	int solver::solve() {
		int ret = 0;
		while(step() != BAD_RC_COORD)
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell as long a game is running or ready to start running.
	 * Opens cells that are certain first before attempting to open a new cell.
	 * 
	 * Returns the cell opened, or BAD_RC_COORD if none is opened
	 **/
	rc_coord solver::step() {
		dbg::cout2 << ">";
		if(g.gamestate() == grid::NEW) {
			std::uniform_int_distribution<> uid_row(0, g.height() - 1);
			std::uniform_int_distribution<> uid_col(0, g.width() - 1);
			rc_coord cell(uid_row(rng), uid_col(rng));
			apply_open(cell);
			return cell;
		} else if(g.gamestate() != grid::RUNNING) {
			return BAD_RC_COORD;
		}


		rc_coord ret = step_certain();
		if(ret != BAD_RC_COORD) {
			return ret;
		}

		region remain = approx_remain();

		std::vector<rc_coord> best_locs;
		float best_prob = 2; //higher than any real probability could be
		float default_prob = (remain.min() + remain.max()) / (2.f * remain.size());
		constexpr float threshhold = .001;

		for(unsigned row = 0; row < g.height(); ++row) {
			for(unsigned col = 0; col < g.width(); ++col) {
				if(g.get(row,col) == grid::ms_hidden || g.get(row,col) == grid::ms_question) {
					const region_set::subset_type& regions_at = regions.regions_intersecting(rc_coord(row, col));
					float probability = regions_at.empty() ? default_prob : 0;
					for(region_set::iterator reg : regions_at) {
						probability = std::max((reg->min() + reg->max()) / (2.f * reg->size()), probability); //pick the worst probability
					}
					if(fabs(probability - best_prob) < threshhold) { //close enough in probability
						best_locs.push_back(rc_coord(row,col));
					} else if(probability < best_prob) {
						best_locs.clear();
						best_locs.push_back(rc_coord(row,col));
						best_prob = probability;
					}
				}
			}
		}

		std::vector<rc_coord> payout_locs;
		float best_payout = 0;
		if(best_locs.size() > 1 && fabs(best_prob - default_prob) > threshhold) {
			for(rc_coord cell : best_locs) {
				float payout = expected_payout(cell);
				if(fabs(best_payout - payout) < threshhold) {
					payout_locs.push_back(cell);
				} else if (payout >= best_payout) {
					payout_locs.clear();
					payout_locs.push_back(cell);
					best_payout = payout;
				} else if (payout < 0) {
					apply_flag(cell);
					return cell;
				}
			}
		} else {
			payout_locs = best_locs;
		}

		if(!payout_locs.empty()) {
			std::uniform_int_distribution<> uid(0, payout_locs.size() - 1);
			ret = payout_locs[uid(rng)];
			apply_open(ret);
			return ret;
		}
		return BAD_RC_COORD;
		
	}




}

