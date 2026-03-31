/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to compute simple checksum to prevent dead code elimination */
static unsigned long compute_checksum(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned long sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum = (sum * 31) + bytes[i];
    }
    return sum;
}

/* Bubble sort for 128-bit integers - forces many comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* These comparisons will use double_int::cmp internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 large_const1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 large_const2 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 32) | 0xDEADBEEFULL;
    unsigned __int128 large_const3 = ((unsigned __int128)1 << 96) - 1;
    unsigned __int128 large_const4 = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    
    /* Array of 128-bit integers for sorting */
    unsigned __int128 values[8];
    
    /* Perform arithmetic operations requiring 128-bit precision */
    values[0] = large_const1 + large_const2;
    values[1] = large_const2 - large_const3;
    values[2] = large_const3 * 3;
    values[3] = large_const4 >> 32;
    values[4] = large_const1 << 16;
    values[5] = (large_const2 + large_const3) / 2;
    values[6] = large_const4 ^ large_const1;
    values[7] = large_const1 | large_const2;
    
    /* Sort the array - this triggers many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        /* Access array at calculated offset */
        huge_array[(size_t)offset] ^= (char)(values[i] & 0xFF);
        offset_sum += (unsigned long)offset;
    }
    
    /* Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = large_const2;
    unsigned __int128 end = large_const2 + 1000;
    
    for (unsigned __int128 i = start; i < end; i += 7) {
        /* Switch statement with large 128-bit case constants */
        switch ((unsigned long)(i & 0xFF)) {
            case 0x10:
                loop_checksum += i * 2;
                break;
            case 0x20:
                loop_checksum += i / 2;
                break;
            case 0x30:
                loop_checksum += i + 0x1000;
                break;
            case 0x40:
                loop_checksum += i - 0x1000;
                break;
            default:
                loop_checksum += i;
                break;
        }
        
        /* Structure with wide bit-fields */
        struct WideBitfield wbf;
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (i >> 200) & ((1ULL << 56) - 1);
        
        /* Manipulate bit-fields */
        wbf.a ^= wbf.b;
        wbf.c |= wbf.d;
        loop_checksum += (unsigned __int128)wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons in conditional statements */
    unsigned __int128 cmp_result = 0;
    
    if (large_const1 < large_const2) cmp_result += 1;
    if (large_const1 > large_const3) cmp_result += 2;
    if (large_const2 <= large_const4) cmp_result += 4;
    if (large_const3 >= large_const1) cmp_result += 8;
    if (large_const4 == large_const4) cmp_result += 16;
    if (large_const1 != large_const2) cmp_result += 32;
    
    /* Force comparisons through ternary operator */
    unsigned __int128 min_val = (large_const1 < large_const2) ? large_const1 : large_const2;
    unsigned __int128 max_val = (large_const3 > large_const4) ? large_const3 : large_const4;
    
    /* More arithmetic with comparisons */
    unsigned __int128 diff = (max_val > min_val) ? (max_val - min_val) : (min_val - max_val);
    
    /* Combine all results into final checksum */
    unsigned long final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum ^= (unsigned long)(values[i] >> 64);
        final_checksum ^= (unsigned long)(values[i] & 0xFFFFFFFFFFFFFFFFULL);
    }
    
    final_checksum ^= offset_sum;
    final_checksum ^= (unsigned long)(loop_checksum >> 64);
    final_checksum ^= (unsigned long)(loop_checksum & 0xFFFFFFFFFFFFFFFFULL);
    final_checksum ^= (unsigned long)cmp_result;
    final_checksum ^= (unsigned long)(min_val >> 64);
    final_checksum ^= (unsigned long)(max_val & 0xFFFFFFFFFFFFFFFFULL);
    final_checksum ^= (unsigned long)(diff >> 32);
    
    /* Also compute checksum of the structure layout */
    struct WideBitfield test_struct;
    test_struct.a = 0x123456789ABCDEFULL;
    test_struct.b = 0xFEDCBA987654321ULL;
    test_struct.c = 0x5555555555555555ULL;
    test_struct.d = 0xAAAAAAAAAAAAAAAALL;
    
    final_checksum ^= compute_checksum(&test_struct, sizeof(test_struct));
    
    printf("Final checksum: 0x%016lx\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
