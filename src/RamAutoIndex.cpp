/*
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All Rights reserved
 * 
 * The Universal Permissive License (UPL), Version 1.0
 * 
 * Subject to the condition set forth below, permission is hereby granted to any person obtaining a copy of this software,
 * associated documentation and/or data (collectively the "Software"), free of charge and under any and all copyright rights in the 
 * Software, and any and all patent rights owned or freely licensable by each licensor hereunder covering either (i) the unmodified 
 * Software as contributed to or provided by such licensor, or (ii) the Larger Works (as defined below), to deal in both
 * 
 * (a) the Software, and
 * (b) any piece of software and/or hardware listed in the lrgrwrks.txt file if one is included with the Software (each a “Larger
 * Work” to which the Software is contributed by such licensors),
 * 
 * without restriction, including without limitation the rights to copy, create derivative works of, display, perform, and 
 * distribute the Software and make, use, sell, offer for sale, import, export, have made, and have sold the Software and the 
 * Larger Work(s), and to sublicense the foregoing rights on either these or other terms.
 * 
 * This license is subject to the following condition:
 * The above copyright notice and either this complete permission notice or at a minimum a reference to the UPL must be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/************************************************************************
 *
 * @file RamAutoIndex.cpp
 *
 * Implements class for auto index generation of Ram index programs.
 * The minimal indexes are generated by solving a maximum matching 
 * problem.
 ***********************************************************************/

#include "RamAutoIndex.h"

/*
 * Class IndexSet 
 */ 

/** map the keys in the key set to lexicographical order */
void RamAutoIndex::solve() { 
    if(searches.size() == 0) return;

//    // -- hack to disable indexing --
//
//    // every search pattern gets its naive index
//    for(SearchColumns cur : searches) {
//
//        // obtain order
//        LexicographicalOrder order;
//        SearchColumns mask = cur;
//        for(int i=0; mask != 0; i++) {
//            if (!(1<<i & mask)) continue;
//            order.push_back(i);
//            // clear bit
//            mask &= ~(1<<i);
//        }
//
//        // add new order
//        orders.push_back(order);
//
//        // register pseudo chain
//        chainToOrder.push_back(Chain());
//        chainToOrder.back().insert(cur);
//    }
//
//    std::cout << "Orders: " << orders << "\n";
//    std::cout << "Chains: " << chainToOrder << "\n";
//
//    return;
//
//    // ------------------------------



    // Construct the matching poblem
    for (SearchSet::const_iterator it = searches.begin(); it != searches.end(); ++it) {
        // For this node check if other nodes are strict subsets
        for (SearchSet::const_iterator itt = searches.begin(); itt != searches.end(); ++itt) {
            if (isStrictSubset(*it, *itt)) {
                matching.addEdge(*it, toB(*itt));
            }
        }
    } 

    // Perform the hopcroft-karp on the graph and receive matchings (mapped A-> B and B->A)
    // Assume: alg.calculate is not called on an empty graph
    ASSERT(searches.size() > 0);
    const RamMaxMatching::Matchings& matchings = matching.calculate();

    // Extract the chains given the nodes and matchings
    const ChainOrderMap chains = getChainsFromMatching(matchings, searches);

    // Should never get no chains back as we never call calculate on an empty graph
    ASSERT(chains.size() > 0);

    for (ChainOrderMap::const_iterator it = chains.begin(); it != chains.end(); ++it) {
        std::vector<int> ids;
        SearchColumns initDelta = *(it->begin());
        insertIndex(ids, initDelta);

        for (Chain::iterator iit = it->begin(); next(iit) != it->end(); ++iit) {
            SearchColumns delta = *(next(iit)) - *iit; 
            insertIndex(ids, delta);
        }

        ASSERT(ids.size() > 0);

        orders.push_back(ids);
    }

    // Construct the matching poblem
    for (SearchSet::const_iterator it = searches.begin(); it != searches.end(); ++it) {
        int idx = map(*it); 
        size_t l = card(*it); 
        SearchColumns k = 0; 
        for (size_t i=0;i<l;i++) { 
            k = k + (1 << (orders[idx][i])); 
        }
        ASSERT(k == *it && "incorrect lexicographical order");  
    } 
}


/** given an unmapped node from set A we follow it from set B until it cannot be matched from B
  if not mateched from B then umn is a chain*/
RamAutoIndex::Chain RamAutoIndex::getChain(const SearchColumns umn, const RamMaxMatching::Matchings& match) {
    SearchColumns start = umn; // start at an unmateched node
    Chain chain;
    // Assume : no circular mappings, i.e. a in A -> b in B -> ........ -> a in A is not allowed.
    // Given this, the loop will terminate
    while (true) {
        RamMaxMatching::Matchings::const_iterator mit = match.find(toB(start)); // we start from B side
        chain.insert(start);

        if(mit == match.end()) {
            return chain;
        }

        SearchColumns a = mit->second;
        chain.insert(a);
        start = a; 
    }
}

/** get all chains from the matching */
const RamAutoIndex::ChainOrderMap RamAutoIndex::getChainsFromMatching(const RamMaxMatching::Matchings& match, const SearchSet& nodes) {
    ASSERT(nodes.size() > 0);

    // Get all unmatched nodes from A
    const SearchSet& umKeys = getUnmatchedKeys(match, nodes);

    // Case: if no unmatched nodes then we have an anti-chain
    if (umKeys.size() == 0){ 
        for (SearchSet::const_iterator nit = nodes.begin(); nit != nodes.end(); ++nit) {
            std::set<SearchColumns> a;
            a.insert(*nit);
            chainToOrder.push_back(a);
            return chainToOrder;
        }
    }

    ASSERT(umKeys.size() > 0);

    // A worklist of used nodes
    SearchSet usedKeys;

    // Case: nodes < umKeys or if nodes == umKeys then anti chain - this is handled by this loop
    for (SearchSet::iterator it = umKeys.begin(); it != umKeys.end(); ++it) {
        Chain c = getChain(*it, match);
        ASSERT(c.size() > 0);
        chainToOrder.push_back(c);
    }

    ASSERT(chainToOrder.size() > 0);

    return chainToOrder;
}