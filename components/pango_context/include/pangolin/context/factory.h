#pragma once

#include <pangolin/utils/shared.h>

#define PANGO_CREATE(x) Shared<x> x::Create(x::Params p)
