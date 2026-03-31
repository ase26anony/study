/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for 128-bit integer comparisons
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 20];  /* 1MB array for 32-bit compatibility */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:70;
    unsigned __int128 d:58;
} __attribute__((packed));

/* Function to generate checksum to prevent dead code elimination */
static unsigned long long compute_checksum(const void *data, size_t len) {
    unsigned long long sum = 0;
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        sum = (sum * 31) + p[i];
    }
    return sum;
}

/* Simple bubble sort to force many 128-bit comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
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
    /* Initialize huge_array with pattern */
    for (size_t i = 0; i < sizeof(huge_array); i++) {
        huge_array[i] = (char)(i * 7);
    }
    
    /* 1. Wide Integer Constant Folding */
    /* Constants that exceed 64-bit range */
    const unsigned __int128 c1 = ((unsigned __int128)0x123456789ABCDEF0ULL) * 0x100000001ULL;
    const unsigned __int128 c2 = ((unsigned __int128)0xFEDCBA9876543210ULL) << 64;
    const unsigned __int128 c3 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) + 1;  /* Exactly 2^64 */
    const unsigned __int128 c4 = ~((unsigned __int128)0);  /* All bits set */
    
    /* Arithmetic operations producing 128-bit results */
    unsigned __int128 vals[8];
    vals[0] = c1;
    vals[1] = c2;
    vals[2] = c3;
    vals[3] = c4;
    vals[4] = c1 + c2;          /* Addition */
    vals[5] = c3 * 3;           /* Multiplication */
    vals[6] = c4 - c1;          /* Subtraction */
    vals[7] = c2 >> 32;         /* Right shift */
    
    /* 2. Sort array to force many comparisons */
    sort_128bit_array(vals, 8);
    
    /* 3. Array indexing with large offsets */
    unsigned long long offset_sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = vals[i] % sizeof(huge_array);
        offset_sum += huge_array[(size_t)offset];
        
        /* Additional offset calculations with different strides */
        unsigned __int128 offset2 = (vals[i] * 7) % sizeof(huge_array);
        offset_sum += huge_array[(size_t)offset2];
    }
    
    /* 4. Loop boundary comparisons with wide integers */
    unsigned __int128 loop_counter = c3;  /* Start at 2^64 */
    const unsigned __int128 loop_end = c3 + 1000;
    unsigned __int128 loop_sum = 0;
    
    for (unsigned __int128 i = loop_counter; i < loop_end; i += 17) {
        /* 5. Switch statement with large 128-bit cases */
        /* Use modulo to create manageable number of cases */
        unsigned __int128 switch_val = i % 8;
        
        switch ((unsigned long long)switch_val) {
            case 0:
                loop_sum += i * 2;
                break;
            case 1:
                loop_sum += i + 0x100000000ULL;
                break;
            case 2:
                loop_sum += i - 0x100000000ULL;
                break;
            case 3:
                loop_sum += i << 3;
                break;
            case 4:
                loop_sum += i >> 2;
                break;
            case 5:
                loop_sum += i * 3;
                break;
            case 6:
                loop_sum += i / 5;
                break;
            case 7:
                loop_sum += i % 13;
                break;
        }
        
        /* 6. Structure with wide bit-field offsets */
        struct wide_bitfield wbf;
        memset(&wbf, 0, sizeof(wbf));
        
        /* Set bit-fields using 128-bit values */
        wbf.a = i & (((unsigned __int128)1 << 70) - 1);
        wbf.b = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        wbf.c = (i >> 128) & (((unsigned __int128)1 << 70) - 1);
        wbf.d = (i >> 198) & (((unsigned __int128)1 << 58) - 1);
        
        /* Access bit-fields in sequence */
        loop_sum += (unsigned long long)wbf.a;
        loop_sum += (unsigned long long)wbf.b;
        loop_sum += (unsigned long long)wbf.c;
        loop_sum += (unsigned long long)wbf.d;
    }
    
    /* Combine all results into final checksum */
    unsigned long long final_checksum = offset_sum + loop_sum;
    
    /* Add array values to checksum */
    for (int i = 0; i < 8; i++) {
        final_checksum += (unsigned long long)(vals[i] >> 64);
        final_checksum += (unsigned long long)(vals[i] & 0xFFFFFFFFFFFFFFFFULL);
    }
    
    /* Output checksum to prevent optimization */
    printf("Checksum: %llu\n", final_checksum);
    
    return 0;
}
