/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 80;
    unsigned __int128 d: 10;
} wbf;

/* Function to generate checksum to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Helper function to add to checksum */
static void add_to_checksum(unsigned __int128 val) {
    checksum ^= val;
    checksum = (checksum << 1) | (checksum >> 127);
}

/* 1. Wide Integer Constant Folding and Sorting */
static void wide_int_sorting(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[10] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        ((unsigned __int128)0x123456789ABCDEF0ULL) * 0x100000001ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) * 0xFFFFFFFFULL,
        ((unsigned __int128)1 << 127) - 1,
        ((unsigned __int128)1 << 127)
    };
    
    /* Perform arithmetic operations that require 128-bit precision */
    for (int i = 0; i < 9; i++) {
        values[i] = values[i] + (values[i] >> 3);
        values[i] = values[i] * 0x1234567ULL;
        values[i] = values[i] << (i % 64);
    }
    
    /* Bubble sort to force many comparisons */
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9 - i; j++) {
            /* This comparison will use double_int::cmp internally */
            if (values[j] > values[j + 1]) {
                unsigned __int128 temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }
    
    /* Use sorted values for array indexing */
    for (int i = 0; i < 10; i++) {
        /* Calculate offset within bounds using modulo */
        size_t offset = (size_t)(values[i] % (sizeof(huge_array) / sizeof(huge_array[0])));
        huge_array[offset] = (char)(values[i] & 0xFF);
        add_to_checksum(values[i]);
    }
}

/* 2. Loop Boundary Comparisons with Wide Integers */
static void wide_loop_comparisons(void) {
    /* Start with a large 128-bit value */
    unsigned __int128 start = ((unsigned __int128)0x123456789ABCDEF0ULL << 64);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 7;
    
    /* Loop with 128-bit counter - each iteration compares i < end */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* 3. Switch Statements with Large Cases */
        switch ((uint64_t)(i >> 64)) {  /* Use high 64 bits for switch */
            case 0x123456789ABCDEF0ULL:
                huge_array[(size_t)(i % sizeof(huge_array))] += 1;
                break;
            case 0xFFFFFFFFFFFFFFFFULL:
                huge_array[(size_t)(i % sizeof(huge_array))] += 2;
                break;
            case 0x8000000000000000ULL:
                huge_array[(size_t)(i % sizeof(huge_array))] += 3;
                break;
            case 0x7FFFFFFFFFFFFFFFULL:
                huge_array[(size_t)(i % sizeof(huge_array))] += 4;
                break;
            default:
                huge_array[(size_t)(i % sizeof(huge_array))] += 5;
                break;
        }
        
        /* 4. Structure with Wide Bit-field Offsets */
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 80) - 1);
        wbf.d = (i >> 208) & ((1ULL << 10) - 1);
        
        add_to_checksum(wbf.a);
        add_to_checksum(wbf.b);
        add_to_checksum(wbf.c);
        add_to_checksum(wbf.d);
    }
}

/* 5. Array Indexing with Large Offsets */
static void large_offset_calculations(void) {
    unsigned __int128 base = ((unsigned __int128)0x87654321FEDCBA98ULL << 64);
    unsigned __int128 stride = 0x100000001ULL;
    
    /* Calculate array offsets using 128-bit arithmetic */
    for (int i = 0; i < 100; i++) {
        unsigned __int128 offset = base + (unsigned __int128)i * stride;
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
        
        /* Multiple comparisons in bounds checking */
        if (safe_offset < sizeof(huge_array)) {
            huge_array[safe_offset] = (char)((offset >> 32) & 0xFF);
        }
        
        /* Compare offsets for ordering */
        static unsigned __int128 prev_offset = 0;
        if (i > 0) {
            /* This comparison triggers double_int::cmp */
            if (offset > prev_offset) {
                add_to_checksum(offset - prev_offset);
            }
        }
        prev_offset = offset;
    }
}

/* 6. Complex 128-bit Comparisons in Conditional Expressions */
static void complex_comparisons(void) {
    /* Generate various 128-bit values for comparison */
    unsigned __int128 masks[] = {
        ~(unsigned __int128)0,  /* All ones */
        (unsigned __int128)1 << 127,
        (unsigned __int128)1 << 64,
        (unsigned __int128)0xFFFFFFFF00000000ULL << 64,
    };
    
    /* Perform multiple comparisons in complex expressions */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Each of these comparisons uses double_int::cmp */
            if (masks[i] < masks[j]) {
                add_to_checksum(masks[i]);
            } else if (masks[i] > masks[j]) {
                add_to_checksum(masks[j]);
            } else {
                add_to_checksum(masks[i] ^ masks[j]);
            }
            
            /* Compound comparison */
            if (masks[i] >= masks[j] && masks[i] <= (masks[j] << 1)) {
                huge_array[(i * 16 + j) % sizeof(huge_array)]++;
            }
        }
    }
}

int main(void) {
    /* Initialize huge_array */
    for (size_t i = 0; i < sizeof(huge_array); i += 4096) {
        huge_array[i] = (char)(i & 0xFF);
    }
    
    /* Initialize bitfield structure */
    wbf.a = 0x123456789ABCDEFULL;
    wbf.b = 0xFEDCBA987654321ULL;
    wbf.c = 0x5555555555555555ULL;
    wbf.d = 0x2AAULL;
    
    /* Execute all comparison-intensive operations */
    wide_int_sorting();
    wide_loop_comparisons();
    large_offset_calculations();
    complex_comparisons();
    
    /* Final checksum calculation to prevent optimization */
    unsigned __int128 final_checksum = checksum;
    for (size_t i = 0; i < sizeof(huge_array); i += 1048576) {  /* 1MB intervals */
        final_checksum ^= (unsigned __int128)huge_array[i];
        final_checksum = (final_checksum << 3) | (final_checksum >> 125);
    }
    
    /* Output to prevent dead code elimination */
    printf("Checksum high: 0x%016llx\n", (unsigned long long)(final_checksum >> 64));
    printf("Checksum low:  0x%016llx\n", (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
