#pragma once

#define PANGO_CREATE(x) ExpectShared<x> x::Create(x::Params p)
