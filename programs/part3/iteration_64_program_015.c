/* double_int_coverage.c - Targets GCC's double_int::cmp function */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values - forces many comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison will use double_int::cmp internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Generate large 128-bit constants */
static unsigned __int128 make_large_constant(uint64_t hi, uint64_t lo) {
    return ((unsigned __int128)hi << 64) | lo;
}

int main(void) {
    /* Initialize with constants > 2^64 */
    unsigned __int128 values[8];
    
    /* Create values that require 128-bit representation */
    values[0] = make_large_constant(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
    values[1] = make_large_constant(0x0, 0xFFFFFFFFFFFFFFFFULL); /* Exactly 2^64-1 */
    values[2] = make_large_constant(0x1, 0x0); /* Exactly 2^64 */
    values[3] = values[0] + values[1]; /* Arithmetic that overflows 64 bits */
    values[4] = values[2] << 3; /* Shift producing large value */
    values[5] = values[0] - values[1]; /* May underflow but still 128-bit */
    values[6] = make_large_constant(0x8000000000000000ULL, 0x0); /* 2^127 */
    values[7] = make_large_constant(0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL); /* Max 128-bit */
    
    /* Force constant folding with arithmetic */
    unsigned __int128 folded = (values[0] * 0x100000001ULL) >> 2;
    checksum ^= folded;
    
    /* Sort the array - this performs many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets calculated from 128-bit values */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] ^= (char)(values[i] & 0xFF);
        checksum += (unsigned __int128)huge_array[(size_t)offset];
    }
    
    /* Loop with 128-bit counter and comparisons */
    unsigned __int128 start = make_large_constant(0x1000, 0x0);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i >> 64)) { /* Using high 64 bits for switch */
            case 0x1000:
                checksum += i * 2;
                break;
            case 0x1001:
                checksum += i * 3;
                break;
            case 0x1002:
                checksum += i * 5;
                break;
            default:
                checksum += i;
                break;
        }
        
        /* Structure with wide bit-fields */
        struct wide_bitfield wbf;
        wbf.a = (i >> 10) & ((1ULL << 70) - 1);
        wbf.b = (i >> 80) & ((1ULL << 58) - 1);
        wbf.c = (i >> 138) & ((1ULL << 72) - 1); /* Note: 138 = 10+58+70 */
        wbf.d = (i >> 210) & ((1ULL << 56) - 1); /* 210 = 138+72 */
        
        /* Operations on bit-fields that may trigger offset calculations */
        checksum += (unsigned __int128)wbf.a;
        checksum += (unsigned __int128)wbf.b;
        checksum += (unsigned __int128)wbf.c;
        checksum += (unsigned __int128)wbf.d;
        
        /* More 128-bit comparisons in loop condition */
        if (i > start + 500) {
            checksum ^= i;
        }
    }
    
    /* Additional comparisons in conditional expressions */
    unsigned __int128 x = make_large_constant(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
    unsigned __int128 y = make_large_constant(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAABULL);
    
    if (x < y) checksum += 1;
    if (x > y) checksum += 2;
    if (x == y) checksum += 3;
    if (x != y) checksum += 4;
    
    /* Final output to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
