/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for coverage testing
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Wide integer constants that exceed 64-bit range */
#define WIDE_CONST_1 (((unsigned __int128)0x123456789ABCDEF0ULL) << 32 | 0xFEDCBA9876543210ULL)
#define WIDE_CONST_2 (((unsigned __int128)0x9876543210ABCDEFULL) << 32 | 0x0123456789ABCDEFULL)
#define WIDE_CONST_3 (((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 32)
#define WIDE_CONST_4 (((unsigned __int128)0x8000000000000000ULL) << 32)
#define WIDE_CONST_5 (((unsigned __int128)0x7FFFFFFFFFFFFFFFULL) << 32 | 0xFFFFFFFFFFFFFFFFULL)

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 120;
    unsigned __int128 d: 8;
};

/* Function to perform bubble sort on 128-bit integers */
static void sort_wide_ints(unsigned __int128 *arr, int n) {
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
    /* These operations force 128-bit arithmetic and comparisons */
    unsigned __int128 offset = base + index * stride;
    
    /* Bounds checking triggers double_int::cmp */
    unsigned __int128 array_size = sizeof(huge_array);
    if (offset >= array_size) {
        offset = offset % array_size;
    }
    
    return (size_t)offset;
}

int main(void) {
    /* Initialize array with wide integer constants */
    unsigned __int128 wide_array[10];
    
    wide_array[0] = WIDE_CONST_1;
    wide_array[1] = WIDE_CONST_2;
    wide_array[2] = WIDE_CONST_3;
    wide_array[3] = WIDE_CONST_4;
    wide_array[4] = WIDE_CONST_5;
    
    /* Generate more values through arithmetic operations */
    for (int i = 5; i < 10; i++) {
        wide_array[i] = wide_array[i-5] + ((unsigned __int128)i << 60);
        wide_array[i] = wide_array[i] * 0x100000001ULL;
        wide_array[i] = wide_array[i] >> (i % 32);
    }
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_wide_ints(wide_array, 10);
    
    /* Initialize the large array with some data */
    for (size_t i = 0; i < sizeof(huge_array); i += 4096) {
        huge_array[i] = (char)(i % 256);
    }
    
    /* Access array using 128-bit offset calculations */
    unsigned char checksum = 0;
    for (int i = 0; i < 10; i++) {
        size_t offset = calculate_offset(wide_array[i], i, 0x100000001ULL);
        checksum ^= huge_array[offset % sizeof(huge_array)];
    }
    
    /* Loop with 128-bit counter - triggers comparisons in loop control */
    unsigned __int128 loop_start = WIDE_CONST_1;
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 100;
    
    struct wide_bitfield wbf = {0};
    wbf.a = 0x1FFFFFFFFFFFFFFFULL;  /* 70-bit value */
    wbf.b = 0x3FFFFFFFFFFFFFFFULL;  /* 58-bit value */
    wbf.c = WIDE_CONST_1;
    wbf.d = 0xFF;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Switch with large 128-bit case values */
        switch (i & 0xF) {  /* Use lower bits for switch */
            case 0:
                wbf.a = (wbf.a + i) & 0x3FFFFFFFFFFFFFFFFFULL;
                break;
            case 1:
                wbf.b = (wbf.b + i) & 0x3FFFFFFFFFFFFFFFULL;
                break;
            case 2:
                wbf.c = wbf.c - i;
                break;
            case 3:
                wbf.d = (wbf.d ^ (i & 0xFF)) & 0xFF;
                break;
            default:
                /* Complex expression forcing 128-bit comparison */
                if ((i * 3) < (loop_end * 2)) {
                    wbf.a = (wbf.a << 1) | (wbf.d & 1);
                }
                break;
        }
        
        /* More comparisons in conditional */
        if (i > (loop_start + 500)) {
            wbf.c = wbf.c >> 1;
        }
        
        /* Access array with calculated offset */
        size_t offset = calculate_offset(i, i % 100, 17);
        checksum += huge_array[offset % sizeof(huge_array)];
    }
    
    /* Additional 128-bit comparisons in complex expressions */
    unsigned __int128 x = WIDE_CONST_2;
    unsigned __int128 y = WIDE_CONST_3;
    
    /* Chain of comparisons */
    if (x < y && y > WIDE_CONST_4 && x != 0) {
        x = x * 2;
    }
    
    if ((x + y) > (WIDE_CONST_5 >> 1)) {
        y = y / 3;
    }
    
    /* Final checksum calculation to prevent optimization */
    checksum ^= (unsigned char)(wbf.a & 0xFF);
    checksum ^= (unsigned char)(wbf.b & 0xFF);
    checksum ^= (unsigned char)(wbf.c & 0xFF);
    checksum ^= (unsigned char)(wbf.d & 0xFF);
    checksum ^= (unsigned char)(x & 0xFF);
    checksum ^= (unsigned char)(y & 0xFF);
    
    printf("Checksum: %u\n", (unsigned int)checksum);
    
    return 0;
}
