// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

#include "../bsdiff.h"

namespace Upp {

String errMsg;

String BsGetLastError() {
	return errMsg;
}

bool Err(String str) {
	errMsg = str;
	return false;
}

}
