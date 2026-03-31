/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accum sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;    /* MAX short _Fract */
    const signed short _Fract sf_min = -1.0r;                 /* MIN short _Fract */
    const signed _Accum sa_max = 32767.999969482421875k;      /* MAX signed _Accum (Q15.16) */
    const signed _Accum sa_min = -32768.0k;                   /* MIN signed _Accum */
    
    /* These will trigger range checks during conversion */
    volatile signed short _Fract test1 = (signed short _Fract)sa_max;  /* Should overflow */
    volatile signed short _Fract test2 = (signed short _Fract)sa_min;  /* Should overflow */
    
    /* Test 2: Values just at/over boundaries */
    const signed _Accum sa_near_max = 32767.999847412109375k;  /* Just below MAX that fits in short _Fract */
    const signed _Accum sa_over_max = 32768.0k;                /* Just over MAX */
    
    volatile signed short _Fract test3 = (signed short _Fract)sa_near_max;  /* Should work */
    volatile signed short _Fract test4 = (signed short _Fract)sa_over_max;  /* Should overflow */
    
    /* Test 3: Complex constant expressions forcing range checks */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 65535.0r;  /* Large value */
    constexpr signed short _Fract c2 = (signed short _Fract)c1;   /* Conversion check */
    
    /* Test 4: Mixed signed/unsigned with boundary checks */
    const unsigned short _Fract uf_max = 0.999969482421875ur;     /* MAX unsigned short _Fract */
    const signed _Accum sa_pos = 1.5k;
    
    /* This conversion requires checking against unsigned max */
    volatile unsigned short _Fract test5 = (unsigned short _Fract)sa_pos;
    
    /* Test 5: Negative to unsigned (should trigger min check) */
    const signed _Accum sa_neg = -1.0k;
    volatile unsigned short _Fract test6 = (unsigned short _Fract)sa_neg;  /* Underflow */
    
    /* Test 6: Arithmetic that overflows then conversion */
    const signed short _Fract a = 0.75r;
    const signed short _Fract b = 0.75r;
    const signed _Accum product = (signed _Accum)a * (signed _Accum)b * 2.0k;
    volatile signed short _Fract test7 = (signed short _Fract)product;  /* 1.125 > 1.0 */
    
    /* Test 7: Saturation qualifier tests */
    signed _Sat short _Fract sat1 = 2.0r;  /* Should saturate to MAX */
    unsigned _Sat short _Fract sat2 = -0.5ur;  /* Should saturate to 0 */
    
    /* Force evaluation with volatile */
    volatile signed _Sat short _Fract vsat1 = sat1;
    volatile unsigned _Sat short _Fract vsat2 = sat2;
    
    /* Test 8: Loop with boundary accumulation */
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 10; i++) {
        accumulator += 4000.0k;  /* Will exceed short _Fract range */
    }
    volatile signed short _Fract test8 = (signed short _Fract)accumulator;
    
    /* Test 9: Exact boundary case - maximum representable in target */
    /* For signed short _Fract to signed _Accum conversion check */
    const signed long _Accum sla_huge = 140737488355327.9999847412109375lr;  /* Large value */
    volatile signed _Accum test9 = (signed _Accum)sla_huge;  /* Range check */
    
    /* Test 10: Minimum signed value check */
    const signed long _Accum sla_neg_huge = -140737488355328.0lr;
    volatile signed _Accum test10 = (signed _Accum)sla_neg_huge;  /* Range check */
    
    /* Test 11: Using function calls to get boundary values */
    volatile signed short _Fract test11 = get_sfract_max();
    volatile signed _Accum test12 = (signed _Accum)test11 * 1.5k;
    volatile signed short _Fract test13 = (signed short _Fract)test12;  /* Overflow */
    
    /* Test 12: Unsigned maximum boundary */
    volatile unsigned short _Fract test14 = get_ufract_max();
    volatile signed _Accum test15 = (signed _Accum)test14 + 0.1k;
    volatile unsigned short _Fract test16 = (unsigned short _Fract)test15;  /* Overflow */
    
    /* Test 13: Minimum signed accumulation */
    volatile signed long _Accum test17 = get_saccum_min();
    volatile signed _Accum test18 = (signed _Accum)test17;  /* Should work or overflow */
    
    /* Create observable output to prevent elimination */
    uint32_t checksum = 0;
    checksum += *(uint16_t*)&test1;
    checksum += *(uint16_t*)&test2;
    checksum += *(uint16_t*)&test3;
    checksum += *(uint16_t*)&test4;
    checksum += *(uint16_t*)&test5;
    checksum += *(uint16_t*)&test6;
    checksum += *(uint16_t*)&test7;
    checksum += *(uint16_t*)&vsat1;
    checksum += *(uint16_t*)&vsat2;
    checksum += *(uint16_t*)&test8;
    checksum += *(uint32_t*)&test9;
    checksum += *(uint32_t*)&test10;
    checksum += *(uint16_t*)&test11;
    checksum += *(uint32_t*)&test12;
    checksum += *(uint16_t*)&test13;
    checksum += *(uint16_t*)&test14;
    checksum += *(uint32_t*)&test15;
    checksum += *(uint16_t*)&test16;
    checksum += *(uint64_t*)&test17;
    checksum += *(uint32_t*)&test18;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
