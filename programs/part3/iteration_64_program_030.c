#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to compute simple checksum to prevent dead code elimination */
static unsigned long long compute_checksum(unsigned __int128 value) {
    return (unsigned long long)(value >> 64) ^ (unsigned long long)value;
}

/* Simple bubble sort for 128-bit values to force many comparisons */
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

int main(void) {
    /* Initialize large 128-bit constants */
    unsigned __int128 base1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 base2 = ((unsigned __int128)0x9876543210FEDCBAULL << 64) | 0x0123456789ABCDEFULL;
    unsigned __int128 large_const = 0xFFFFFFFFFFFFFFFFULL;
    large_const = (large_const << 64) | 0xFFFFFFFFFFFFFFFFULL; /* 2^128 - 1 */
    
    /* Array of 128-bit values for sorting */
    unsigned __int128 values[8];
    
    /* Generate values using arithmetic operations that require 128-bit precision */
    values[0] = base1 + base2;
    values[1] = base1 - base2;
    values[2] = base1 * 3;
    values[3] = base2 * 5;
    values[4] = base1 << 3;
    values[5] = base2 >> 2;
    values[6] = large_const - base1;
    values[7] = (base1 + base2) * 7;
    
    /* Sort the array - this will perform many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long long sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then modulo to stay in bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        /* Access array using the offset */
        sum += huge_array[(size_t)offset];
        
        /* Modify array at calculated offset */
        huge_array[(size_t)offset] = (char)(i + 1);
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = base1;
    unsigned __int128 end = base1 + 100;
    unsigned __int128 step = 13;
    
    struct WideBitfield wbf = {0};
    unsigned __int128 switch_checksum = 0;
    
    /* This loop forces 128-bit comparisons in loop control */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch statement with large 128-bit case constants */
        switch ((unsigned long long)(i & 0x7)) { /* Use lower bits for switch */
            case 0:
                wbf.a = (i >> 10) & ((1ULL << 70) - 1);
                break;
            case 1:
                wbf.b = (i >> 5) & ((1ULL << 58) - 1);
                break;
            case 2:
                wbf.c = i & ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL);
                break;
            case 3:
                wbf.d = (i >> 120) & 0xFF;
                break;
            default:
                /* Manipulate bitfields */
                wbf.a ^= (i >> 20);
                wbf.b ^= (i >> 30);
                break;
        }
        
        /* Access array using 128-bit offset calculation */
        unsigned __int128 idx = (i * 7) % (sizeof(huge_array) - 1);
        huge_array[(size_t)idx] ^= (char)i;
        switch_checksum += i;
    }
    
    /* Compute final checksum from all operations */
    unsigned long long final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum += compute_checksum(values[i]);
    }
    
    final_checksum += compute_checksum(wbf.a);
    final_checksum += compute_checksum(wbf.b);
    final_checksum += compute_checksum(wbf.c);
    final_checksum += compute_checksum(wbf.d);
    final_checksum += compute_checksum(switch_checksum);
    final_checksum += sum;
    
    /* Use the checksum to prevent optimization */
    printf("Checksum: %llu\n", final_checksum);
    
    return (int)(final_checksum % 256);
}
