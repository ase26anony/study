/* wide-int-test.c - Test program for double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to generate checksum to prevent optimization */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values */
void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison should trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to access array with 128-bit offsets */
void access_with_large_offsets(unsigned __int128 offsets[], int n) {
    for (int i = 0; i < n; i++) {
        /* Calculate offset within bounds using modulo */
        size_t offset = (size_t)(offsets[i] % (sizeof(huge_array) - 1));
        huge_array[offset] = (char)(i & 0xFF);
        checksum += (unsigned __int128)huge_array[offset];
    }
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Constants > 2^64 - 1 */
        ((unsigned __int128)0x123456789ABCDEF0ULL) * 0x100000001ULL,
        ((unsigned __int128)0xFEDCBA9876543210ULL) << 64,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) + 1,
        ((unsigned __int128)0x8000000000000000ULL) << 1,
        ((unsigned __int128)0x5555555555555555ULL) * 0x3333333333333333ULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL) | (((unsigned __int128)0xBBBBBBBBBBBBBBBBULL) << 64),
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Various operations that generate 128-bit results */
        results[i*2] = constants[i] + ((unsigned __int128)i << 60);
        results[i*2 + 1] = constants[i] * ((unsigned __int128)0x100000001ULL);
        
        /* Force comparisons during arithmetic */
        if (results[i*2] < constants[i]) {
            results[i*2] = ~results[i*2];
        }
    }
    
    /* Sort the results - each comparison should trigger double_int::cmp */
    sort_128bit_array(results, num_constants * 2);
    
    /* Access array with large offsets */
    access_with_large_offsets(results, num_constants * 2);
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = ((unsigned __int128)0x1000000000000000ULL) << 32;
    unsigned __int128 end = start + 100;
    unsigned __int128 step = 7;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i & 0xF)) {  /* Reduced to 4-bit for simplicity */
            case 0:
                wbf.a = i & 0x3FFFFFFFFFFFFFFFULL;
                break;
            case 1:
                wbf.b = i & 0x3FFFFFFFFFFFFFFFULL;
                break;
            case 2:
                wbf.c = i & 0xFFFFFFFFFFFFFFFFULL;
                break;
            case 3:
                wbf.d = i & 0xFFFFFFFFFFFFFFFFULL;
                break;
            default:
                wbf.a = (wbf.a + wbf.b) & 0x3FFFFFFFFFFFFFFFULL;
                break;
        }
        
        /* More comparisons in loop control */
        if (i > start + 50) {
            wbf.c = (wbf.c << 1) | (wbf.d >> 63);
        }
        
        checksum += i + wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons for boundary checking */
    unsigned __int128 max_val = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64;
    unsigned __int128 min_val = 0;
    
    for (int i = 0; i < num_constants * 2; i++) {
        if (results[i] > max_val) {
            results[i] = max_val;
        }
        if (results[i] < min_val) {
            results[i] = min_val;
        }
        checksum += results[i];
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Checksum (lower 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (upper 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    
    return 0;
}
