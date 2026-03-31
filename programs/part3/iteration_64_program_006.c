/* double_int_coverage.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:70;
    unsigned __int128 d:58;
} __attribute__((packed));

/* Function to perform bubble sort on 128-bit integers */
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

/* Function to calculate array offset using 128-bit arithmetic */
static size_t calculate_offset(unsigned __int128 base, unsigned __int128 index, 
                               unsigned __int128 stride) {
    /* Force 128-bit multiplication and addition */
    unsigned __int128 offset = base + index * stride;
    
    /* Modulo to keep within array bounds - triggers comparisons */
    if (offset >= (unsigned __int128)(sizeof(huge_array))) {
        offset = offset % (unsigned __int128)(sizeof(huge_array));
    }
    
    return (size_t)offset;
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL) << 64 | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64 | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x8000000000000000ULL) << 64 | 0x0000000000000000ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL) << 64 | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x0000000000000000ULL) << 64 | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL) << 64 | 0x5555555555555555ULL,
        ((unsigned __int128)0x5555555555555555ULL) << 64 | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)0x1000000000000000ULL) << 64 | 0x0000000000000001ULL,
    };
    
    const int num_values = sizeof(values) / sizeof(values[0]);
    
    /* 1. Sort array of 128-bit integers - triggers many comparisons */
    printf("Sorting 128-bit integers...\n");
    sort_128bit_array(values, num_values);
    
    /* 2. Array indexing with large offsets */
    printf("Performing array offset calculations...\n");
    unsigned __int128 base = ((unsigned __int128)0x100000000ULL) << 64;
    unsigned __int128 stride = ((unsigned __int128)0x1000ULL) << 32;
    
    for (int i = 0; i < num_values; i++) {
        size_t offset = calculate_offset(base, values[i], stride);
        huge_array[offset] = (char)(i + 'A');
    }
    
    /* 3. Loop with 128-bit counter and switch statement */
    printf("Running 128-bit loop with switch...\n");
    unsigned __int128 checksum = 0;
    
    /* Define large case constants for switch */
    unsigned __int128 case1 = ((unsigned __int128)0x123456789ABCDEF0ULL) << 64;
    unsigned __int128 case2 = ((unsigned __int128)0xFEDCBA9876543210ULL) << 64;
    unsigned __int128 case3 = ((unsigned __int128)0xAAAAAAAAAAAAAAAALL) << 64;
    unsigned __int128 case4 = ((unsigned __int128)0x5555555555555555ULL) << 64;
    
    /* Loop with 128-bit induction variable */
    for (unsigned __int128 i = case1; 
         i < case1 + 1000;  /* Comparison in loop condition */
         i += 17) {
        
        /* Switch with large 128-bit cases - triggers comparison for case matching */
        switch (i) {
            case ((unsigned __int128)0x123456789ABCDEF0ULL) << 64:
                checksum += i * 2;
                break;
            case ((unsigned __int128)0xFEDCBA9876543210ULL) << 64:
                checksum += i * 3;
                break;
            case ((unsigned __int128)0xAAAAAAAAAAAAAAAALL) << 64:
                checksum += i * 5;
                break;
            case ((unsigned __int128)0x5555555555555555ULL) << 64:
                checksum += i * 7;
                break;
            default:
                checksum += i;
                break;
        }
        
        /* 4. Structure with wide bit-fields */
        struct wide_bitfield wbf;
        wbf.a = i >> 58;
        wbf.b = i & (((unsigned __int128)1 << 58) - 1);
        wbf.c = (i * 3) >> 70;
        wbf.d = (i * 3) & (((unsigned __int128)1 << 58) - 1);
        
        /* Operations that may trigger offset calculations */
        checksum += (unsigned __int128)wbf.a << wbf.b;
        checksum += (unsigned __int128)wbf.c << wbf.d;
    }
    
    /* 5. Additional arithmetic operations producing 128-bit results */
    printf("Performing 128-bit arithmetic...\n");
    unsigned __int128 x = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64;
    unsigned __int128 y = ((unsigned __int128)0x1000000000000000ULL) << 32;
    
    /* Various operations that require 128-bit precision */
    unsigned __int128 prod = x * y;          /* Multiplication */
    unsigned __int128 sum = x + y;           /* Addition */
    unsigned __int128 diff = x - y;          /* Subtraction */
    unsigned __int128 shifted = x << 3;      /* Left shift */
    
    /* Comparisons between results - each triggers double_int::cmp */
    if (prod > sum) checksum += prod;
    if (sum < diff) checksum += sum;
    if (diff >= shifted) checksum += diff;
    if (shifted <= prod) checksum += shifted;
    
    /* 6. Final checksum output to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n",
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Use array to prevent dead code elimination */
    volatile char dummy = huge_array[0] + huge_array[sizeof(huge_array)-1];
    (void)dummy;
    
    return 0;
}
