/* wide-int-compare.c - Targets GCC's double_int::cmp function */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values to force many comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* Each comparison should trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function using switch with large 128-bit case labels */
static int process_with_switch(unsigned __int128 value) {
    /* Force compiler to sort and compare these large case values */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            return 1;
        case ((unsigned __int128)0xABCDEF0123456789ULL << 64) | 0x9876543210FEDCBAULL:
            return 2;
        case ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL:
            return 3;
        case ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0xFEDCBA9876543210ULL:
            return 4;
        case ((unsigned __int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL:
            return 5;
        default:
            return 0;
    }
}

int main(void) {
    /* Initialize checksum */
    checksum = 0;
    
    /* 1. Wide Integer Constant Folding */
    /* Create 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xABCDEF0123456789ULL << 64) | 0x9876543210FEDCBAULL,
        ((unsigned __int128)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL,
        ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x1ULL << 64) | 0x0ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    };
    
    int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[8];
    for (int i = 0; i < num_constants; i++) {
        /* Various operations that force constant folding */
        results[i] = constants[i] + ((unsigned __int128)0x100000001ULL);
        results[i] = results[i] * 3ULL;
        results[i] = results[i] << 2;
        results[i] = results[i] - 0x1000ULL;
        
        /* Add to checksum */
        checksum += results[i];
    }
    
    /* 2. Sort array to force many 128-bit comparisons */
    sort_128bit_array(results, num_constants);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < num_constants; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array size */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) / sizeof(huge_array[0]));
        
        /* Access array - compiler must compare offset against bounds */
        if (offset < sizeof(huge_array)) {
            huge_array[(size_t)offset] = (char)(i & 0xFF);
            checksum += (unsigned __int128)huge_array[(size_t)offset];
        }
    }
    
    /* 4. Loop with 128-bit counter */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 64) | 0x0ULL;
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Each loop comparison (i < end) requires 128-bit comparison */
        
        /* Use switch with large cases */
        int switch_result = process_with_switch(i);
        checksum += switch_result;
        
        /* 5. Structure with wide bit-field offsets */
        struct WideBitfield wbf;
        wbf.a = i & (((unsigned __int128)1 << 70) - 1);
        wbf.b = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        wbf.c = (i >> 128) | ((i & 0xFFFF) << 104);
        wbf.d = (i >> 240) & 0xFF;
        
        /* Access all bit-fields */
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
        
        /* Array access with 128-bit derived index */
        size_t idx = (size_t)((i * 7) % (sizeof(huge_array) / sizeof(huge_array[0])));
        if (idx < sizeof(huge_array)) {
            huge_array[idx] = (char)(wbf.d & 0xFF);
            checksum += (unsigned __int128)huge_array[idx];
        }
    }
    
    /* Additional comparisons in conditional statements */
    unsigned __int128 x = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 y = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0ULL;
    
    /* These comparisons should trigger the uncovered cmp logic */
    if (x < y) checksum += 1;
    if (x > y) checksum += 2;
    if (x == y) checksum += 3;
    if (x != y) checksum += 4;
    if (x <= y) checksum += 5;
    if (x >= y) checksum += 6;
    
    /* Force use of all results to prevent optimization */
    printf("Checksum (lower 64 bits): 0x%016llx\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (upper 64 bits): 0x%016llx\n", 
           (unsigned long long)(checksum >> 64));
    
    /* Use sorted results */
    printf("Sorted results (first 3):\n");
    for (int i = 0; i < 3 && i < num_constants; i++) {
        printf("  [%d] = 0x%016llx%016llx\n", i,
               (unsigned long long)(results[i] >> 64),
               (unsigned long long)(results[i] & 0xFFFFFFFFFFFFFFFFULL));
    }
    
    return 0;
}
