/* double_int_coverage.c
 * Designed to exercise GCC's double_int comparison logic
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to generate checksum to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values to force many comparisons */
void sort_128bit_array(unsigned __int128 arr[], int n) {
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

/* Function to access array using 128-bit offsets */
void access_with_128bit_offset(unsigned __int128 offset) {
    /* Modulo to stay within bounds */
    size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
    huge_array[safe_offset] ^= 0x55;  /* Modify to prevent optimization */
    checksum += huge_array[safe_offset];
}

int main(void) {
    /* Initialize huge_array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i += 4096) {
        huge_array[i] = (char)(i % 256);
    }

    /* Create 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Values that require full 128-bit comparison */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL,
        ((unsigned __int128)0x1000000000000000ULL << 64) | 0x0000000000000000ULL,
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that produce 128-bit results */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Operations that require 128-bit arithmetic */
        results[i*2] = constants[i] + ((unsigned __int128)i << 60);
        results[i*2 + 1] = constants[i] * 3 - ((unsigned __int128)1 << 63);
        
        /* Force comparisons in arithmetic */
        if (results[i*2] < results[i*2 + 1]) {
            results[i*2] ^= results[i*2 + 1];
        }
    }
    
    /* Sort the results - this will trigger many double_int comparisons */
    sort_128bit_array(results, num_constants * 2);
    
    /* Access array using 128-bit offsets derived from sorted values */
    for (int i = 0; i < num_constants * 2; i++) {
        access_with_128bit_offset(results[i]);
    }
    
    /* Loop with 128-bit counter - forces comparison in loop control */
    unsigned __int128 loop_start = ((unsigned __int128)0x1000000000000000ULL << 64);
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 7;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i >> 64)) {  /* Use high bits for switch */
            case 0x1000000000000000ULL:
                wbf.a = (i & (((unsigned __int128)1 << 70) - 1));
                checksum += wbf.a;
                break;
            case 0x1000000000000001ULL:
                wbf.b = (i & (((unsigned __int128)1 << 58) - 1));
                checksum += wbf.b;
                break;
            case 0x1000000000000002ULL:
                wbf.c = (i & (((unsigned __int128)1 << 72) - 1));
                checksum += wbf.c;
                break;
            case 0x1000000000000003ULL:
                wbf.d = (i & (((unsigned __int128)1 << 56) - 1));
                checksum += wbf.d;
                break;
            default:
                /* Access array with calculated offset */
                size_t offset = (size_t)(i % (sizeof(huge_array) / sizeof(huge_array[0])));
                checksum += huge_array[offset];
                break;
        }
        
        /* Additional comparisons in loop body */
        if (i > loop_start + 500) {
            wbf.a ^= wbf.b;
            wbf.c |= wbf.d;
        }
    }
    
    /* More direct comparisons to ensure coverage */
    unsigned __int128 compare_a = ((unsigned __int128)0xFFFFFFFF00000000ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    unsigned __int128 compare_b = ((unsigned __int128)0xFFFFFFFF00000000ULL << 64) | 0xBBBBBBBBBBBBBBBBULL;
    unsigned __int128 compare_c = ((unsigned __int128)0xFFFFFFFF00000001ULL << 64) | 0x0000000000000000ULL;
    
    /* Chain of comparisons that will test all branches */
    int cmp_results[6];
    cmp_results[0] = (compare_a < compare_b) ? -1 : (compare_a > compare_b) ? 1 : 0;
    cmp_results[1] = (compare_a < compare_c) ? -1 : (compare_a > compare_c) ? 1 : 0;
    cmp_results[2] = (compare_b < compare_c) ? -1 : (compare_b > compare_c) ? 1 : 0;
    cmp_results[3] = (compare_c < compare_a) ? -1 : (compare_c > compare_a) ? 1 : 0;
    cmp_results[4] = (compare_b < compare_a) ? -1 : (compare_b > compare_a) ? 1 : 0;
    cmp_results[5] = (compare_c < compare_b) ? -1 : (compare_c > compare_b) ? 1 : 0;
    
    /* Use comparison results */
    for (int i = 0; i < 6; i++) {
        checksum += cmp_results[i];
    }
    
    /* Final checksum output to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n",
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Verify array was accessed */
    printf("Array element 0: 0x%02X\n", (unsigned char)huge_array[0]);
    
    return 0;
}
