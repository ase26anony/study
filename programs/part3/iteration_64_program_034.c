/* double_int_coverage.c - Target GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 low_bits: 70;
    unsigned __int128 mid_bits: 58;
    unsigned __int128 high_bits: 72;
    unsigned __int128 padding: 56;
};

/* Function to generate checksum to prevent dead code elimination */
static unsigned long long compute_checksum(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned long long checksum = 0x123456789ABCDEFULL;
    for (size_t i = 0; i < size; i++) {
        checksum = (checksum * 31) + bytes[i];
    }
    return checksum;
}

/* Simple bubble sort to force many 128-bit comparisons */
void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison will use double_int::cmp internally */
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
    
    /* Generate 128-bit constants using multiplication that overflows 64-bit */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL;
    values[2] = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x123456789ABCDEFULL;
    values[3] = values[0] + values[1];
    values[4] = values[1] - values[2];
    values[5] = values[0] << 3;  /* Left shift to create large values */
    values[6] = values[1] >> 2;  /* Right shift */
    values[7] = ~(values[0] ^ values[1]);  /* Bitwise operations */
    
    /* Sort the array - triggers many double_int comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce modulo array size */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / sizeof(huge_array[0]));
        huge_array[(size_t)offset] = (char)(i + 'A');
        offset_sum += (unsigned long long)offset;
    }
    
    /* Loop with 128-bit counter - forces comparison in loop control */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 100;
    
    for (unsigned __int128 i = start; i < end; i += 5) {
        /* Switch with large 128-bit case constants */
        switch ((unsigned long long)(i & 0xFF)) {
            case 0x10:
                loop_checksum += i * 2;
                break;
            case 0x20:
                loop_checksum += i / 2;
                break;
            case 0x30:
                loop_checksum += i + 0x1000;
                break;
            case 0x40:
                loop_checksum += i - 0x1000;
                break;
            default:
                loop_checksum += i;
                break;
        }
        
        /* Access structure with wide bit-fields */
        struct WideBitfield wbf;
        wbf.low_bits = i & ((1ULL << 70) - 1);
        wbf.mid_bits = (i >> 70) & ((1ULL << 58) - 1);
        wbf.high_bits = (i >> 128) & ((1ULL << 72) - 1);
        
        /* Force bit-field comparisons */
        if (wbf.low_bits > wbf.mid_bits) {
            loop_checksum += wbf.low_bits;
        }
        if (wbf.mid_bits < wbf.high_bits) {
            loop_checksum += wbf.high_bits;
        }
    }
    
    /* Additional comparisons in conditional statements */
    unsigned __int128 cmp_result = 0;
    
    if (values[0] < values[1]) cmp_result += 1;
    if (values[1] > values[2]) cmp_result += 2;
    if (values[2] <= values[3]) cmp_result += 4;
    if (values[3] >= values[4]) cmp_result += 8;
    if (values[4] == values[5]) cmp_result += 16;
    if (values[5] != values[6]) cmp_result += 32;
    
    /* Create a checksum from all results to prevent optimization */
    unsigned long long final_checksum = 0;
    final_checksum += compute_checksum(values, sizeof(values));
    final_checksum += offset_sum;
    final_checksum += (unsigned long long)(loop_checksum & 0xFFFFFFFFFFFFFFFFULL);
    final_checksum += (unsigned long long)(loop_checksum >> 64);
    final_checksum += (unsigned long long)cmp_result;
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %llu\n", final_checksum % 1000000);
    
    return 0;
}
