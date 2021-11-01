// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
#include <Core/Core.h>

namespace Upp {

void OrderToConnections(const Vector<int> &order, Vector<int> &left, Vector<int> &right) {
	left = clone(order);
	left.Remove(left.size()-1);
	right = clone(order);
	right.Remove(0);
}

void ConnectionsToOrder(const Vector<int> &left, const Vector<int> &right, Vector<int> &order) {
	int next = 0;
	Vector<bool> removed(left.size(), false);
	for (int id = 0; id <= order.size(); id++) {
		for (int i = 0; i < left.size(); ++i) {
			if (!removed[i]) {
				if (left[i] == next) { 
					order[id] = next;
					next = right[i];
					removed[i] = true;
					break;
				} else if (right[i] == next) {
					order[id] = next;
					next = left[i];
					removed[i] = true;
					break;
				}
			}
		}
	}
}
	
}