/* wide-int-comparisons.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize wide-int-comparisons.c -o wide-int-comparisons
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 120;
    unsigned __int128 d: 8;
} wbf;

/* Function to compute simple checksum */
static unsigned long long checksum = 0;

static void update_checksum(unsigned __int128 val) {
    checksum ^= (unsigned long long)(val >> 64);
    checksum ^= (unsigned long long)val;
    checksum = (checksum << 1) | (checksum >> 63);
}

/* Bubble sort for 128-bit values - forces many comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* Each comparison should trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main(void) {
    /* Initialize large 128-bit constants */
    unsigned __int128 base1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 base2 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    unsigned __int128 base3 = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    unsigned __int128 base4 = ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL;
    
    /* Array of 128-bit values to be sorted */
    unsigned __int128 values[8];
    
    /* Generate values using various 128-bit operations */
    values[0] = base1;
    values[1] = base2;
    values[2] = base3;
    values[3] = base4;
    values[4] = base1 + base2;                    /* Addition */
    values[5] = base3 - base4;                    /* Subtraction */
    values[6] = base1 << 3;                       /* Left shift */
    values[7] = ((unsigned __int128)base2 * base3) >> 64;  /* Multiplication */
    
    /* Sort the array - forces many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Access huge_array using 128-bit offset calculations */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce modulo array size */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / sizeof(huge_array[0]));
        huge_array[(size_t)offset] ^= (char)(values[i] & 0xFF);
        update_checksum(values[i]);
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    unsigned __int128 start = base1 & 0xFFFF;
    unsigned __int128 end = start + 100;
    unsigned __int128 step = 7;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with 128-bit case values - forces sorting/comparison during compilation */
        switch ((unsigned long long)(i & 0xFFFFFFFF)) {
            case 0x12345678ULL:
                update_checksum(i * 2);
                break;
            case 0x87654321ULL:
                update_checksum(i / 2);
                break;
            case 0xABCDEF01ULL:
                update_checksum(i + 0x1000);
                break;
            case 0xFEDCBA98ULL:
                update_checksum(i - 0x1000);
                break;
            default:
                update_checksum(i);
        }
        
        /* Manipulate wide bit-field structure */
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 120) - 1);
        wbf.d = (i >> 248) & 0xFF;
        
        update_checksum(wbf.a);
        update_checksum(wbf.b);
        update_checksum(wbf.c);
        update_checksum(wbf.d);
    }
    
    /* Additional comparisons in conditional expressions */
    unsigned __int128 cmp1 = base1 * 2;
    unsigned __int128 cmp2 = base2 / 2;
    
    if (cmp1 > cmp2) {
        update_checksum(cmp1 - cmp2);
    } else if (cmp1 < cmp2) {
        update_checksum(cmp2 - cmp1);
    } else {
        update_checksum(cmp1 + cmp2);
    }
    
    /* Array of pointers with 128-bit index calculations */
    void* ptr_array[100];
    for (int i = 0; i < 100; i++) {
        ptr_array[i] = &huge_array[i * 1000];
    }
    
    /* Access using 128-bit index calculation */
    unsigned __int128 idx = (base3 % 50) + (base4 % 50);
    if (idx < 100) {
        char* ptr = (char*)ptr_array[(int)idx];
        *ptr ^= 0xAA;
        update_checksum((unsigned __int128)(uintptr_t)ptr);
    }
    
    /* Final checksum output to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
