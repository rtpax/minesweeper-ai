/**
 * Conglomerate holds a number of regions, and as each is added the union is calculated
 * 
 * The union of regions is not associative, AUBUC is more strict than (AUB)UC or AU(BUC).
 * The goal of the conglomerate class is to mitigate this effect and get the most information
 * possible from the union of multiple regions while remaining computationally feasable.
 * Calculating every possible order of unions is not a good solutions since it has a high complexity
 * (\f$O(N!)\f$) and it would still not be guaranteed to be as strict as possible 
 * 
 * The model for conglomerate is as follows.
 * As each region is added:
 *     Calculate the the intersect of the input and the conglomerate as a whole
 *     
 * 
 * To preserve information, regions can only be added to a conglomerate, not removed.
 * When added, regions are compared not only against the conglomerate as a whole, but against
 * the individual regions within the conglomerate
 * 
 **/
class conglomerate {
    
}

