// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
#ifndef _STEM4U_Combinations_h_
#define _STEM4U_Combinations_h_

#include "Combinations/combinations.h"
#include <Core/Core.h>

namespace Upp {


template <class T, class Functor>
uint64 ForEachPermutation(T &data, int mid, Functor f) {
	try {
		return for_each_permutation(data.begin(), data.begin() + mid, data.end(), f);
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T, class Functor>
uint64 ForEachPermutation(T &data, Functor f) {
	return ForEachPermutation(data, f);
}

template <class T, class Functor>
uint64 ForEachReversiblePermutation(T &data, int mid, Functor f) {
	try {
		return for_each_reversible_permutation(data.begin(), data.begin() + mid, data.end(), f);
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T, class Functor>
uint64 ForEachReversiblePermutation(T &data, Functor f) {
	return ForEachPermutation(data, f);
}

template <class T, class Functor>
uint64 ForEachCircularPermutation(T &data, int mid, Functor f) {
	try {
		return for_each_circular_permutation(data.begin(), data.begin() + mid, data.end(), f);
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T, class Functor>
uint64 ForEachCircularPermutation(T &data, Functor f) {
	return ForEachPermutation(data, f);
}

template <class T, class Functor>
uint64 ForEachReversibleCircularPermutation(T &data, int mid, Functor f) {
	try {
		return for_each_reversible_circular_permutation(data.begin(), data.begin() + mid, data.end(), f);
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T, class Functor>
uint64 ForEachReversibleCircularPermutation(T &data, Functor f) {
	return ForEachReversibleCircularPermutation(data, f);
}

template <class T, class Functor>
uint64 ForEachCombination(T &data, int mid, Functor f) {
	try {
		return for_each_combination(data.begin(), data.begin() + mid, data.end(), f);
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T, class Functor>
uint64 ForEachCombination(T &data, Functor f) {
	return ForEachPermutation(data, f);
}


template <class T>
uint64 CountEachPermutation(const T &data, int mid) {
	try {
		return count_each_permutation(data.begin(), data.begin() + mid, data.end());
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T>
uint64 CountEachPermutation(const T &data) {
	return count_each_permutation(data.begin(), data.begin() + data.size(), data.end());
}

template <class T>
uint64 CountEachReversiblePermutation(const T &data, int mid) {
	try {
		return count_each_reversible_permutation(data.begin(), data.begin() + mid, data.end());
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T>
uint64 CountEachReversiblePermutation(const T &data) {
	return count_each_reversible_permutation(data.begin(), data.begin() + data.size(), data.end());
}

template <class T>
uint64 CountEachCircularPermutation(const T &data, int mid) {
	try {
		return count_each_circular_permutation(data.begin(), data.begin() + mid, data.end());
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T>
uint64 CountEachCircularPermutation(const T &data) {
	return count_each_circular_permutation(data.begin(), data.begin() + data.size(), data.end());
}

template <class T>
uint64 CountEachReversibleCircularPermutation(const T &data, int mid) {
	try {
		return count_each_reversible_circular_permutation(data.begin(), data.begin() + mid, data.end());
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T>
uint64 CountEachReversibleCircularPermutation(const T &data) {
	return count_each_reversible_circular_permutation(data.begin(), data.begin() + data.size(), data.end());
}

template <class T>
uint64 CountEachCombinationPermutation(const T &data, int mid) {
	try {
		return count_each_combination(data.begin(), data.begin() + mid, data.end());
	} catch (const std::invalid_argument &) {
		return 0;
	} catch (const std::overflow_error &) {
		return 0;
	}
}

template <class T>
uint64 CountEachCombinationPermutation(const T &data) {
	return count_each_combination(data.begin(), data.begin() + data.size(), data.end());
}


template <class UInt>
UInt CountEachPermutation(UInt d1, UInt d2) {
	return count_each_permutation(d1, d2);
}

template <class UInt>
UInt CountEachReversiblePermutation(UInt d1, UInt d2) {
	return count_each_reversible_permutation(d1, d2);
}

template <class UInt>
UInt CountEachCircularPermutation(UInt d1, UInt d2) {
	return count_each_circular_permutation(d1, d2);
}

template <class UInt>
UInt CountEachReversibleCircularPermutation(UInt d1, UInt d2) {
	return count_each_reversible_circular_permutation(d1, d2);
}

template <class UInt>
UInt CountEachCombination(UInt d1, UInt d2) {
	return count_each_combination(d1, d2);
}

	
}
	
#endif
