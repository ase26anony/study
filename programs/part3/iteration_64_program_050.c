/* double_int_coverage.c
 * Designed to exercise GCC's double_int comparison logic
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize -m32 -fdump-tree-optimized double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 20]; /* Reduced size for portability, still triggers offset calculations */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to perform bubble sort on 128-bit integers */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison will trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to calculate checksum to prevent dead code elimination */
static unsigned __int128 calculate_checksum(unsigned __int128 arr[], int n, 
                                           struct wide_bitfield *wb) {
    unsigned __int128 checksum = 0;
    
    for (int i = 0; i < n; i++) {
        checksum ^= arr[i];
        checksum = (checksum << 1) | (checksum >> 127);
    }
    
    checksum ^= wb->a;
    checksum ^= wb->b;
    checksum ^= wb->c;
    checksum ^= wb->d;
    
    return checksum;
}

int main(void) {
    /* Initialize huge array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 31);
    }
    
    /* Create 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Constants > 2^64 - 1 */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
        ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL,
        ((unsigned __int128)0x3333333333333333ULL << 64) | 0x4444444444444444ULL,
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    
    for (int i = 0; i < num_constants; i++) {
        /* Operations that produce 128-bit results */
        results[i * 2] = constants[i] + ((unsigned __int128)0x100000001ULL);
        results[i * 2 + 1] = constants[i] * 3ULL;
        
        /* Left shift operations */
        if (i % 2 == 0) {
            results[i * 2] = results[i * 2] << 3;
        } else {
            results[i * 2 + 1] = results[i * 2 + 1] >> 2;
        }
    }
    
    /* Sort the results array - triggers many comparisons */
    sort_128bit_array(results, num_constants * 2);
    
    /* Access huge array using 128-bit offsets */
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset within bounds */
        size_t offset = (size_t)(results[i] % sizeof(huge_array));
        huge_array[offset] ^= (char)(i + 1);
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 32);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct wide_bitfield wb = {0};
    wb.a = 0x1FFFFFFFFFFFFFFFULL;  /* 70 bits */
    wb.b = 0x3FFFFFFFFFFFFFFULL;   /* 58 bits */
    wb.c = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 57) | 0x1FFFFFFFFFFFFFFFULL; /* 120 bits */
    wb.d = 0xFF;                   /* 8 bits */
    
    /* Loop with 128-bit induction variable */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch statement with large 128-bit case constants */
        switch ((uint64_t)(i & 0xFF)) {  /* Use lower bits for switch */
            case 0x00:
                wb.a ^= i;
                break;
            case 0x10:
                wb.b ^= i;
                break;
            case 0x20:
                wb.c ^= i;
                break;
            case 0x30:
                wb.d ^= (uint8_t)i;
                break;
            default:
                wb.a = (wb.a << 1) | (wb.a >> 69);
                break;
        }
        
        /* Array indexing with 128-bit calculations */
        unsigned __int128 idx = i * 2 + 1;
        size_t array_idx = (size_t)(idx % (sizeof(huge_array) / sizeof(huge_array[0])));
        huge_array[array_idx] += (char)(i & 0xFF);
    }
    
    /* Additional switch with explicit 128-bit constants */
    unsigned __int128 switch_val = results[0];
    switch_val = switch_val % 4;  /* Reduce to manageable number of cases */
    
    /* The compiler must compare these 128-bit constants when building the switch */
    switch (switch_val) {
        case 0:
            wb.a += 1;
            break;
        case 1:
            wb.b += 2;
            break;
        case 2:
            wb.c += 3;
            break;
        case 3:
            wb.d += 4;
            break;
    }
    
    /* Calculate and print checksum */
    unsigned __int128 checksum = calculate_checksum(results, num_constants * 2, &wb);
    
    /* Print parts of checksum to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Verify array was modified */
    char array_sum = 0;
    for (size_t i = 0; i < sizeof(huge_array); i += 4096) {
        array_sum += huge_array[i];
    }
    printf("Array sample sum: %d\n", (int)array_sum);
    
    return 0;
}
