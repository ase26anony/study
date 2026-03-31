/* wide-int-test.c - Target double-int.cc cmp function coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for unsigned __int128 array */
static void sort_wide_array(unsigned __int128 arr[], int n) {
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

/* Generate large 128-bit constants */
static unsigned __int128 make_large_constant(uint64_t hi, uint64_t lo) {
    return ((unsigned __int128)hi << 64) | lo;
}

int main(void) {
    /* Initialize huge_array to prevent UB */
    memset(huge_array, 0xAA, sizeof(huge_array));
    
    /* Create array of wide integers requiring 128-bit comparisons */
    unsigned __int128 wide_values[8];
    
    /* Constants that exceed 64-bit range */
    wide_values[0] = make_large_constant(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
    wide_values[1] = make_large_constant(0x0, 0xFFFFFFFFFFFFFFFFULL); /* 2^64 - 1 */
    wide_values[2] = make_large_constant(0x1, 0x0); /* 2^64 */
    wide_values[3] = wide_values[0] + wide_values[1]; /* Arithmetic producing >64-bit */
    wide_values[4] = wide_values[0] << 3; /* Shift producing >64-bit */
    wide_values[5] = wide_values[2] * 0x100000001ULL; /* Multiplication >64-bit */
    wide_values[6] = make_large_constant(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
    wide_values[7] = ~(unsigned __int128)0; /* Max 128-bit value */
    
    /* Sort array - triggers many double_int comparisons */
    sort_wide_array(wide_values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = wide_values[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] ^= (char)(checksum & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* Loop with 128-bit counter and comparisons */
    unsigned __int128 start = make_large_constant(0x1000, 0x0);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 7;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i & 0xFF)) {  /* Reduced for practicality, but compiler still sorts cases */
            case 0x10: wbf.a = i; break;
            case 0x20: wbf.b = i; break;
            case 0x30: wbf.c = i; break;
            case 0x40: wbf.d = i; break;
            default:   wbf.a ^= i; break;
        }
        
        /* Complex 128-bit arithmetic that may be constant-folded */
        unsigned __int128 temp = i * 0x123456789ABCDEFULL;
        if (temp > wide_values[4]) {
            wbf.b ^= temp;
        }
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* More 128-bit comparisons in conditional expressions */
    unsigned __int128 threshold = make_large_constant(0x8000000000000000ULL, 0x0);
    for (int i = 0; i < 8; i++) {
        if (wide_values[i] > threshold) {
            wide_values[i] >>= 1;
        } else if (wide_values[i] < (threshold >> 2)) {
            wide_values[i] <<= 1;
        }
        
        checksum += wide_values[i];
    }
    
    /* Final checksum output to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
