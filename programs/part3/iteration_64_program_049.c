/* double_int_coverage.c - Program to exercise GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:70;
    unsigned __int128 d:58;
} __attribute__((packed));

/* Function to compute simple checksum */
static unsigned long long checksum = 0;

/* Bubble sort for unsigned __int128 array */
void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison will trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to access array using 128-bit offsets */
void access_with_128bit_offset(unsigned __int128 offset) {
    /* Modulo to stay within bounds */
    size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
    checksum += huge_array[safe_offset];
}

int main(void) {
    /* Initialize huge_array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 7 + 3);
    }
    
    /* 1. Wide Integer Constant Folding */
    /* Constants that exceed 64-bit range */
    unsigned __int128 val1 = ((unsigned __int128)0x123456789ABCDEF0ULL) * 0x100000001ULL;
    unsigned __int128 val2 = ((unsigned __int128)0xFEDCBA9876543210ULL) << 64;
    unsigned __int128 val3 = ~((unsigned __int128)0);  /* All 1s */
    unsigned __int128 val4 = ((unsigned __int128)1) << 120;  /* 2^120 */
    
    /* Arithmetic operations producing 128-bit results */
    unsigned __int128 sum = val1 + val2;
    unsigned __int128 diff = val3 - val1;
    unsigned __int128 prod = val1 * 2;
    unsigned __int128 shifted = val4 >> 1;
    
    /* 2. Array of 128-bit values for sorting */
    unsigned __int128 values[8];
    values[0] = val1;
    values[1] = val2;
    values[2] = val3;
    values[3] = val4;
    values[4] = sum;
    values[5] = diff;
    values[6] = prod;
    values[7] = shifted;
    
    /* Sort the array - triggers many comparisons */
    sort_128bit_array(values, 8);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = values[i] + ((unsigned __int128)i << 40);
        access_with_128bit_offset(offset);
    }
    
    /* 4. Loop with 128-bit counter */
    unsigned __int128 start = val1;
    unsigned __int128 end = val1 + 1000;
    unsigned __int128 step = 100;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* 5. Switch statement with large 128-bit cases */
        switch ((unsigned long long)(i & 0xFF)) {  /* Use lower bits for switch */
            case 0x10:
                checksum += 1;
                break;
            case 0x20:
                checksum += 2;
                break;
            case 0x30:
                checksum += 3;
                break;
            case 0x40:
                checksum += 4;
                break;
            default:
                checksum += 5;
        }
        
        /* 6. Structure with wide bit-field offsets */
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 70) - 1);  /* Will be 0 for our range */
        wbf.d = (i >> 198) & ((1ULL << 58) - 1);  /* Will be 0 for our range */
        
        checksum += (unsigned long long)wbf.a;
        checksum += (unsigned long long)wbf.b;
    }
    
    /* Additional comparisons to ensure coverage */
    if (val1 < val2) checksum += 10;
    if (val3 > val4) checksum += 20;
    if (val1 == val1) checksum += 30;
    
    /* Create array bounds checking with 128-bit values */
    unsigned __int128 large_index = val4;
    if (large_index < sizeof(huge_array)) {
        checksum += huge_array[(size_t)large_index % sizeof(huge_array)];
    }
    
    /* Force use of sorted values */
    for (int i = 0; i < 8; i++) {
        checksum += (unsigned long long)(values[i] >> 64);
        checksum += (unsigned long long)(values[i] & 0xFFFFFFFFFFFFFFFFULL);
    }
    
    /* Output checksum to prevent dead code elimination */
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
