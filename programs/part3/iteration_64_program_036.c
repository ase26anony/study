/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* 128-bit integer array for sorting */
#define ARRAY_SIZE 8
static unsigned __int128 values[ARRAY_SIZE];

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:80;
    unsigned __int128 d:48;
} wbf;

/* Initialize 128-bit constants that exceed 64-bit range */
static void init_values(void) {
    /* Constants that require 128-bit representation */
    const unsigned __int128 base = (unsigned __int128)0x123456789ABCDEF0ULL;
    
    values[0] = base * 0x100000001ULL;  /* > 2^64 */
    values[1] = values[0] + 0xFFFFFFFFFFFFFFFULL;
    values[2] = values[0] << 3;
    values[3] = values[1] - 0x1000000000000000ULL;
    values[4] = (unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64;
    values[5] = values[4] + 0x123456789ABCDEFULL;
    values[6] = 0;
    values[7] = (unsigned __int128)1 << 127;  /* Maximum 128-bit value */
}

/* Simple bubble sort to force many 128-bit comparisons */
static void sort_values(void) {
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        for (int j = 0; j < ARRAY_SIZE - i - 1; j++) {
            /* This comparison will use double_int::cmp internally */
            if (values[j] > values[j + 1]) {
                unsigned __int128 temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }
}

/* Access array using 128-bit offsets (modulo to stay in bounds) */
static void access_with_offsets(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) - 1);
        huge_array[(size_t)offset] ^= (char)(values[i] & 0xFF);
    }
}

/* Loop with 128-bit counter and switch with large cases */
static void loop_with_wide_counter(void) {
    const unsigned __int128 start = values[0];
    const unsigned __int128 end = values[0] + 10;
    const unsigned __int128 step = 1;
    
    /* Loop with 128-bit induction variable */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with 128-bit case labels - forces comparison during compilation */
        switch ((uint64_t)i) {  /* Cast to 64-bit for switch, but i is 128-bit in comparisons */
            case 0x123456789ABCDEF0ULL:  /* Large constant */
                huge_array[0] += 1;
                break;
            case 0xFEDCBA9876543210ULL:
                huge_array[1] += 2;
                break;
            case 0xFFFFFFFFFFFFFFFFULL:
                huge_array[2] += 3;
                break;
            default:
                huge_array[3] += 4;
                break;
        }
        
        /* Additional comparison in loop condition */
        if (i > values[ARRAY_SIZE/2]) {
            huge_array[4] += 5;
        }
    }
}

/* Manipulate wide bit-field structure */
static void manipulate_bitfields(void) {
    /* Initialize with 128-bit values */
    wbf.a = values[0] & (((unsigned __int128)1 << 70) - 1);
    wbf.b = values[1] & (((unsigned __int128)1 << 58) - 1);
    wbf.c = values[2] & (((unsigned __int128)1 << 80) - 1);
    wbf.d = values[3] & (((unsigned __int128)1 << 48) - 1);
    
    /* Operations that may trigger offset calculations */
    wbf.a = wbf.a << 5;
    wbf.b = wbf.b >> 3;
    wbf.c = wbf.c | wbf.d;
    wbf.d = wbf.d & wbf.a;
}

/* Calculate checksum to prevent dead code elimination */
static unsigned __int128 calculate_checksum(void) {
    unsigned __int128 checksum = 0;
    
    /* Include array values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= values[i];
        checksum = (checksum << 1) | (checksum >> 127);
    }
    
    /* Include bit-field values */
    checksum += (unsigned __int128)wbf.a;
    checksum += (unsigned __int128)wbf.b;
    checksum += (unsigned __int128)wbf.c;
    checksum += (unsigned __int128)wbf.d;
    
    /* Include some array bytes */
    for (int i = 0; i < 16; i++) {
        checksum += (unsigned __int128)huge_array[i];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize the large array */
    memset(huge_array, 0, sizeof(huge_array));
    
    /* Initialize 128-bit values */
    init_values();
    
    /* Force many 128-bit comparisons through sorting */
    sort_values();
    
    /* Access array with 128-bit offset calculations */
    access_with_offsets();
    
    /* Loop with 128-bit counter and switch */
    loop_with_wide_counter();
    
    /* Manipulate wide bit-fields */
    manipulate_bitfields();
    
    /* Calculate and print checksum */
    unsigned __int128 checksum = calculate_checksum();
    
    /* Print parts of the checksum (since printf doesn't support __int128 directly) */
    uint64_t part1 = (uint64_t)(checksum >> 64);
    uint64_t part2 = (uint64_t)checksum;
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)part1, 
           (unsigned long long)part2);
    
    return 0;
}
