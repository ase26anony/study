/* wide-int-test.c - Test program for double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to generate checksum to prevent optimization */
static unsigned long long checksum = 0;

/* Simple bubble sort for unsigned __int128 array */
static void sort_128bit_array(unsigned __int128 *arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* These comparisons will trigger double_int::cmp */
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
    size_t safe_offset = (size_t)(offset % (sizeof(huge_array) - 1024));
    
    /* Access array at calculated offset */
    for (size_t i = 0; i < 1024; i++) {
        huge_array[safe_offset + i] = (char)(i & 0xFF);
        checksum += huge_array[safe_offset + i];
    }
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Values > 2^64 - 1 */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL,
        0xFFFFFFFFFFFFFFFFULL,  /* Exactly 2^64 - 1 */
        0x10000000000000000ULL, /* Exactly 2^64 */
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Operations that produce 128-bit results */
        results[i * 2] = constants[i] + ((unsigned __int128)0x100000001ULL);
        results[i * 2 + 1] = constants[i] * 3ULL;
        
        /* Left shift operations */
        if (i > 0) {
            results[i * 2] <<= (i % 8);
            results[i * 2 + 1] >>= (i % 8);
        }
    }
    
    const int num_results = num_constants * 2;
    
    /* Sort the results - each comparison uses double_int::cmp */
    sort_128bit_array(results, num_results);
    
    /* Access array using 128-bit offsets derived from sorted values */
    for (int i = 0; i < num_results; i++) {
        access_with_128bit_offset(results[i]);
    }
    
    /* Loop with 128-bit counter - comparisons in loop control */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 32);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i & 0xFF)) {  /* Reduced to 8-bit for simplicity */
            case 0x00:
                wbf.a = i & 0x3FFFFFFFFFFFFFFFULL;
                break;
            case 0x10:
                wbf.b = (i >> 10) & 0x3FFFFFFFFFFFFFULL;
                break;
            case 0x20:
                wbf.c = (i >> 20) & 0xFFFFFFFFFFFFFFFFULL;
                break;
            case 0x30:
                wbf.d = (i >> 30) & 0xFFFFFFFFFFFFFFULL;
                break;
            default:
                wbf.a ^= (uint64_t)i;
                wbf.b ^= (uint64_t)(i >> 32);
                break;
        }
        
        /* More comparisons in conditional statements */
        if (i > (start + 500)) {
            wbf.c = wbf.a + wbf.b;
        } else if (i < (start + 300)) {
            wbf.d = wbf.c - wbf.a;
        }
        
        /* Update checksum with bit-field values */
        checksum += (uint64_t)wbf.a + (uint64_t)wbf.b;
    }
    
    /* Additional comparisons in array bounds checking */
    unsigned __int128 large_index = ((unsigned __int128)0x80000000ULL << 32);
    if (large_index > 0 && large_index < ((unsigned __int128)sizeof(huge_array) * 2)) {
        /* This comparison also uses double_int::cmp */
        checksum += (uint64_t)(large_index >> 32);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Checksum: %llu\n", checksum);
    
    /* Verify array is sorted */
    for (int i = 0; i < num_results - 1; i++) {
        if (results[i] > results[i + 1]) {
            printf("ERROR: Array not sorted at index %d\n", i);
            return 1;
        }
    }
    
    printf("Array sorted correctly\n");
    return 0;
}
