/* double_int_coverage.c - Targets GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 full : 128;
};

/* Function to compute simple checksum */
static unsigned long long compute_checksum(const void *data, size_t len) {
    unsigned long long sum = 0;
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        sum = (sum * 31) + p[i];
    }
    return sum;
}

/* Simple bubble sort for 128-bit values (forces many comparisons) */
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
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[8];
    
    /* Large constants that require 128-bit representation */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = (unsigned __int128)0xFEDCBA9876543210ULL << 64;
    values[2] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 32) | 0xAAAAAAAAAAAAAAAALLU;
    values[3] = values[0] + values[1];
    values[4] = values[1] - values[2];
    values[5] = values[0] * 3;
    values[6] = values[2] >> 16;
    values[7] = ~(values[0] ^ values[1]);
    
    /* Sort array - forces many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] * (i + 1);
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / 2));
        
        /* Access array at calculated offset */
        huge_array[safe_offset] = (char)(offset & 0xFF);
        offset_sum += huge_array[safe_offset];
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = values[2];
    unsigned __int128 end = start + 100;
    unsigned __int128 step = 13;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((unsigned long long)(i & 0xFFFF)) {
            case 0x1234:
                loop_checksum += i * 2;
                break;
            case 0x5678:
                loop_checksum += i * 3;
                break;
            case 0x9ABC:
                loop_checksum += i * 5;
                break;
            case 0xDEF0:
                loop_checksum += i * 7;
                break;
            default:
                loop_checksum += i;
        }
        
        /* Structure manipulation with wide bit-fields */
        struct wide_bitfield wbf;
        wbf.low_part = (i >> 32) & ((1ULL << 70) - 1);
        wbf.high_part = (i >> 102) & ((1ULL << 58) - 1);
        wbf.full = (wbf.high_part << 70) | wbf.low_part;
        
        loop_checksum += wbf.full;
    }
    
    /* Additional comparisons in conditional statements */
    unsigned __int128 cmp_result = 0;
    if (values[0] < values[1]) cmp_result += 1;
    if (values[1] > values[2]) cmp_result += 2;
    if (values[2] <= values[3]) cmp_result += 4;
    if (values[3] >= values[4]) cmp_result += 8;
    if (values[4] == values[5]) cmp_result += 16;
    if (values[5] != values[6]) cmp_result += 32;
    
    /* Force use of all values to prevent optimization */
    unsigned __int128 final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum = final_checksum * 0x100000001ULL + values[i];
    }
    final_checksum += loop_checksum + cmp_result + offset_sum;
    
    /* Output to prevent dead code elimination */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(final_checksum >> 64),
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
