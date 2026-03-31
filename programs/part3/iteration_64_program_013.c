/* wide_int_operations.c - Targets GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning > 64 bits */
struct WideBitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to compute simple checksum */
static unsigned long long compute_checksum(unsigned __int128 value) {
    return (unsigned long long)(value >> 64) ^ (unsigned long long)value;
}

int main(void) {
    /* 1. Declare and initialize 128-bit variables with large constants */
    unsigned __int128 base = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 large_const = ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL;
    unsigned __int128 mask = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* 2. Array of 128-bit values for sorting */
    unsigned __int128 values[8];
    
    /* Initialize with various 128-bit values */
    values[0] = base;
    values[1] = large_const;
    values[2] = base + large_const;
    values[3] = base - (large_const >> 3);
    values[4] = base * 3;
    values[5] = large_const * 5;
    values[6] = (base << 2) | (large_const >> 2);
    values[7] = mask - base;
    
    /* 3. Sort array using bubble sort (many comparisons) */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 7 - i; j++) {
            /* Each comparison triggers double_int::cmp */
            if (values[j] > values[j + 1]) {
                unsigned __int128 temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }
    
    /* 4. Array indexing with large offsets */
    unsigned long long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array size */
        unsigned __int128 offset_calc = values[i] % (1ULL << 31);
        unsigned long long offset = (unsigned long long)offset_calc;
        
        /* Access array (initialize with pattern) */
        huge_array[offset] = (char)(i + 'A');
        offset_sum += offset;
    }
    
    /* 5. Loop with 128-bit counter and switch statement */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[7];
    unsigned __int128 step = (end - start) / 16;
    
    if (step == 0) step = 1;
    
    /* Loop forces 128-bit comparison in control logic */
    for (unsigned __int128 i = start; i < end && i < start + 100; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((unsigned long long)(i & 0xFF)) {
            case 0x10:
                loop_checksum += i * 2;
                break;
            case 0x20:
                loop_checksum += i * 3;
                break;
            case 0x30:
                loop_checksum += i * 5;
                break;
            case 0x40:
                loop_checksum += i * 7;
                break;
            default:
                loop_checksum += i;
                break;
        }
        
        /* 6. Structure with wide bit-field manipulation */
        struct WideBitfield wbf;
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (i >> 200) & ((1ULL << 56) - 1);
        
        /* More comparisons in bit-field operations */
        if (wbf.a > wbf.b) {
            loop_checksum += wbf.a - wbf.b;
        }
        if (wbf.c < wbf.d) {
            loop_checksum += wbf.d - wbf.c;
        }
    }
    
    /* 7. Final checksum to prevent dead code elimination */
    unsigned long long final_checksum = offset_sum;
    
    for (int i = 0; i < 8; i++) {
        final_checksum ^= compute_checksum(values[i]);
    }
    
    final_checksum ^= compute_checksum(loop_checksum);
    
    /* Use the result */
    printf("Checksum: 0x%016llx\n", final_checksum);
    
    return (int)(final_checksum & 0xFF);
}
