#pragma once
#include "core/TeachingState.h"
#include <string>

bool teachingToJson(const TeachingState& s, std::string* out);
bool teachingFromJson(const std::string& in, TeachingState* out);
