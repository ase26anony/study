/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
};

/* Function to perform bubble sort on 128-bit integers */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* These comparisons will trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to calculate array offset using 128-bit arithmetic */
static size_t calculate_offset(unsigned __int128 base, unsigned __int128 index, 
                               unsigned __int128 stride) {
    /* 128-bit calculation that may overflow 64 bits */
    unsigned __int128 offset = base + index * stride;
    /* Modulo to keep within array bounds - requires comparison */
    if (offset >= (1ULL << 31)) {
        offset = offset % (1ULL << 31);
    }
    return (size_t)offset;
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL * 0x100000001ULL,
        ((unsigned __int128)1 << 80) | 0x123456789ABCDEF0ULL,
        ((unsigned __int128)1 << 96) - 1,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL,
        (unsigned __int128)0x8000000000000000ULL * 0x8000000000000000ULL,
        (unsigned __int128)0x7FFFFFFFFFFFFFFFULL * 0x7FFFFFFFFFFFFFFFULL,
        (unsigned __int128)0x12345678ULL << 80,
        (unsigned __int128)0x9ABCDEF0ULL << 80,
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    for (int i = 0; i < num_constants; i++) {
        /* Operations that may trigger constant folding with comparisons */
        results[i*2] = constants[i] + ((unsigned __int128)i << 60);
        results[i*2 + 1] = constants[i] * 2 - ((unsigned __int128)i << 59);
    }
    
    /* Sort the results - many 128-bit comparisons happen here */
    sort_128bit_array(results, num_constants * 2);
    
    /* Access large array using 128-bit offset calculations */
    unsigned char checksum = 0;
    for (int i = 0; i < num_constants; i++) {
        unsigned __int128 base = results[i];
        unsigned __int128 stride = results[num_constants * 2 - i - 1];
        
        /* Calculate offset - triggers comparisons in bounds checking */
        size_t offset = calculate_offset(base, i, stride);
        
        /* Access array and update checksum */
        huge_array[offset % (1ULL << 31)] ^= (i & 0xFF);
        checksum ^= huge_array[offset % (1ULL << 31)];
    }
    
    /* Loop with 128-bit counter - comparisons in loop control */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 32);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i >> 64)) {  /* Use high bits for switch */
            case 0x100000000ULL:
                wbf.a = (wbf.a + i) & (((unsigned __int128)1 << 70) - 1);
                break;
            case 0x100000001ULL:
                wbf.b = (wbf.b + i) & (((unsigned __int128)1 << 58) - 1);
                break;
            case 0xFFFFFFFFULL:
                wbf.a = (wbf.a ^ i) & (((unsigned __int128)1 << 70) - 1);
                break;
            default:
                wbf.b = (wbf.b ^ i) & (((unsigned __int128)1 << 58) - 1);
                break;
        }
        
        /* More comparisons in conditional */
        if (i > start + 500) {
            wbf.a = (wbf.a << 1) & (((unsigned __int128)1 << 70) - 1);
        }
        if (i < start + 300) {
            wbf.b = (wbf.b >> 1) & (((unsigned __int128)1 << 58) - 1);
        }
        
        /* Access array with 128-bit index */
        size_t idx = (size_t)(i % (1ULL << 31));
        huge_array[idx] += (unsigned char)(wbf.a & 0xFF);
        checksum += huge_array[idx];
    }
    
    /* Additional comparisons with edge cases */
    unsigned __int128 max_128 = ~(unsigned __int128)0;
    unsigned __int128 mid_128 = max_128 >> 1;
    
    if (results[0] < mid_128) {
        checksum += 1;
    }
    if (results[num_constants * 2 - 1] > mid_128) {
        checksum += 2;
    }
    
    /* Compare all pairs to maximize comparison coverage */
    for (int i = 0; i < num_constants * 2; i++) {
        for (int j = i + 1; j < num_constants * 2; j++) {
            if (results[i] == results[j]) {
                checksum ^= 0x55;
            }
            if (results[i] < results[j]) {
                checksum ^= 0xAA;
            }
            if (results[i] > results[j]) {
                checksum ^= 0x33;
            }
        }
    }
    
    /* Final checksum to prevent optimization */
    printf("Checksum: 0x%02x\n", checksum & 0xFF);
    
    return (int)(checksum & 0xFF);
}
