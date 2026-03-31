/* double_int_coverage.c
 * Targets GCC's double_int::cmp() function for coverage testing
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-original -fdump-tree-optimized double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to generate checksum to prevent dead code elimination */
static unsigned __int128 compute_checksum(unsigned __int128 *arr, size_t n) {
    unsigned __int128 sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum = (sum * 0x123456789ABCDEFULL) + arr[i];
    }
    return sum;
}

/* Simple bubble sort to force many 128-bit comparisons */
static void sort_128bit_array(unsigned __int128 *arr, size_t n) {
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            /* This comparison will use double_int::cmp() internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        /* Values that require full 128-bit representation */
        0x123456789ABCDEF0ULL * 0x100000001ULL,                    /* > 2^64 */
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x1ULL, /* 2^64 + 1 */
        0xDEADBEEFCAFEBABEULL * 0x123456789ULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        0xFFFFFFFFFFFFFFFFULL,
        0x10000000000000000ULL,  /* Exactly 2^64 */
        0,
        ((unsigned __int128)0x123456789ABCDEFULL << 60) | 0xFEDCBA987654321ULL,
        0x7FFFFFFFFFFFFFFFULL * 2ULL,  /* Causes overflow into high bits */
        ((unsigned __int128)0x1ULL << 127) - 1,  /* Max signed 128-bit */
    };
    
    const size_t num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[20];
    size_t result_idx = 0;
    
    for (size_t i = 0; i < num_constants; i++) {
        /* Various operations that produce 128-bit results */
        results[result_idx++] = constants[i] + 0x1000000000000000ULL;
        results[result_idx++] = constants[i] * 3ULL;
        results[result_idx++] = constants[i] << 3;
        results[result_idx++] = constants[i] - 0x8000000000000000ULL;
    }
    
    const size_t num_results = result_idx;
    
    /* 2. Sort the results - forces many 128-bit comparisons */
    sort_128bit_array(results, num_results);
    
    /* 3. Array indexing with large offsets */
    unsigned __int128 offset_sum = 0;
    for (size_t i = 0; i < num_results; i++) {
        /* Calculate offset using 128-bit arithmetic, then modulo to stay in bounds */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] ^= (char)(results[i] & 0xFF);
        offset_sum += offset;
    }
    
    /* 4. Loop with 128-bit counter and switch statement */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = ((unsigned __int128)0x1000000000000000ULL << 64) | 0x8000000000000000ULL;
    unsigned __int128 end = start + 100;
    unsigned __int128 step = 7;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch (i & 0xF) {  /* Use lower bits for manageable switch */
            case 0:
                loop_checksum += i * 2;
                break;
            case 1:
                loop_checksum += i >> 1;
                break;
            case 2:
                loop_checksum += i + 0x123456789ABCDEFULL;
                break;
            case 3:
                loop_checksum += i - 0xFEDCBA987654321ULL;
                break;
            case 4:
                loop_checksum ^= i;
                break;
            case 5:
                loop_checksum = (loop_checksum << 3) | (i & 0x7);
                break;
            case 6:
                loop_checksum += i * i;
                break;
            case 7:
                loop_checksum -= i / 3;
                break;
            default:
                loop_checksum += 0x123456789ABCDEFULL;
                break;
        }
        
        /* 5. Structure with wide bit-fields */
        struct wide_bitfield wbf;
        wbf.a = (i >> 58) & ((1ULL << 70) - 1);
        wbf.b = (i >> 0) & ((1ULL << 58) - 1);
        wbf.c = (i << 8) & (((unsigned __int128)1 << 120) - 1);
        wbf.d = (i >> 120) & 0xFF;
        
        /* Manipulate the bit-fields */
        wbf.a ^= wbf.b;
        wbf.c |= wbf.d;
        loop_checksum += (unsigned __int128)wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* 6. Final checksum calculation to prevent optimization */
    unsigned __int128 final_checksum = compute_checksum(results, num_results);
    final_checksum ^= offset_sum;
    final_checksum += loop_checksum;
    
    /* Add array content to checksum */
    for (size_t i = 0; i < 256; i++) {
        final_checksum = (final_checksum * 0x1234567) + huge_array[i];
    }
    
    /* Output to prevent dead code elimination */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(final_checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n",
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Additional comparisons to ensure coverage */
    unsigned __int128 test_vals[4] = {
        0,
        ((unsigned __int128)1 << 127),
        0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL
    };
    
    /* Force comparisons in different contexts */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (test_vals[i] < test_vals[j]) {
                final_checksum += 1;
            }
            if (test_vals[i] > test_vals[j]) {
                final_checksum += 2;
            }
            if (test_vals[i] == test_vals[j]) {
                final_checksum += 3;
            }
        }
    }
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
