/* wide-int-test.c - Test program for double_int comparisons */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to compute simple checksum */
static unsigned long long checksum = 0;

/* Bubble sort for unsigned __int128 array */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison should trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                checksum ^= (unsigned long long)(temp >> 64);
            }
        }
    }
}

/* Function to access array using 128-bit offsets */
static void access_with_large_offsets(unsigned __int128 offsets[], int n) {
    unsigned __int128 array_size = sizeof(huge_array);
    
    for (int i = 0; i < n; i++) {
        /* Calculate offset within bounds */
        unsigned __int128 offset = offsets[i] % array_size;
        
        /* Access array - compiler will generate bounds checking */
        huge_array[(size_t)offset] ^= (char)(offset & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
}

int main(void) {
    /* Initialize with large 128-bit constants */
    unsigned __int128 values[10];
    
    /* Constants that exceed 64-bit range */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    values[2] = values[0] + values[1];
    values[3] = values[0] * 3;
    values[4] = values[1] >> 32;
    values[5] = values[2] - values[0];
    values[6] = values[3] << 16;
    values[7] = ~values[0];
    values[8] = values[4] | values[5];
    values[9] = values[6] & values[7];
    
    /* Sort array - generates many 128-bit comparisons */
    sort_128bit_array(values, 10);
    
    /* Access array with large offsets */
    access_with_large_offsets(values, 10);
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = values[0];
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct wide_bitfield wbf = {0};
    wbf.a = values[1] & ((1ULL << 70) - 1);
    wbf.b = values[2] & ((1ULL << 58) - 1);
    wbf.c = values[3] & ((1ULL << 72) - 1);
    wbf.d = values[4] & ((1ULL << 56) - 1);
    
    /* Loop with 128-bit induction variable */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch ((unsigned long long)(i & 0xFF)) {
            case 0x10:
                wbf.a ^= (i & 0x3F);
                break;
            case 0x20:
                wbf.b ^= (i & 0x1F);
                break;
            case 0x30:
                wbf.c ^= (i & 0x7F);
                break;
            case 0x40:
                wbf.d ^= (i & 0x0F);
                break;
            default:
                wbf.a += 1;
                break;
        }
        
        /* More comparisons in loop condition */
        if (i > start + 500) {
            wbf.b += (i - start);
        }
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons */
    if (values[9] > values[8]) {
        checksum += values[9] - values[8];
    }
    
    if (values[0] < values[1]) {
        checksum += values[1] - values[0];
    }
    
    /* Final checksum to prevent optimization */
    printf("Checksum: %llu\n", checksum % 1000000);
    
    return 0;
}
