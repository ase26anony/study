/* double_int_coverage.c
 * Targets GCC's double_int::cmp function for coverage analysis
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize -fdump-tree-vrp double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 full : 128;
};

/* Function to perform 128-bit comparisons through sorting */
static void sort_128bit_array(unsigned __int128 arr[], int size) {
    /* Bubble sort to force many comparisons */
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* These comparisons will use double_int::cmp internally */
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
    
    /* Modulo to keep within array bounds - requires comparison */
    if (offset >= (1ULL << 31)) {
        offset = offset % (1ULL << 31);
    }
    
    return (size_t)offset;
}

int main(void) {
    /* Initialize large 128-bit constants */
    const unsigned __int128 mask64 = (1ULL << 64) - 1;
    const unsigned __int128 large_const1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const unsigned __int128 large_const2 = ((unsigned __int128)0x9876543210FEDCBAULL << 64) | 0x0123456789ABCDEFULL;
    const unsigned __int128 large_const3 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    
    /* Array of 128-bit values for sorting */
    unsigned __int128 values[8];
    
    /* Generate values using 128-bit arithmetic operations */
    values[0] = large_const1;
    values[1] = large_const2;
    values[2] = large_const3;
    values[3] = large_const1 + large_const2;  /* 128-bit addition */
    values[4] = large_const1 * 3;             /* 128-bit multiplication */
    values[5] = large_const2 >> 32;           /* 128-bit shift */
    values[6] = (large_const3 << 16) | 0xAAAA; /* 128-bit shift and OR */
    values[7] = ~large_const1;                /* 128-bit bitwise NOT */
    
    /* Sort the array - this will trigger many double_int comparisons */
    sort_128bit_array(values, 8);
    
    /* Access huge_array using 128-bit calculated offsets */
    unsigned __int128 checksum = 0;
    for (int i = 0; i < 8; i++) {
        size_t offset = calculate_offset(values[i], i, 17);
        huge_array[offset] = (char)(values[i] & 0xFF);
        checksum ^= values[i];
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    unsigned __int128 loop_start = large_const1 & mask64;
    unsigned __int128 loop_end = loop_start + 100;
    
    struct wide_bitfield wbf = {0};
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += 5) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i & 0xFFFFFFFF)) {  /* Use lower bits for manageable switch */
            case 0x12345678:
                wbf.low_part = i;
                break;
            case 0x87654321:
                wbf.high_part = i >> 70;
                break;
            case 0xABCDEF01:
                wbf.full = i | ((unsigned __int128)i << 64);
                break;
            default:
                wbf.low_part ^= (i & 0x3FFFFFFFFFFFFFFFULL);
                break;
        }
        
        /* More 128-bit comparisons in conditional */
        if (i > (loop_start + 50)) {
            wbf.high_part |= 0x5555;
        }
        
        checksum += wbf.low_part + wbf.high_part;
    }
    
    /* Additional 128-bit comparisons in complex expressions */
    unsigned __int128 cmp1 = values[0] * values[1];
    unsigned __int128 cmp2 = values[2] * values[3];
    
    if (cmp1 < cmp2) {
        checksum += cmp1;
    } else if (cmp1 > cmp2) {
        checksum += cmp2;
    } else {
        checksum += cmp1 + cmp2;
    }
    
    /* Nested comparisons */
    if ((values[4] < values[5]) && (values[6] > values[7])) {
        checksum *= 2;
    }
    
    /* Boundary case comparisons */
    unsigned __int128 max128 = ~(unsigned __int128)0;
    unsigned __int128 min128 = 0;
    
    if (values[0] > min128 && values[0] < max128) {
        checksum |= 1;
    }
    
    /* Final output to prevent optimization */
    printf("Checksum (lower 64 bits): %llu\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Array element at offset 1000: %d\n", (int)huge_array[1000]);
    
    return 0;
}
