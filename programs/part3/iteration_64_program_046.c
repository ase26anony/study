/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize -m32 wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 24];  /* 16MB array - large enough for offset calculations */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 extra : 60;
    unsigned __int128 padding : 68;
} __attribute__((packed));

/* Function to generate checksum to prevent dead code elimination */
static unsigned long long checksum = 0;

/* Simple bubble sort for 128-bit integers to force many comparisons */
void sort_128bit_array(unsigned __int128 *arr, int size) {
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

/* Function that uses switch with large 128-bit case labels */
unsigned int switch_128bit(unsigned __int128 value) {
    unsigned int result = 0;
    
    /* Switch with large 128-bit constants - compiler must sort these */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            result = 1;
            break;
        case ((unsigned __int128)0xAAAAAAAAAAAAAAAALLU << 64) | 0x5555555555555555LLU:
            result = 2;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFLLU << 64) | 0x0000000000000000LLU:
            result = 3;
            break;
        case ((unsigned __int128)0x8000000000000000LLU << 64) | 0x0000000000000001LLU:
            result = 4;
            break;
        case ((unsigned __int128)0x1000000000000000LLU << 64) | 0x2000000000000000LLU:
            result = 5;
            break;
        default:
            result = 0;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize large 128-bit constants */
    unsigned __int128 constants[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAALLU << 64) | 0x5555555555555555LLU,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFLLU << 64) | 0x0000000000000000LLU,
        ((unsigned __int128)0x8000000000000000LLU << 64) | 0x0000000000000001LLU,
        ((unsigned __int128)0x1000000000000000LLU << 64) | 0x2000000000000000LLU,
        ((unsigned __int128)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL,
        ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL,
        ((unsigned __int128)0x3333333333333333ULL << 64) | 0x4444444444444444ULL,
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Operations that require full 128-bit comparison */
        results[i*2] = constants[i] + ((unsigned __int128)1 << 63);
        results[i*2 + 1] = constants[i] * 3;
        
        /* Force comparison in conditional */
        if (constants[i] > ((unsigned __int128)0x8000000000000000ULL << 64)) {
            results[i*2] >>= 1;
        }
    }
    
    /* 2. Sort the results - this will generate many 128-bit comparisons */
    sort_128bit_array(results, num_constants * 2);
    
    /* 3. Array indexing with large offsets derived from 128-bit values */
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) - 1);
        huge_array[(size_t)offset] = (char)(i & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = ((unsigned __int128)0x1000000000000000ULL << 64);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Each iteration compares i < end using 128-bit comparison */
        
        /* 5. Use switch with 128-bit cases inside the loop */
        unsigned int sw_result = switch_128bit(i);
        checksum += sw_result;
        
        /* 6. Manipulate struct with wide bit-fields */
        wbf.low_part = i & (((unsigned __int128)1 << 70) - 1);
        wbf.high_part = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        wbf.extra = (i * 2) & (((unsigned __int128)1 << 60) - 1);
        
        checksum += (unsigned long long)(wbf.low_part ^ wbf.high_part ^ wbf.extra);
        
        /* Additional comparisons in loop conditions */
        if (i > start + 500) {
            wbf.padding = i - 500;
        }
    }
    
    /* 7. More comparisons in complex expressions */
    unsigned __int128 max_val = 0;
    unsigned __int128 min_val = ~(unsigned __int128)0;
    
    for (int i = 0; i < num_constants * 2; i++) {
        /* Multiple comparisons that exercise the cmp function */
        if (results[i] > max_val) {
            max_val = results[i];
        }
        if (results[i] < min_val) {
            min_val = results[i];
        }
        
        /* Compare against various thresholds */
        unsigned __int128 threshold1 = ((unsigned __int128)0x8000000000000000ULL << 64);
        unsigned __int128 threshold2 = ((unsigned __int128)0x4000000000000000ULL << 64);
        
        if (results[i] > threshold1 && results[i] < threshold2 + threshold1) {
            checksum += 1;
        }
    }
    
    /* 8. Final checksum output to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    printf("Max value: 0x%016llx%016llx\n", 
           (unsigned long long)(max_val >> 64), 
           (unsigned long long)(max_val & 0xFFFFFFFFFFFFFFFFULL));
    printf("Min value: 0x%016llx%016llx\n", 
           (unsigned long long)(min_val >> 64), 
           (unsigned long long)(min_val & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Use sorted results to demonstrate they were actually sorted */
    printf("First sorted value: 0x%016llx%016llx\n",
           (unsigned long long)(results[0] >> 64),
           (unsigned long long)(results[0] & 0xFFFFFFFFFFFFFFFFULL));
    printf("Last sorted value: 0x%016llx%016llx\n",
           (unsigned long long)(results[num_constants*2 - 1] >> 64),
           (unsigned long long)(results[num_constants*2 - 1] & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
