/* wide_int_operations.c - Targeting double_int::cmp uncovered lines */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 padding : 128 - 70 - 58;
};

/* Function to compute simple checksum */
static unsigned long compute_checksum(const void *data, size_t len) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned long sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (sum * 31) + bytes[i];
    }
    return sum;
}

/* Bubble sort for 128-bit values (forces many comparisons) */
static void sort_128bit_array(unsigned __int128 *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* These comparisons will trigger double_int::cmp */
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
    unsigned __int128 base = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                             0xFEDCBA9876543210ULL;
    
    unsigned __int128 multiplier = 0x100000001ULL;
    unsigned __int128 large_constant = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Array of 128-bit values for sorting */
    unsigned __int128 values[8];
    
    /* Generate values using 128-bit arithmetic */
    values[0] = base;
    values[1] = base + large_constant;
    values[2] = base * multiplier;
    values[3] = base << 3;
    values[4] = base - large_constant;
    values[5] = (base >> 2) + (large_constant << 4);
    values[6] = base ^ large_constant;
    values[7] = ~base;
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Access large array using 128-bit offsets */
    unsigned long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset within bounds using modulo */
        size_t offset = (size_t)(values[i] % (sizeof(huge_array) / 8));
        huge_array[offset] = (char)(values[i] & 0xFF);
        offset_sum += huge_array[offset];
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = base >> 4;
    unsigned __int128 end = start + 100;
    unsigned __int128 step = 7;
    
    struct WideBitfield wbf = {0};
    unsigned __int128 switch_checksum = 0;
    
    /* This loop forces 128-bit comparisons in loop control */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch (i & 0xF) {  /* Reduced to 16 cases for simplicity */
            case 0:  wbf.low_part = i; break;
            case 1:  wbf.high_part = i >> 70; break;
            case 2:  wbf.low_part += i; break;
            case 3:  wbf.high_part ^= i; break;
            case 4:  wbf.low_part = i * 3; break;
            case 5:  wbf.high_part = i / 5; break;
            case 6:  wbf.low_part |= i; break;
            case 7:  wbf.high_part &= i; break;
            case 8:  wbf.low_part = ~i; break;
            case 9:  wbf.high_part = i << 2; break;
            case 10: wbf.low_part = i >> 1; break;
            case 11: wbf.high_part = i % 13; break;
            case 12: wbf.low_part ^= wbf.high_part; break;
            case 13: wbf.high_part |= wbf.low_part; break;
            case 14: wbf.low_part = i + wbf.high_part; break;
            case 15: wbf.high_part = i - wbf.low_part; break;
        }
        switch_checksum += i + wbf.low_part + wbf.high_part;
    }
    
    /* Additional 128-bit comparisons in conditionals */
    unsigned __int128 cmp_a = values[3] * values[5];
    unsigned __int128 cmp_b = values[2] << 4;
    
    int comparison_results = 0;
    if (cmp_a < cmp_b) comparison_results |= 1;
    if (cmp_a > cmp_b) comparison_results |= 2;
    if (cmp_a == cmp_b) comparison_results |= 4;
    if (cmp_a <= cmp_b) comparison_results |= 8;
    if (cmp_a >= cmp_b) comparison_results |= 16;
    if (cmp_a != cmp_b) comparison_results |= 32;
    
    /* Compute final checksum to prevent optimization */
    unsigned long final_checksum = 0;
    final_checksum += compute_checksum(values, sizeof(values));
    final_checksum += compute_checksum(&wbf, sizeof(wbf));
    final_checksum += offset_sum;
    final_checksum += (unsigned long)switch_checksum;
    final_checksum += (unsigned long)(switch_checksum >> 64);
    final_checksum += comparison_results;
    
    /* Use the result */
    printf("Checksum: %lu\n", final_checksum % 1000000);
    
    return (final_checksum > 0) ? 0 : 1;
}
