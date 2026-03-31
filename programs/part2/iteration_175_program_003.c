#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that triggers 128-bit helper generation */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper */
    return a / b;
}

/* Function with nothrow attribute */
void __attribute__((nothrow)) atomic_128bit_op(__int128 *val) {
    __int128 desired = *val + 1;
    __int128 expected = *val;
    
    /* Atomic compare-exchange on 128-bit may require helpers */
    __atomic_compare_exchange(val, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Header content that will be included in multiple TUs */
#ifndef TRIGGER_OPS_H
#define TRIGGER_OPS_H

static inline unsigned __int128 trigger_helper_gen(unsigned __int128 x) {
    /* Multiple operations that may require different helpers */
    unsigned __int128 result = x / 1000U;          /* __udivti3 */
    result = result % 999U;                       /* __umodti3 */
    result = result * result;                     /* __multi3 */
    return result;
}

#endif /* TRIGGER_OPS_H */

int main() {
    volatile __int128 v1 = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    __int128 v2 = ((__int128)0x1111111111111111 << 64) | 0x2222222222222222ULL;
    __int128 atomic_val = 0;
    
    /* Trigger 128-bit division with volatile operand */
    __int128 div_result = 0;
    for (int i = 0; i < 10; i++) {
        div_result = do_128bit_division(v1 + i, v2);
    }
    
    /* Atomic operations on 128-bit */
    for (int i = 0; i < 5; i++) {
        atomic_128bit_op(&atomic_val);
    }
    
    /* Use OpenMP target region if available */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: div_result)
    {
        /* Operations inside target region may generate helpers */
        div_result = div_result / 2;
    }
    #endif
    
    /* Use OpenACC if available */
    #ifdef _OPENACC
    #pragma acc parallel copy(div_result)
    {
        div_result = div_result + 1;
    }
    #endif
    
    /* Trigger helper from header */
    unsigned __int128 header_result = trigger_helper_gen(v1);
    
    /* Prevent dead code elimination */
    uint64_t hash[4] = {0};
    memcpy(&hash[0], &div_result, sizeof(div_result));
    memcpy(&hash[2], &header_result, sizeof(header_result));
    
    uint64_t final_hash = hash[0] ^ hash[1] ^ hash[2] ^ hash[3];
    printf("Result hash: 0x%016llx\n", (unsigned long long)final_hash);
    
    return 0;
}
