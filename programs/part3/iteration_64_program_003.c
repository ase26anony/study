/* wide-int-comparisons.c
 * Targets GCC's double_int::cmp function for coverage testing
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize -m32 -fdump-tree-optimized wide-int-comparisons.c -o wide-int-comparisons
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 20];  /* 1MB array for safe access */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 extra : 60;
    unsigned __int128 padding : 68;
} __attribute__((packed));

/* Function to perform bubble sort on 128-bit integers */
static void sort_128bit_array(unsigned __int128 *arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* This comparison triggers double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to calculate array offset using 128-bit arithmetic */
static size_t calculate_offset(unsigned __int128 base, 
                               unsigned __int128 index, 
                               unsigned __int128 stride) {
    /* 128-bit calculation that may overflow 64 bits */
    unsigned __int128 offset = base + index * stride;
    
    /* Modulo to keep within array bounds - triggers comparison */
    if (offset >= (unsigned __int128)(sizeof(huge_array))) {
        offset = offset % (unsigned __int128)(sizeof(huge_array));
    }
    
    return (size_t)offset;
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL,
        (unsigned __int128)1ULL << 80,
        (unsigned __int128)0xAAAAAAAAAAAAAAAALL << 32,
        (unsigned __int128)0x5555555555555555LL * 0x1000000000000000LL,
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL * 0x10ULL,
        (unsigned __int128)0x8000000000000000ULL << 32,
        (unsigned __int128)0x7FFFFFFFFFFFFFFFULL * 0x100ULL
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Operations that produce 128-bit results */
        results[i * 2] = constants[i] + (constants[(i + 1) % num_constants] << 3);
        results[i * 2 + 1] = (constants[i] * 0x12345ULL) - constants[(i + 2) % num_constants];
    }
    
    int total_results = num_constants * 2;
    
    /* 2. Sort the results - triggers many 128-bit comparisons */
    sort_128bit_array(results, total_results);
    
    /* 3. Array indexing with large offsets */
    unsigned char checksum = 0;
    
    for (int i = 0; i < total_results; i++) {
        /* Calculate offset using 128-bit arithmetic */
        size_t offset = calculate_offset(
            results[i], 
            (unsigned __int128)i * 0x1000ULL,
            (unsigned __int128)0x100ULL
        );
        
        /* Access array (initialize and read) */
        if (offset < sizeof(huge_array)) {
            huge_array[offset] = (unsigned char)(results[i] & 0xFF);
            checksum ^= huge_array[offset];
        }
    }
    
    /* 4. Loop with 128-bit counter and switch statement */
    unsigned __int128 loop_start = (unsigned __int128)0x100000000ULL << 32;
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 250;
    
    struct wide_bitfield wbf = {0};
    wbf.low_part = 0x123456789ABCDEFULL;
    wbf.high_part = 0xFEDCBA9876543ULL;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Switch with large 128-bit case values */
        switch (i) {
            case (unsigned __int128)0x100000000ULL << 32:  /* 2^64 */
                wbf.low_part += i & 0x3FFFFFFF;
                break;
            case (unsigned __int128)0x100000000ULL << 32 + 250:
                wbf.high_part ^= (i >> 32) & 0x3FFFFFF;
                break;
            case (unsigned __int128)0x100000000ULL << 32 + 500:
                wbf.extra = (wbf.low_part << 10) | (wbf.high_part >> 5);
                break;
            case (unsigned __int128)0x100000000ULL << 32 + 750:
                wbf.padding = ~(wbf.low_part | wbf.high_part | wbf.extra);
                break;
            default:
                /* Mix all bit-fields */
                wbf.low_part = (wbf.low_part ^ wbf.high_part) + wbf.extra;
                break;
        }
        
        /* Additional comparison in loop condition */
        if (i > loop_start + 500) {
            wbf.high_part = (wbf.high_part << 1) | (wbf.low_part >> 69);
            wbf.low_part <<= 1;
        }
    }
    
    /* 5. Final comparisons and checksum calculation */
    unsigned __int128 final_checksum = 0;
    
    /* Compare all pairs of results */
    for (int i = 0; i < total_results; i++) {
        for (int j = i + 1; j < total_results; j++) {
            /* More comparisons triggering double_int::cmp */
            if (results[i] < results[j]) {
                final_checksum += results[i];
            } else if (results[i] > results[j]) {
                final_checksum += results[j];
            } else {
                final_checksum += results[i] ^ results[j];
            }
        }
    }
    
    /* Add bit-field contributions */
    final_checksum += wbf.low_part;
    final_checksum += wbf.high_part << 16;
    final_checksum += wbf.extra << 32;
    final_checksum += wbf.padding << 48;
    
    /* Mix in the byte checksum */
    final_checksum ^= (unsigned __int128)checksum;
    
    /* Output to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(final_checksum >> 64),
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
