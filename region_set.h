#ifndef MS_REGION_SET_H
#define MS_REGION_SET_H


#include "region.h"
#include <set>
#include <boost/multi_array.hpp>

namespace ms {

/**
 * lexicographically compare cells in a region
 * 
 * \note only cells are compared, not min or max. thus, if `a.samearea(b)` then 
 * `!(a < b || b < a)` and they are equivalent in a set. this is a desirable characteristic
 * because it allows easy searching, but must be careful when adding to a set that you don't
 * "lose" your region
 **/
struct region_cmp_no_min_max {
    bool operator()(const region& arg1, const region& arg2) {
		return arg1._cells < arg2._cells;
    }
};

struct region_iter_cmp_no_min_max {
    bool operator()(std::set<region, region_cmp_no_min_max>::iterator arg1, std::set<region, region_cmp_no_min_max>::iterator arg2) {
        return region_cmp_no_min_max(*arg1, *arg2);
    }
}

class region_key;

class region_set {
public:
    typedef std::set<region, region_cmp_no_min_max> set_type;

    typedef set_type::iterator iterator;
    typedef set_type::const_iterator const_iterator;
    typedef set_type::reverse_iterator reverse_iterator;
    typedef set_type::const_reverse_iterator const_reverse_iterator;

    typedef std::set<iterator, region_iter_cmp_no_min_max> subset_type;
    typedef subset_type key_type;


    region_set(unsigned height, unsigned width);

    std::pair<iterator,bool> add(const region&);
    iterator remove(iterator);
    void clear();

    const subset_type& get_modified_regions() const;
    void reset_modified_regions();

    subset_type regions_intersecting(const region&) const;

    int remove_safe(rc_coord);
    int remove_bomb(rc_coord);
    
    size_t size() const { return contents.size(); }
    bool empty() const { return contents.empty(); }
    iterator begin() { return contents.begin(); }
    iterator end() { return contents.end(); }
    const_iterator cbegin() const { return contents.cbegin(); }
    const_iterator cend() const { return contents.cend(); }
    reverse_iterator rbegin() { return contents.rbegin(); }
    reverse_iterator rend() { return contents.rend(); }
    const_reverse_iterator crbegin() const { return contents.rcbegin(); }
    const_reverse_iterator crend() const { return contents.rcend(); }

private:

    set_type contents;
    subset_type modified_regions;    
    boost::multi_array<key_type, 2> keys;

    void order_preserve_merge(set_type::iterator, const region&);
    void indicate_modified(set_type::iterator);
};

class region_key {
private:
    friend region_set;
public:

    
}


}

#endif //MS_REGION_SET_H