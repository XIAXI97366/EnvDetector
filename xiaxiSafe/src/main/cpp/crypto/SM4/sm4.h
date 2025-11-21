//
// Created by xiaobai on 2021/8/30.
//

#ifndef CMAKEDATA_SM4_H
#define CMAKEDATA_SM4_H
# pragma once

#include <stddef.h>
#include <stdint.h>

# define SM4_ENCRYPT     1
# define SM4_DECRYPT     0

# define SM4_BLOCK_SIZE    16
# define SM4_KEY_SCHEDULE  32

typedef struct SM4_KEY_st {
    uint32_t rk[SM4_KEY_SCHEDULE];
} SM4_KEY;



#ifdef __cplusplus
extern "C"{
#endif
    int sm4_set_key(const uint8_t *key, SM4_KEY *ks);

    void sm4_encrypt(const uint8_t *in, uint8_t *out, const SM4_KEY *ks);

    void sm4_decrypt(const uint8_t *in, uint8_t *out, const SM4_KEY *ks);
#ifdef __cplusplus
}
#endif

#endif //CMAKEDATA_SM4_H
