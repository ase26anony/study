/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 padding : 128 - 70 - 58;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Initialize array with pattern */
static void init_huge_array(void) {
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 31 + 17);
    }
}

/* Simple bubble sort for 128-bit values - forces many comparisons */
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

/* Generate a 128-bit constant that exceeds 64-bit range */
static unsigned __int128 make_large_constant(uint64_t base) {
    /* Create constant > 2^64 - 1 */
    return ((unsigned __int128)base << 64) | (base * 0xFEDCBA9876543210ULL);
}

int main(void) {
    init_huge_array();
    
    /* 1. Wide Integer Constant Folding */
    unsigned __int128 constants[8];
    
    /* Generate 128-bit constants that require double_int representation */
    constants[0] = 0x123456789ABCDEF0ULL * (unsigned __int128)0x100000001ULL;
    constants[1] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    constants[2] = make_large_constant(0xDEADBEEF);
    constants[3] = constants[0] + constants[1];
    constants[4] = constants[1] - constants[2];
    constants[5] = constants[0] << 3;
    constants[6] = constants[1] >> 2;
    constants[7] = ~constants[0];
    
    /* 2. Sort array - forces many 128-bit comparisons */
    sort_128bit_array(constants, 8);
    
    /* 3. Array Indexing with Large Offsets */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = constants[i] % sizeof(huge_array);
        checksum += (unsigned __int128)huge_array[(size_t)offset] * i;
    }
    
    /* 4. Loop Boundary Comparisons with Wide Integers */
    unsigned __int128 start = constants[0];
    unsigned __int128 end = constants[0] + 100;
    unsigned __int128 step = constants[1] % 10 + 1;
    
    struct wide_bitfield wbf = {0};
    wbf.low_part = constants[2] & (((unsigned __int128)1 << 70) - 1);
    wbf.high_part = (constants[2] >> 70) & (((unsigned __int128)1 << 58) - 1);
    
    /* 5. Switch Statements with Large Cases */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch on derived 128-bit values */
        unsigned __int128 switch_val = i & 0x7;  /* Lower 3 bits for switch */
        
        switch (switch_val) {
            case 0:
                wbf.low_part += i;
                break;
            case 1:
                wbf.high_part += i;
                break;
            case 2:
                wbf.low_part ^= i;
                break;
            case 3:
                wbf.high_part ^= i;
                break;
            case 4:
                wbf.low_part |= i;
                break;
            case 5:
                wbf.high_part |= i;
                break;
            case 6:
                wbf.low_part &= ~i;
                break;
            case 7:
                wbf.high_part &= ~i;
                break;
        }
        
        /* Access array using 128-bit offset calculation */
        unsigned __int128 idx = (i * 7) % sizeof(huge_array);
        checksum += (unsigned __int128)huge_array[(size_t)idx];
    }
    
    /* Add bitfield values to checksum */
    checksum += wbf.low_part;
    checksum += wbf.high_part;
    
    /* Add sorted constants to checksum */
    for (int i = 0; i < 8; i++) {
        checksum += constants[i] * (i + 1);
    }
    
    /* 6. Additional comparisons in conditional expressions */
    unsigned __int128 max_val = 0;
    for (int i = 0; i < 8; i++) {
        /* More comparisons that use double_int::cmp */
        if (constants[i] > max_val) {
            max_val = constants[i];
        }
        if (constants[i] < checksum) {
            checksum -= constants[i];
        }
    }
    
    /* Final output to prevent optimization */
    printf("Checksum (lower 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
