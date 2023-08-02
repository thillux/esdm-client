#pragma once

#include <iostream>
#include <boost/format.hpp>
#include <vector>
#include "config.h"

#ifdef JENT_KERNEL
#include <kcapi.h>
#endif


#ifdef JENT_KERNEL //build with '-DjentKernel' to make this available
void kcapiTest(size_t requestedNumberOfBytes = 32){
    struct kcapi_handle* jent_rng = NULL;
    int ret;

    ret = kcapi_rng_init(&jent_rng, "jitterentropy_rng", 0);
    if (ret != 0){
        std::cout << "ret(" << ret << ") != 0 -> error while calling kcapi_rng_init\n";
    }
    std::vector<uint8_t> e;
    e.resize(requestedNumberOfBytes);

    int ret2 = -1;
    ret2 = kcapi_rng_generate(jent_rng, e.data(), e.size());

    if (ret2 < 0){
        std::cout << "Error: " << ret2 << " < 0\n";
        return;
    }

    std::cout << "0x";
    for (size_t i = 0; i < e.size(); i++){
        std::cout << boost::format("%02x") % static_cast<int>(e[i]);
    }
    kcapi_rng_destroy(jent_rng);
    jent_rng = NULL;
}
#endif
