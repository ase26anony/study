/* wide-int-comparisons.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O2 -fdump-tree-optimized wide-int-comparisons.c -o wide-int-comparisons
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Compare function for sorting (triggers double_int::cmp internally) */
static int compare_u128(const void *a, const void *b) {
    unsigned __int128 val_a = *(const unsigned __int128 *)a;
    unsigned __int128 val_b = *(const unsigned __int128 *)b;
    
    /* These comparisons will be lowered to double_int::cmp */
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

/* Simple bubble sort to force many comparisons */
static void sort_u128_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (compare_u128(&arr[j], &arr[j + 1]) > 0) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

int main(void) {
    /* Initialize with 128-bit constants that exceed 64-bit range */
    unsigned __int128 values[10];
    
    /* Large constants requiring 128-bit representation */
    values[0] = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    values[1] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    values[2] = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    values[3] = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    values[4] = 0x10000000000000000ULL;  /* Exactly 2^64 */
    values[5] = 0xFFFFFFFFFFFFFFFFULL;   /* Max 64-bit */
    values[6] = values[0] + values[1];   /* Arithmetic producing 128-bit results */
    values[7] = values[2] - 1;
    values[8] = values[3] << 2;          /* Shift producing 128-bit result */
    values[9] = values[4] * 3;
    
    /* Sort the array - forces many 128-bit comparisons */
    sort_u128_array(values, 10);
    
    /* Array indexing with large offsets using 128-bit calculations */
    for (int i = 0; i < 10; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] = (char)(i + 'A');
        checksum += (unsigned __int128)huge_array[(size_t)offset] * offset;
    }
    
    /* Loop with 128-bit counter and comparisons */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 100;
    unsigned __int128 step = 10;
    
    struct wide_bitfield wbf = {0};
    wbf.a = values[1] & (((unsigned __int128)1 << 70) - 1);
    wbf.b = values[2] & (((unsigned __int128)1 << 58) - 1);
    wbf.c = values[3] & (((unsigned __int128)1 << 120) - 1);
    wbf.d = values[4] & 0xFF;
    
    /* Loop with 128-bit induction variable - forces comparisons in loop control */
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch ((uint64_t)(i & 0xFFFFFFFFFFFFFFF0ULL)) {
            case 0x1000000000000000ULL:  /* Large case value */
                wbf.a ^= (i & (((unsigned __int128)1 << 70) - 1));
                checksum += wbf.a;
                break;
            case 0x2000000000000000ULL:
                wbf.b ^= (i & (((unsigned __int128)1 << 58) - 1));
                checksum += wbf.b;
                break;
            case 0x3000000000000000ULL:
                wbf.c ^= (i & (((unsigned __int128)1 << 120) - 1));
                checksum += wbf.c;
                break;
            default:
                wbf.d ^= (uint8_t)i;
                checksum += wbf.d;
                break;
        }
        
        /* More 128-bit comparisons in loop body */
        if (i > values[5]) {
            checksum += i;
        }
        if (i < values[6]) {
            checksum -= i;
        }
    }
    
    /* Additional 128-bit constant comparisons */
    unsigned __int128 large_const = 
        ((unsigned __int128)0xDEADBEEFCAFEBABEULL << 64) | 0x1234567890ABCDEFULL;
    
    /* Series of comparisons that will trigger the uncovered block */
    if (values[0] < large_const) checksum += 1;
    if (values[1] > large_const) checksum += 2;
    if (values[2] < large_const) checksum += 3;
    if (values[3] > large_const) checksum += 4;
    if (values[4] == large_const) checksum += 5;
    if (values[5] != large_const) checksum += 6;
    
    /* Complex expression with multiple 128-bit comparisons */
    unsigned __int128 threshold = ((unsigned __int128)1 << 127);
    for (int i = 0; i < 10; i++) {
        if (values[i] > threshold && values[i] < (threshold << 1)) {
            checksum = checksum * 3 + values[i];
        }
    }
    
    /* Prevent the entire computation from being optimized away */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
