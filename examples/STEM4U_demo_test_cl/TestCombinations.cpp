#include <STEM4U/combinations.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cassert>

#include <Core/Core.h>

using namespace Upp;

// print out a range separated by commas,
//    return number of values printed.
template <class It>
unsigned display(It begin, It end) {
    unsigned r = 0;
    if (begin != end) {
        UppLog() << *begin;
        ++r;
        for (++begin; begin != end; ++begin) {
            UppLog() << ", " << *begin;
            ++r;
        }
    }
    return r;
}

// functor called for each permutation
class f {
    size_t len;
    std::uint64_t count;
public:
    explicit f(size_t l) : len(l), count(0) {}

    template <class It>
    bool operator()(It first, It last) {// called for each permutation
        ++count;						// count the number of times this is called
        UppLog() << "\n[ ";				// print out [first, mid) surrounded with [ ... ]
        size_t r = display(first, last);
        if (r < len) {					// If [mid, last) is not empty, then print it out too
            UppLog() << " | ";			//     prefixed by " | "
            display(last, std::next(last, len - r));
        }
        UppLog() << " ]";
        return false;  // Don't break out of the loop
    }

    operator std::uint64_t() const {return count;}
};

void TestCombinations() {
	UppLog() << "\n\nCombinations demo";
	
    const int r = 3;
    const int n = 6;
    
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    UppLog() << "\nInitial vector:\n[ ";
    display(v.begin(), v.end());
    UppLog() << " ]\nCombinations:";
    
    std::uint64_t count;
    
    count = for_each_permutation(v.begin(), v.begin() + r, v.end(), f(v.size()));
    VERIFY(count == count_each_permutation(v.begin(), v.begin() + r, v.end()));
    UppLog() << "\nFound " << count << " permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = for_each_reversible_permutation(v.begin(), v.begin() + r, v.end(), f(v.size()));
    VERIFY(count == count_each_reversible_permutation(v.begin(), v.begin() + r, v.end()));
    UppLog() << "\nFound " << count << " permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = for_each_circular_permutation(v.begin(), v.begin() + r, v.end(), f(v.size()));
    VERIFY(count == count_each_circular_permutation(v.begin(), v.begin() + r, v.end()));
    UppLog() << "\nFound " << count << " permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = for_each_reversible_circular_permutation(v.begin(), v.begin() + r, v.end(), f(v.size()));
    VERIFY(count == count_each_reversible_circular_permutation(v.begin(), v.begin() + r, v.end()));
    UppLog() << "\nFound " << count << " permutations of " << v.size() << " objects taken " << r << " at a time.";    
}
