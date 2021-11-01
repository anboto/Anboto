// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
	
UPP::String ToString() const {
	std::stringstream s;
	s << *this;
	return s.str();
}

