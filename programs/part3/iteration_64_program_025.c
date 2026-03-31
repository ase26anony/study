/* double_int_coverage.c
 * Designed to exercise GCC's double_int comparison logic
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-optimized double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning > 64 bits */
struct WideBitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 120;
    unsigned __int128 d: 8;
};

/* Function to prevent dead code elimination */
static volatile unsigned __int128 checksum = 0;

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

/* Function to calculate array offset using 128-bit arithmetic */
size_t calculate_offset(unsigned __int128 base, unsigned __int128 index, 
                       unsigned __int128 stride) {
    /* 128-bit calculation that may overflow 64 bits */
    unsigned __int128 offset = base + index * stride;
    
    /* Modulo to keep within array bounds */
    return (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[10];
    
    /* Large constants that require 128-bit representation */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = (unsigned __int128)0xFEDCBA9876543210ULL << 64;
    values[2] = values[0] + values[1];
    values[3] = values[1] - values[0];
    values[4] = values[0] * 3ULL;
    values[5] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 32) | 0xAAAAAAAAULL;
    values[6] = values[5] + 0x1000000000000000ULL;
    values[7] = values[6] << 2;
    values[8] = ~(unsigned __int128)0;  /* Max 128-bit value */
    values[9] = values[8] / 2;
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(values, 10);
    
    /* Array indexing with large offsets */
    unsigned __int128 base = 0x1000000000000000ULL;
    unsigned __int128 stride = 0x100000001ULL;
    
    for (int i = 0; i < 10; i++) {
        size_t offset = calculate_offset(base, values[i], stride);
        huge_array[offset] = (char)(i + 'A');
        checksum += (unsigned __int128)huge_array[offset];
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[9];
    unsigned __int128 step = (end - start) / 100;
    
    if (step == 0) step = 1;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end && i < start + step * 10; i += step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i >> 64)) {  /* Use high bits for switch */
            case 0x123456789ABCDEF0ULL >> 32:
                wbf.a = (i & 0x3FFFFFFFFFFFFFFFULL);
                break;
            case 0xFEDCBA9876543210ULL >> 32:
                wbf.b = (i & 0x3FFFFFFFFFFFFFFFULL);
                break;
            case 0xFFFFFFFFFFFFFFFFULL >> 32:
                wbf.c = i;
                break;
            default:
                wbf.d = (i & 0xFF);
                break;
        }
        
        /* Additional comparisons in loop condition */
        if (i > values[5] && i < values[8]) {
            wbf.a ^= (unsigned __int128)0x5555555555555555ULL;
        }
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* More complex 128-bit comparisons */
    unsigned __int128 threshold = ((unsigned __int128)0x8000000000000000ULL << 64);
    
    for (int i = 0; i < 10; i++) {
        if (values[i] > threshold) {
            values[i] >>= 1;
        } else if (values[i] < (threshold >> 2)) {
            values[i] <<= 1;
        }
        
        /* Nested comparisons */
        if ((values[i] > values[(i + 1) % 10]) && 
            (values[i] < values[(i + 2) % 10])) {
            checksum += values[i];
        }
    }
    
    /* Final checksum output to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Verify array was accessed */
    printf("First array char: %c\n", huge_array[0]);
    
    return 0;
}
