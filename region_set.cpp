#include "region_set.h"

namespace ms {


region_set::region_set(unsigned height, unsigned width) : keys(boost::extents[height][width]) {}

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
std::pair<region_set::iterator,bool> region_set::add(const region&) {
    typedef std::pair<iterator, bool> ret_type;

    if(!to_add.is_helpful()) {
        return ret_type(contents.end(), false);
    }

    iterator added;
    bool did_add = false;
    iterator similar = contents.lower_bound(to_add);
    if(similar == contents.end() || !similar->samearea(to_add)) {
        auto added_info = contents.insert(to_add);
        assert(added_info.second);
        added = added_info.first;
        did_add = true;
        for(rc_coord cell : to_add) {
            keys[cell.row][cell.col].insert(added);
        }
    } else {
        did_add = similar->min() < to_add.min() || similar->max() > to_add.max();
        order_preserve_merge_to(similar, to_add);
        added = similar;
    }

    if (did_add) {
        
    }
    
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
    return regions.erase(to_remove);
}
void region_set::clear() {
    for(key_type& key : keys) {
        key.clear();
    }
    contents.clear();
}

const region_set::subset_type& region_set::get_modified_regions() const {
    
}
void region_set::reset_modified_regions();

region_set::subset_type region_set::regions_intersecting(const region&) const;

int remove_safe(rc_coord);
int remove_bomb(rc_coord);




}

