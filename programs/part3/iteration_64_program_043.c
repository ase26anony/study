/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24]; /* 16MB array - large enough for offset calculations */

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 low_bits: 70;
    unsigned __int128 mid_bits: 50;
    unsigned __int128 high_bits: 8;
};

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

/* Function to compute checksum to prevent dead code elimination */
static unsigned __int128 compute_checksum(unsigned __int128 arr[], int n, 
                                          struct WideBitfield *wb) {
    unsigned __int128 checksum = 0;
    
    for (int i = 0; i < n; i++) {
        checksum ^= arr[i];
        checksum = (checksum << 1) | (checksum >> 127); /* Rotate left */
    }
    
    /* Include bitfield values in checksum */
    checksum ^= wb->low_bits;
    checksum ^= (wb->mid_bits << 70);
    checksum ^= (wb->high_bits << 120);
    
    return checksum;
}

int main(void) {
    /* Initialize huge array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 7);
    }
    
    /* Create 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        /* Values that require full 128-bit representation */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((unsigned __int128)0x1ULL << 64) | 0x0ULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that generate new 128-bit values */
    unsigned __int128 computed[16];
    int comp_idx = 0;
    
    /* Generate values through various operations */
    for (int i = 0; i < num_constants && comp_idx < 16; i++) {
        /* Addition that may overflow 64 bits */
        computed[comp_idx++] = constants[i] + 0x10000000000000001ULL;
        
        /* Multiplication that requires 128-bit precision */
        if (comp_idx < 16) {
            computed[comp_idx++] = constants[i] * 3ULL;
        }
        
        /* Left shift operations */
        if (comp_idx < 16 && i < 4) {
            computed[comp_idx++] = constants[i] << (i + 1);
        }
        
        /* Bitwise operations */
        if (comp_idx < 16) {
            computed[comp_idx++] = constants[i] ^ constants[(i + 1) % num_constants];
        }
    }
    
    const int num_computed = comp_idx;
    
    /* Sort the computed values - triggers many double_int::cmp calls */
    sort_128bit_array(computed, num_computed);
    
    /* Array indexing with large offsets derived from 128-bit values */
    unsigned char sum = 0;
    for (int i = 0; i < num_computed; i++) {
        /* Calculate offset using 128-bit value, modulo array size */
        size_t offset = (size_t)(computed[i] % sizeof(huge_array));
        sum += huge_array[offset];
        
        /* Additional offset calculation with stride */
        size_t offset2 = (size_t)((computed[i] * 7) % sizeof(huge_array));
        sum += huge_array[offset2];
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    unsigned __int128 loop_start = ((unsigned __int128)0x1000ULL << 64) | 0x0ULL;
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 7;
    
    struct WideBitfield wb = {0};
    wb.low_bits = 0x123456789ABCDEFULL;
    wb.mid_bits = 0x3FFFFFFFFFFFFULL;
    wb.high_bits = 0xFF;
    
    unsigned __int128 loop_checksum = 0;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Switch statement with large 128-bit case constants */
        switch ((uint64_t)(i & 0xFF)) { /* Use lower bits for switch */
            case 0x10:
                wb.low_bits ^= i;
                break;
            case 0x20:
                wb.mid_bits += (i >> 64);
                break;
            case 0x30:
                wb.high_bits |= (i & 0xFF);
                break;
            case 0x40:
                wb.low_bits = (wb.low_bits << 1) | (wb.low_bits >> 69);
                break;
            default:
                wb.mid_bits ^= wb.low_bits;
                break;
        }
        
        /* Access array using 128-bit derived offset */
        size_t offset = (size_t)((i * 13) % sizeof(huge_array));
        loop_checksum += huge_array[offset];
        
        /* Additional 128-bit comparison */
        if (i > (loop_start + 500)) {
            wb.high_bits++;
        }
    }
    
    /* Final checksum computation to prevent optimization */
    unsigned __int128 final_checksum = compute_checksum(computed, num_computed, &wb);
    final_checksum ^= loop_checksum;
    final_checksum ^= (unsigned __int128)sum;
    
    /* Output result to prevent dead code elimination */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(final_checksum >> 64),
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
