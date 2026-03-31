/* double_int_coverage.c
 * Designed to exercise GCC's internal double_int comparison logic
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-original -fdump-tree-optimized double_int_coverage.c -o double_int_coverage
 * For 32-bit: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized double_int_coverage.c -o double_int_coverage_32
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 120;
    unsigned __int128 d: 8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for unsigned __int128 values */
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

/* Function to access array using 128-bit offsets */
static void access_with_128bit_offset(unsigned __int128 offset) {
    /* Modulo to stay within bounds */
    size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
    huge_array[safe_offset] ^= 0x55;  /* Modify array content */
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL,
        ((unsigned __int128)1 << 80) | 0x123456789ABCDEF0ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL,
        ((unsigned __int128)0x8888888888888888ULL << 64) | 0x9999999999999999ULL,
        (unsigned __int128)0x7FFFFFFFFFFFFFFFULL * 0x10ULL,
        ~((unsigned __int128)0) - 1000,
        ((unsigned __int128)1 << 127) - 1,
        (unsigned __int128)0x5555555555555555ULL * 0x200000002ULL,
        (unsigned __int128)0xAAAAAAAAAAAAAAAALL * 0x300000003ULL
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations producing 128-bit results */
    unsigned __int128 results[num_constants * 2];
    for (int i = 0; i < num_constants; i++) {
        /* Various operations that require 128-bit arithmetic */
        results[i*2] = constants[i] + ((unsigned __int128)i << 60);
        results[i*2 + 1] = constants[i] * ((unsigned __int128)0x100000001ULL + i);
    }
    
    /* 2. Sort the results - triggers many comparisons */
    sort_128bit_array(results, num_constants * 2);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = results[i] * 0x123456789ABCDEFULL;
        access_with_128bit_offset(offset);
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = ((unsigned __int128)0x123456789ABCDEF0ULL << 64);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 7;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* 5. Switch statement with large 128-bit case constants */
        switch ((unsigned __int128)(i & 0xFF)) {
            case ((unsigned __int128)0x123456789ABCDEF0ULL & 0xFF):
                wbf.a = (wbf.a + i) & (((unsigned __int128)1 << 70) - 1);
                break;
            case ((unsigned __int128)0xFEDCBA9876543210ULL & 0xFF):
                wbf.b = (wbf.b * 3) & (((unsigned __int128)1 << 58) - 1);
                break;
            case ((unsigned __int128)0x8888888888888888ULL & 0xFF):
                wbf.c = wbf.c ^ i;
                break;
            case ((unsigned __int128)0x5555555555555555ULL & 0xFF):
                wbf.d = (wbf.d + 1) & 0xFF;
                break;
            default:
                wbf.a = (wbf.a >> 1) & (((unsigned __int128)1 << 70) - 1);
                break;
        }
        
        /* Access array with calculated offset */
        unsigned __int128 offset = i * 0x12345;
        access_with_128bit_offset(offset);
    }
    
    /* 6. Additional comparisons in conditional statements */
    unsigned __int128 x = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64);
    unsigned __int128 y = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64);
    
    if (x > y) {
        results[0] = x - y;
    }
    
    if (x < (y << 1)) {
        results[1] = x + y;
    }
    
    if (x == ~((unsigned __int128)0)) {
        results[2] = 0;
    }
    
    /* Calculate checksum to prevent optimization */
    for (int i = 0; i < num_constants * 2; i++) {
        checksum ^= results[i];
    }
    
    checksum ^= wbf.a;
    checksum ^= wbf.b;
    checksum ^= wbf.c;
    checksum ^= wbf.d;
    
    /* Use checksum to affect return value */
    return (int)(checksum & 0x7FFFFFFF);
}
