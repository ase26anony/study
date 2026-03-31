/* double_int_coverage.c - Targets GCC's double_int::cmp function */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Helper for modular array access */
static size_t mod_array_size(unsigned __int128 idx) {
    return idx % (sizeof(huge_array) / sizeof(huge_array[0]));
}

/* Simple bubble sort for 128-bit values */
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

/* Process switch with large 128-bit cases */
static void process_with_switch(unsigned __int128 value) {
    /* Switch with large case values forces sorting/comparison during compilation */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            checksum += value * 3;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL:
            checksum += value * 5;
            break;
        case ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL:
            checksum += value * 7;
            break;
        case ((unsigned __int128)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL:
            checksum += value * 11;
            break;
        default:
            checksum += value;
            break;
    }
}

int main(void) {
    /* Initialize 128-bit constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL,
        ((unsigned __int128)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL,
        ((unsigned __int128)0x1000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,
    };
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations producing 128-bit results */
    unsigned __int128 results[num_constants * 2];
    for (int i = 0; i < num_constants; i++) {
        /* Operations that require 128-bit precision */
        results[i*2] = constants[i] + ((unsigned __int128)1 << 63);
        results[i*2 + 1] = constants[i] * 3 - ((unsigned __int128)1 << 62);
    }
    
    /* 2. Sort the results - each comparison triggers double_int::cmp */
    sort_128bit_array(results, num_constants * 2);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = results[i] + ((unsigned __int128)i << 40);
        size_t safe_idx = mod_array_size(offset);
        huge_array[safe_idx] = (char)(i & 0xFF);
        checksum += (unsigned __int128)huge_array[safe_idx];
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 loop_start = ((unsigned __int128)0x1000000000000000ULL << 64);
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 100;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Loop comparison i < loop_end triggers double_int::cmp */
        
        /* 5. Switch statement with large 128-bit cases */
        process_with_switch(i);
        
        /* 6. Structure with wide bit-fields */
        struct WideBitfield wbf;
        wbf.low_part = i & (((unsigned __int128)1 << 70) - 1);
        wbf.high_part = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        
        /* Access bit-fields - compiler calculates offsets using double_int */
        checksum += (unsigned __int128)wbf.low_part;
        checksum += (unsigned __int128)wbf.high_part << 70;
        
        /* Additional arithmetic to ensure wide integer usage */
        unsigned __int128 product = i * ((unsigned __int128)0x12345678 << 64);
        if (product > ((unsigned __int128)0xFFFFFFFFULL << 96)) {
            checksum += 1;
        }
    }
    
    /* 7. Additional comparisons in conditional expressions */
    unsigned __int128 a = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 b = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    /* These comparisons all trigger double_int::cmp */
    if (a < b) checksum += 1000;
    if (a > b) checksum += 2000;
    if (a == b) checksum += 3000;
    if (a != b) checksum += 4000;
    if (a <= b) checksum += 5000;
    if (a >= b) checksum += 6000;
    
    /* Output checksum to prevent optimization */
    printf("Checksum (low 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    
    return 0;
}
