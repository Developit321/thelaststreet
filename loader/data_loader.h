#pragma once

#include <string>

#include "tls/street.h"

// Reads the files in data/ into core types. This is the only code that
// knows about JSON; in Unreal it is replaced by Unreal's own JSON reader.

struct StreetLoadResult {
    bool ok = false;
    std::string error;
    tls::StreetLayout street;
};

StreetLoadResult LoadStreet(const std::string& path);
