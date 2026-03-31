/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 20]; /* 1MB array for safe access */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 low_bits: 70;
    unsigned __int128 high_bits: 58;
    unsigned __int128 full_value;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Initialize with large 128-bit constants */
static unsigned __int128 large_constants[] = {
    (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
    (unsigned __int128)0xFEDCBA9876543210ULL * 0x1000000000000001ULL,
    (unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64,
    (unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 32,
    (unsigned __int128)0x5555555555555555ULL << 96,
    (unsigned __int128)0xDEADBEEFDEADBEEFULL * 0xCAFEBABECAFEBABEULL,
    (unsigned __int128)0x8000000000000000ULL << 32,
    (unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64,
};

#define ARRAY_SIZE (sizeof(large_constants)/sizeof(large_constants[0]))

/* Simple bubble sort to force many 128-bit comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* This comparison will use double_int::cmp internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function using switch with large 128-bit case labels */
static int process_large_switch(unsigned __int128 value) {
    /* Switch with large constants forces sorting/comparison during compilation */
    switch (value) {
        case (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL:
            return 1;
        case (unsigned __int128)0xFEDCBA9876543210ULL * 0x1000000000000001ULL:
            return 2;
        case (unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64:
            return 3;
        case (unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 32:
            return 4;
        case (unsigned __int128)0x5555555555555555ULL << 96:
            return 5;
        default:
            return 0;
    }
}

int main(void) {
    struct wide_bitfield wbf = {0};
    unsigned __int128 working_array[ARRAY_SIZE];
    
    /* 1. Initialize and manipulate 128-bit variables */
    unsigned __int128 a = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    unsigned __int128 b = (unsigned __int128)0xFEDCBA9876543210ULL * 0x1000000000000001ULL;
    unsigned __int128 c = a + b;
    unsigned __int128 d = a * 3;
    unsigned __int128 e = b >> 32;
    unsigned __int128 f = c << 16;
    
    /* Force comparisons during constant folding */
    if (a < b) checksum += 1;
    if (c > d) checksum += 2;
    if (e != f) checksum += 4;
    
    /* 2. Copy to array and sort (many comparisons) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        working_array[i] = large_constants[i];
    }
    sort_128bit_array(working_array, ARRAY_SIZE);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Calculate offset using 128-bit arithmetic, then modulo to stay in bounds */
        unsigned __int128 offset = working_array[i] % (sizeof(huge_array) - 100);
        huge_array[(size_t)offset] = (char)i;
        checksum += huge_array[(size_t)offset];
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = a;
    unsigned __int128 end = a + 1000;
    unsigned __int128 step = (b - a) / 100;
    
    if (step == 0) step = 1;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Each loop iteration compares i < end (128-bit comparison) */
        
        /* Use switch with large values */
        int switch_result = process_large_switch(i % 100);
        checksum += switch_result;
        
        /* Manipulate wide bit-field structure */
        wbf.low_bits = (i >> 32) & ((1ULL << 70) - 1);
        wbf.high_bits = (i >> 102) & ((1ULL << 58) - 1);
        wbf.full_value = i;
        
        checksum += wbf.low_bits + wbf.high_bits;
    }
    
    /* 5. Additional arithmetic producing 128-bit results */
    unsigned __int128 products[4];
    products[0] = a * b;
    products[1] = b * c;
    products[2] = c * d;
    products[3] = d * e;
    
    /* More comparisons */
    for (int i = 0; i < 3; i++) {
        if (products[i] < products[i + 1]) {
            checksum += products[i];
        }
    }
    
    /* 6. Complex expression with multiple comparisons */
    unsigned __int128 x = a;
    unsigned __int128 y = b;
    for (int i = 0; i < 10; i++) {
        x = (x * 1103515245 + 12345) & 0x7FFFFFFFFFFFFFFFULL;
        y = (y * 1664525 + 1013904223) & 0x7FFFFFFFFFFFFFFFULL;
        
        /* Force 128-bit comparison in conditional */
        unsigned __int128 mask = (unsigned __int128)0xFFFFFFFFULL << 32;
        if ((x & mask) < (y & mask)) {
            checksum += i;
        }
    }
    
    /* Final output to prevent optimization */
    printf("Checksum (low 64 bits): %llu\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Use array to prevent unused variable warnings */
    printf("Array access sample: %d\n", huge_array[1000]);
    
    return 0;
}
