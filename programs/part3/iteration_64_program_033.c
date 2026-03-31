/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize -m32 wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
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
static unsigned __int128 compute_checksum(unsigned __int128 value) {
    return (value ^ (value >> 64)) + (value << 7);
}

int main(void) {
    /* Initialize large 128-bit constants */
    unsigned __int128 base = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 multiplier = ((unsigned __int128)0x100000001ULL << 32) | 0x100000001ULL;
    unsigned __int128 mask = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* Create array of 128-bit values through various operations */
    unsigned __int128 values[8];
    
    /* 1. Wide integer constant folding operations */
    values[0] = base;
    values[1] = base * multiplier;                     /* Multiplication > 64 bits */
    values[2] = (base << 70) | (base >> 58);           /* Shifts mixing high/low */
    values[3] = values[1] + values[2];                 /* Addition with carry */
    values[4] = values[1] - values[2];                 /* Subtraction with borrow */
    values[5] = values[3] & mask;                      /* Bitwise AND */
    values[6] = values[4] | (base << 32);              /* Bitwise OR */
    values[7] = values[5] ^ values[6];                 /* Bitwise XOR */
    
    /* 2. Sort the array (triggers many 128-bit comparisons) */
    sort_128bit_array(values, 8);
    
    /* 3. Array indexing with large offsets */
    unsigned __int128 offset = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce modulo array size */
        offset = (offset + values[i]) % (sizeof(huge_array) / sizeof(huge_array[0]));
        huge_array[offset] = (char)(values[i] & 0xFF);
        
        /* Additional offset calculation with multiplication */
        unsigned __int128 stride = ((unsigned __int128)i * 0x100000001ULL);
        unsigned __int128 idx = (values[i] * stride) % (sizeof(huge_array) / sizeof(huge_array[0]));
        huge_array[idx] ^= (char)((values[i] >> 8) & 0xFF);
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[7];
    unsigned __int128 step = (end - start) / 100;
    
    if (step == 0) step = 1;
    
    struct wide_bitfield wbf = {0};
    unsigned __int128 loop_checksum = 0;
    
    /* Loop forces 128-bit comparison in control logic */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* 5. Switch statement with large 128-bit case values */
        switch (i & 0xF) {  /* Reduced to 4 bits for reasonable switch size */
            case 0:
                wbf.low_bits = i;
                break;
            case 1:
                wbf.mid_bits = i >> 10;
                break;
            case 2:
                wbf.high_bits = i >> 60;
                break;
            case 3:
                wbf.low_bits ^= i;
                break;
            case 4:
                wbf.mid_bits ^= i >> 5;
                break;
            case 5:
                wbf.high_bits ^= i >> 55;
                break;
            default:
                /* Complex expression forcing 128-bit comparison */
                if ((i & 0xFFF) > (end & 0xFFF)) {
                    wbf.low_bits += i;
                } else if ((i & 0xFFF) < (start & 0xFFF)) {
                    wbf.mid_bits += i;
                } else {
                    wbf.high_bits += i;
                }
                break;
        }
        
        /* 6. Structure with wide bit-field manipulation */
        unsigned __int128 combined = ((unsigned __int128)wbf.high_bits << 120) |
                                     ((unsigned __int128)wbf.mid_bits << 70) |
                                     wbf.low_bits;
        
        loop_checksum = compute_checksum(loop_checksum + combined + i);
        
        /* Early exit to avoid excessive iterations */
        if (i > start + (step * 10)) break;
    }
    
    /* 7. Additional comparisons in conditional expressions */
    unsigned __int128 min_val = values[0];
    unsigned __int128 max_val = values[0];
    
    for (int i = 1; i < 8; i++) {
        /* More comparisons triggering double_int::cmp */
        if (values[i] < min_val) min_val = values[i];
        if (values[i] > max_val) max_val = values[i];
        
        /* Complex conditional with 128-bit comparisons */
        unsigned __int128 diff = (values[i] > values[i-1]) ? 
                                 (values[i] - values[i-1]) : 
                                 (values[i-1] - values[i]);
        
        if (diff > ((unsigned __int128)1 << 80)) {
            values[i] = values[i] ^ diff;
        }
    }
    
    /* 8. Final checksum calculation (prevents optimization) */
    unsigned __int128 final_checksum = 0;
    
    for (int i = 0; i < 8; i++) {
        final_checksum = compute_checksum(final_checksum + values[i]);
    }
    
    final_checksum = compute_checksum(final_checksum + loop_checksum);
    final_checksum = compute_checksum(final_checksum + 
                                     ((unsigned __int128)wbf.low_bits << 0) +
                                     ((unsigned __int128)wbf.mid_bits << 70) +
                                     ((unsigned __int128)wbf.high_bits << 120));
    
    /* Use array content in checksum */
    for (unsigned __int128 i = 0; i < 256 && i < sizeof(huge_array); i++) {
        final_checksum += huge_array[i];
    }
    
    /* Output to prevent dead code elimination */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(final_checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", 
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Min value: 0x%016llX%016llX\n",
           (unsigned long long)(min_val >> 64),
           (unsigned long long)(min_val & 0xFFFFFFFFFFFFFFFFULL));
    printf("Max value: 0x%016llX%016llX\n",
           (unsigned long long)(max_val >> 64),
           (unsigned long long)(max_val & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
