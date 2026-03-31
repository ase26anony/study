/* wide-int-comparisons.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-comparisons.c -o wide-int-comparisons
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 20]; /* 1MB array for safe access */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 full : 128;
} __attribute__((packed));

/* Function to perform bubble sort on 128-bit integers */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison triggers double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function that uses switch with large 128-bit case labels */
static unsigned __int128 process_with_switch(unsigned __int128 value) {
    /* These large constants force 128-bit representation */
    const unsigned __int128 CASE_A = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const unsigned __int128 CASE_B = ((unsigned __int128)0x9876543210FEDCBAULL << 64) | 0x0123456789ABCDEFULL;
    const unsigned __int128 CASE_C = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    const unsigned __int128 CASE_D = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL;
    
    /* Switch statement triggers sorting/comparison of case labels */
    switch (value) {
        case 0: return value + 1;
        case 1: return value * 2;
        /* Large 128-bit case constants */
        case ((unsigned __int128)CASE_A & 0xFFFFFFFFFFFFFFF0ULL): 
            return value >> 4;
        case ((unsigned __int128)CASE_B & 0xFFFFFFFFFFFFFF00ULL):
            return value << 2;
        case ((unsigned __int128)CASE_C):
            return ~value;
        case ((unsigned __int128)CASE_D):
            return value | 0x5555555555555555ULL;
        default:
            return value ^ 0xAAAAAAAAAAAAAAAAULL;
    }
}

int main(void) {
    /* Initialize huge array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 31);
    }
    
    /* Create 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Constants that require full 128-bit representation */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0x9876543210FEDCBAULL << 64) | 0x0123456789ABCDEFULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        0, /* Include zero for boundary testing */
        ~((unsigned __int128)0), /* Max value */
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations that generate 128-bit results */
    unsigned __int128 computed[16];
    int comp_idx = 0;
    
    for (int i = 0; i < num_constants && comp_idx < 16; i++) {
        /* Operations that require 128-bit arithmetic */
        computed[comp_idx++] = constants[i] + 0x100000001ULL;
        computed[comp_idx++] = constants[i] * 3ULL;
        computed[comp_idx++] = constants[i] << 3;
        computed[comp_idx++] = ~constants[i];
    }
    
    /* 2. Sort the computed values - triggers many comparisons */
    sort_128bit_array(computed, comp_idx);
    
    /* 3. Array indexing with large offsets derived from 128-bit values */
    unsigned long checksum = 0;
    for (int i = 0; i < comp_idx; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = computed[i] % sizeof(huge_array);
        checksum += (unsigned long)huge_array[(size_t)offset];
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 loop_start = ((unsigned __int128)0x100000000ULL << 32) | 0x80000000ULL;
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 100;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* This comparison in loop condition triggers double_int::cmp */
        
        /* Access array using 128-bit derived offset */
        unsigned __int128 idx = i % sizeof(huge_array);
        checksum += (unsigned long)huge_array[(size_t)idx];
        
        /* Process with switch statement */
        unsigned __int128 processed = process_with_switch(i);
        checksum += (unsigned long)(processed & 0xFFFFFFFFULL);
    }
    
    /* 5. Structure with wide bit-fields */
    struct wide_bitfield wbf;
    memset(&wbf, 0, sizeof(wbf));
    
    /* Set bit-fields using 128-bit constants */
    wbf.low_part = ((unsigned __int128)0x1FFFFFFFFFFFFFFFULL) & (((unsigned __int128)1 << 70) - 1);
    wbf.high_part = ((unsigned __int128)0x3FFFFFFFFFFFFFFFULL) & (((unsigned __int128)1 << 58) - 1);
    wbf.full = (wbf.high_part << 70) | wbf.low_part;
    
    /* Compare structure fields with 128-bit constants */
    unsigned __int128 compare_val = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    if (wbf.full < compare_val) {
        checksum += 1000;
    } else if (wbf.full > compare_val) {
        checksum += 2000;
    } else {
        checksum += 3000;
    }
    
    /* 6. Additional comparisons in different contexts */
    unsigned __int128 sorted_copy[16];
    memcpy(sorted_copy, computed, sizeof(computed[0]) * comp_idx);
    
    /* Binary search style comparisons */
    int left = 0, right = comp_idx - 1;
    unsigned __int128 search_key = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        /* More comparisons triggering double_int::cmp */
        if (sorted_copy[mid] == search_key) {
            checksum += 5000;
            break;
        }
        if (sorted_copy[mid] < search_key) {
            left = mid + 1;
            checksum += 100;
        } else {
            right = mid - 1;
            checksum += 200;
        }
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
