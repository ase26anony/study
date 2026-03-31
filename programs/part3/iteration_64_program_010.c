/* double_int_coverage.c
 * Designed to exercise GCC's internal double_int comparison logic
 * Compile with: gcc -std=gnu11 -O3 -fno-tree-vectorize double_int_coverage.c -o double_int_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:70;
    unsigned __int128 d:58;
} __attribute__((packed));

/* Helper function to print 128-bit values (for debugging) */
void print_u128(unsigned __int128 value) {
    uint64_t low = (uint64_t)value;
    uint64_t high = (uint64_t)(value >> 64);
    printf("0x%016" PRIx64 "%016" PRIx64, high, low);
}

/* Simple bubble sort for 128-bit values - forces many comparisons */
void sort_u128_array(unsigned __int128 arr[], int n) {
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
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 base1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 base2 = ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL;
    unsigned __int128 base3 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    unsigned __int128 base4 = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    /* Array of 128-bit values to be sorted */
    unsigned __int128 values[8];
    
    /* Generate values using arithmetic operations that require 128-bit precision */
    values[0] = base1 + base2;                    /* Addition */
    values[1] = base1 - base2;                    /* Subtraction */
    values[2] = base1 << 5;                       /* Left shift */
    values[3] = base2 >> 3;                       /* Right shift */
    values[4] = base3 * 3;                        /* Multiplication */
    values[5] = base4 / 2;                        /* Division */
    values[6] = (base1 & base2) | base3;          /* Bitwise operations */
    values[7] = base1 ^ base2 ^ base3;            /* More bitwise operations */
    
    printf("Original array:\n");
    for (int i = 0; i < 8; i++) {
        printf("  values[%d] = ", i);
        print_u128(values[i]);
        printf("\n");
    }
    
    /* Sort the array - this will trigger many double_int comparisons */
    sort_u128_array(values, 8);
    
    printf("\nSorted array:\n");
    for (int i = 0; i < 8; i++) {
        printf("  values[%d] = ", i);
        print_u128(values[i]);
        printf("\n");
    }
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long long checksum = 0;
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        /* Access array at calculated offset */
        huge_array[(size_t)offset] = (char)(i + 'A');
        checksum += (unsigned long long)offset + i;
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    printf("\nLoop with 128-bit counter:\n");
    unsigned __int128 loop_start = base1;
    unsigned __int128 loop_end = base1 + 100;
    unsigned __int128 loop_step = 7;
    
    for (unsigned __int128 i = loop_start; i < loop_end; i += loop_step) {
        /* Switch statement with large 128-bit case constants */
        switch ((uint64_t)(i & 0x3)) {  /* Reduced to 4 cases for simplicity */
            case 0:
                huge_array[(size_t)(i % sizeof(huge_array))] ^= 0x01;
                checksum += 1;
                break;
            case 1:
                huge_array[(size_t)(i % sizeof(huge_array))] ^= 0x02;
                checksum += 2;
                break;
            case 2:
                huge_array[(size_t)(i % sizeof(huge_array))] ^= 0x04;
                checksum += 3;
                break;
            case 3:
                huge_array[(size_t)(i % sizeof(huge_array))] ^= 0x08;
                checksum += 4;
                break;
        }
        
        /* Work with wide bit-field structure */
        struct WideBitfield wbf;
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 70) - 1);
        wbf.d = (i >> 198) & ((1ULL << 58) - 1);
        
        /* Manipulate bit-fields */
        wbf.a ^= wbf.b;
        wbf.c |= wbf.d;
        checksum += (unsigned long long)(wbf.a + wbf.c);
    }
    
    /* Additional comparisons in conditional statements */
    if (values[0] < values[1]) checksum += 1000;
    if (values[2] > values[3]) checksum += 2000;
    if (values[4] <= values[5]) checksum += 3000;
    if (values[6] >= values[7]) checksum += 4000;
    if (values[0] == values[0]) checksum += 5000;  /* Always true */
    if (values[1] != values[2]) checksum += 6000;  /* Likely true */
    
    /* Generate a final checksum to prevent dead code elimination */
    printf("\nFinal checksum: %llu\n", checksum);
    
    /* Verify array bounds weren't exceeded */
    printf("First few bytes of huge_array: ");
    for (int i = 0; i < 10 && i < sizeof(huge_array); i++) {
        printf("%02x ", (unsigned char)huge_array[i]);
    }
    printf("...\n");
    
    return 0;
}
