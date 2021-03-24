#include <STEM4U/Combinations.h>
#include <vector>
#include <numeric>

#include <Core/Core.h>

using namespace Upp;

// print out a range separated by commas,
//    return number of values printed.
template <class It>
unsigned Display(It begin, It end) {
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
    uint64 count;
public:
    explicit f(size_t l) : len(l), count(0) {}

    template <class It>
    bool operator()(It first, It last) {// called for each permutation
        ++count;						// count the number of times this is called
        UppLog() << "\n[ ";				// print out [first, mid) surrounded with [ ... ]
        size_t r = Display(first, last);
        if (r < len) {					// If [mid, last) is not empty, then print it out too
            UppLog() << " | ";			//     prefixed by " | "
            Display(last, std::next(last, len - r));
        }
        UppLog() << " ]";
        return false;  // Don't break out of the loop
    }

    operator uint64() const {return count;}
};

void TestCombinations() {
	UppLog() << "\n\nCombinations demo";
	
    const int r = 3;
    const int n = 6;
    
    Vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);		// 0, 1, 2, ...
    UppLog() << "\nInitial vector:\n[ ";
    Display(v.begin(), v.end());
    UppLog() << " ]\nCombinations:";
    
    uint64 count;
    
    count = ForEachPermutation(v, r, f(v.size()));
    VERIFY(count != 0 && count == CountEachPermutation(v, r));
    UppLog() << "\nFound " << count << " permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = ForEachReversiblePermutation(v, r, f(v.size()));
    VERIFY(count == CountEachReversiblePermutation(v, r));
    UppLog() << "\nFound " << count << " reversible permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = ForEachCircularPermutation(v, r, f(v.size()));
    VERIFY(count == CountEachCircularPermutation(v, r));
    UppLog() << "\nFound " << count << " circular permutations of " << v.size() << " objects taken " << r << " at a time.";
    
    count = ForEachReversibleCircularPermutation(v, r, f(v.size()));
    VERIFY(count == CountEachReversibleCircularPermutation(v, r));
    UppLog() << "\nFound " << count << " reversible circular permutations of " << v.size() << " objects taken " << r << " at a time.";    
}
