#include "region_set.h"

namespace ms {

/**
 * Initializes a region_set with keys for the given dimensions
 * 
 * attempting to add a region outside of these bounds is an error
 **/
region_set::region_set(unsigned height, unsigned width) : keys(boost::extents[height][width]) {}

/**
 * copies a region_set, including contents, keys, and modified regions
 * 
 * Complexity \f$O(N)\f$
 **/
region_set::region_set(const region_set& copy) : keys(boost::extents[copy.keys.shape()[0]][copy.keys.shape()[1]]) {
    contents = copy.contents;
    for(iterator it = contents.begin(); it != contents.end(); ++it) {
        for(rc_coord cell : *it) {
            keys[cell.row][cell.col].insert(it);
        }
    }
    const_iterator this_it = cbegin();
    const_iterator copy_it = copy.cbegin();
    assert(size() == copy.size());
    //equivalent regions should be in the same order
    for(; copy_it != copy.cend(); ++this_it, ++copy_it) {
        if(copy.modified_regions.find(copy_it) != copy.modified_regions.cend()) { //if *copy_it was modified
            modified_regions.insert(this_it);
        }
    }
}

/**
 * Adds a region to the list of regions, merging if a region already exists covering the same area.
 * Skips regions that offer no information. If a new region is added, add it to the appropriate cell_keys
 * 
 * `first` is `regions.end()` if region not added, otherwise returns an iterator to the region added/merged. 
 * 
 * `second` is `true` if the contents were modified.
 *
 * Complexity \f$O(N)\f$
 **/
std::pair<region_set::iterator,bool> region_set::add(const region& to_add) {
    if(!to_add.is_reasonable()) {
        throw bad_region_error("attempted to add invalid region");
    }

    typedef std::pair<iterator, bool> ret_type;

    if(!to_add.is_helpful()) {
        return ret_type(contents.end(), false);
    }

    iterator added;
    bool did_add = false;
    iterator similar = contents.lower_bound(to_add);

    region before_merge;
    if(!to_add.is_reasonable()) {
        debug_printf("!to_add.is_reasonable()\n");
        debug_printf("to_add: "); debug_print_region(to_add);
        debug_printf("before merge: "); debug_print_region(to_add);
        exit(1);
    }
    assert(to_add.is_reasonable());

    if(similar == contents.end() || !similar->samearea(to_add)) {
        auto added_info = contents.insert(to_add);
        assert(added_info.second);
        added = added_info.first;
        did_add = true;
        for(rc_coord cell : to_add) {
            keys[cell.row][cell.col].insert(added);
        }
    } else {
        before_merge = *similar;
        did_add = similar->min() < to_add.min() || similar->max() > to_add.max();
        order_preserve_merge(similar, to_add);
        added = similar;
    }

    if (did_add) {
        modified_regions.insert(added);
    }

    if(!added->is_reasonable()) {
        debug_printf("!added->is_reasonable()\n");
        debug_printf("to_add: "); debug_print_region(to_add);
        debug_printf("\nbefore merge: "); debug_print_region(before_merge);
        debug_printf("\nafter merge: "); debug_print_region(*added);
        exit(1);
    }
    assert(added->is_reasonable());
    return ret_type(added, did_add);
}

/**
 * Removes the specified region from the list of regions and all associated keys.
 * 
 * Complexity \f$O(log(N) + M)\f$ where \f$M\f$ is the size of the region and 
 * \f$N\f$ is the number of regions
 **/
region_set::iterator region_set::remove(iterator to_remove) {
    for(rc_coord cell : *to_remove) {
        key_type& key = keys[cell.row][cell.col];
        key_type::iterator remove_it = key.find(to_remove);
        assert(remove_it != key.end());
        key.erase(remove_it);
    }
    modified_regions.erase(to_remove);
    return contents.erase(to_remove);
}
void region_set::clear() {
    for(unsigned r = 0; r < keys.shape()[0]; ++r) {
        for(unsigned c = 0; c < keys.shape()[1]; ++c) {
            keys[r][c].clear();
        }
    }
    contents.clear();
    modified_regions.clear();
}

const region_set::subset_type& region_set::get_modified_regions() const {
    return modified_regions;
}
void region_set::reset_modified_regions() {
    modified_regions.clear();
}

/**
 * returns a container of type `subset_type` containing all regions that intersect 
 * the input region.
 * 
 * Complexity \f$O(M \cdot N)\f$ where \f$M\f$ is the size of 
 * the input region and \f$N\f$ is the number of regions.
 **/
region_set::subset_type region_set::regions_intersecting(const region& arg) const {
    subset_type ret;
    for(rc_coord cell : arg) {
        const key_type& key = keys[cell.row][cell.col];
        for(const iterator& elem : key) {
            ret.insert(elem);
        }
    }
    return ret;
}
const region_set::subset_type& region_set::regions_intersecting(rc_coord cell) const {
    return keys[cell.row][cell.col];
}

int region_set::remove_safe(rc_coord cell) {
    int removed = 0;
    std::vector<region> replacements;
    key_type& key = keys[cell.row][cell.col];
    while(!key.empty()) {
        key_type::iterator it = key.begin();
        region replace = **it;
        int err = replace.remove_safe(cell);
        (void) err; assert(err != 2 && "attempted to remove safe from region with no safe cells");
        replacements.push_back(std::move(replace));
        remove(*it);
        ++removed;
    }
    for(region& to_add : replacements) {
        add(std::move(to_add));
    }
    return removed;
}

int region_set::remove_bomb(rc_coord cell) {
    int removed = 0;
    std::vector<region> replacements;
    key_type& key = keys[cell.row][cell.col];
    while(!key.empty()) {
        key_type::iterator it = key.begin();
        region replace = **it;
        int err = replace.remove_bomb(cell);
        (void) err; assert(err != 2 && "attempted to remove bomb from region with no bomb cells");
        replacements.push_back(std::move(replace));
        remove(*it);
        ++removed;
    }
    for(region& to_add : replacements) {
        add(std::move(to_add));
    }
    return removed;
}

void region_set::order_preserve_merge(set_type::iterator to_change, const region& to_add) {
    assert(to_change->samearea(to_add));
    if(to_add._min > to_change->_min)
        to_change->_min = to_add._min;
    if(to_add._max < to_change->_max)
        to_change->_max = to_add._max;
}


}

