/* wide-int-compare.c - Target double-int.cc cmp function coverage */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Helper to add to checksum */
static void add_to_checksum(unsigned __int128 val) {
    checksum ^= val;
    checksum = (checksum << 1) | (checksum >> 127);
}

/* Simple bubble sort for 128-bit values */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
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

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[8];
    
    /* Large constants that require 128-bit representation */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    values[2] = values[0] + values[1];
    values[3] = values[1] - values[0];
    values[4] = values[0] << 3;
    values[5] = values[1] >> 2;
    values[6] = ~values[0];
    values[7] = values[0] ^ values[1];
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets using 128-bit calculations */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] * (i + 1);
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
        
        /* Access array - compiler calculates bounds with double_int */
        huge_array[safe_offset] ^= (char)(i + 1);
        add_to_checksum(values[i] + safe_offset);
    }
    
    /* Loop with 128-bit counter - triggers comparisons in loop control */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 100;
    unsigned __int128 step = 13;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i & 0x7)) {  /* Use lower bits for manageable switch */
            case 0:
                wbf.low_part = i;
                break;
            case 1:
                wbf.high_part = i >> 70;
                break;
            case 2:
                wbf.low_part ^= i;
                break;
            case 3:
                wbf.high_part ^= i >> 58;
                break;
            case 4:
                wbf.low_part += i;
                break;
            case 5:
                wbf.high_part += i >> 58;
                break;
            case 6:
                wbf.low_part = wbf.low_part * 3 + 1;
                break;
            case 7:
                wbf.high_part = wbf.high_part * 5 + 1;
                break;
        }
        
        /* More complex switch with computed cases */
        unsigned __int128 switch_val = i % 4;
        if (switch_val == 0) {
            add_to_checksum(i * 2);
        } else if (switch_val == 1) {
            add_to_checksum(i * 3);
        } else if (switch_val == 2) {
            add_to_checksum(i * 5);
        } else {
            add_to_checksum(i * 7);
        }
    }
    
    /* Add bitfield values to checksum */
    add_to_checksum(wbf.low_part);
    add_to_checksum(wbf.high_part);
    
    /* Additional 128-bit comparisons in conditional logic */
    unsigned __int128 a = values[7];
    unsigned __int128 b = values[6];
    
    if (a < b) {
        add_to_checksum(a - b);
    } else if (a > b) {
        add_to_checksum(b - a);
    } else {
        add_to_checksum(a + b);
    }
    
    /* Range checks with 128-bit values */
    unsigned __int128 min_val = values[0];
    unsigned __int128 max_val = values[7];
    
    for (int i = 0; i < 8; i++) {
        if (values[i] < min_val) {
            min_val = values[i];
        }
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    
    add_to_checksum(min_val);
    add_to_checksum(max_val);
    
    /* Complex expression with multiple comparisons */
    unsigned __int128 x = values[3];
    unsigned __int128 y = values[4];
    unsigned __int128 z = values[5];
    
    if ((x < y) && (y < z)) {
        add_to_checksum(x + y + z);
    } else if ((x > y) && (y > z)) {
        add_to_checksum(x - y - z);
    } else {
        add_to_checksum(x * y * z);
    }
    
    /* Output checksum to prevent optimization */
    unsigned char *bytes = (unsigned char *)&checksum;
    printf("Checksum bytes: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", bytes[i]);
    }
    printf("\n");
    
    return 0;
}
