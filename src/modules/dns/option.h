#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFDNSShowType {
    FF_DNS_TYPE_IPV4_BIT = 1,
    FF_DNS_TYPE_IPV6_BIT = 2,
    FF_DNS_TYPE_BOTH = FF_DNS_TYPE_IPV4_BIT | FF_DNS_TYPE_IPV6_BIT,
    FF_DNS_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFDNSShowType;

typedef struct FFDNSOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFDNSShowType showType;
} FFDNSOptions;
