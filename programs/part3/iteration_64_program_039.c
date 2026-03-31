/* wide-int-comparisons.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-comparisons.c -o wide-int-comparisons
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 24];  /* 16MB array */

/* Structure with wide bit-fields spanning more than 64 bits */
struct wide_bitfield {
    unsigned __int128 field_a : 70;
    unsigned __int128 field_b : 58;
    unsigned __int128 field_c : 80;
    unsigned __int128 field_d : 50;
};

/* Function to perform 128-bit comparisons via sorting */
static void sort_128bit_array(unsigned __int128 *arr, int size) {
    /* Bubble sort to force many comparisons */
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* These comparisons will use double_int::cmp internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Function to calculate array offset using 128-bit arithmetic */
static size_t calculate_offset(unsigned __int128 base, unsigned __int128 index, 
                               unsigned __int128 stride) {
    /* Force compiler to use 128-bit arithmetic and comparisons */
    unsigned __int128 offset = base + index * stride;
    
    /* Compare offset against array bounds */
    if (offset >= (unsigned __int128)(sizeof(huge_array))) {
        return 0;
    }
    return (size_t)offset;
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL,
        (unsigned __int128)1ULL << 70,
        (unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64,
        (unsigned __int128)0x5555555555555555LL * 0x1000000000000000LL,
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL,
        (unsigned __int128)0x8000000000000000ULL << 32,
        (unsigned __int128)0x1234567890ABCDEFULL * 0xFEDCBA0987654321ULL
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Sort array of 128-bit constants (forces many comparisons) */
    printf("Sorting 128-bit constants...\n");
    sort_128bit_array(constants, num_constants);
    
    /* 2. Array indexing with large offsets */
    printf("Accessing array with 128-bit offsets...\n");
    unsigned char checksum = 0;
    
    for (int i = 0; i < num_constants; i++) {
        /* Calculate offset using 128-bit arithmetic */
        size_t offset = calculate_offset(
            constants[i], 
            (unsigned __int128)i * 17, 
            (unsigned __int128)3
        );
        
        /* Access array (ensure within bounds) */
        if (offset < sizeof(huge_array)) {
            huge_array[offset] = (char)(constants[i] & 0xFF);
            checksum ^= huge_array[offset];
        }
    }
    
    /* 3. Loop with 128-bit counter and comparisons */
    printf("Looping with 128-bit counter...\n");
    unsigned __int128 loop_counter = constants[0];
    unsigned __int128 loop_end = constants[0] + 1000;
    unsigned __int128 loop_sum = 0;
    
    for (unsigned __int128 i = loop_counter; i < loop_end; i += 7) {
        /* Force comparison in loop condition */
        loop_sum += i;
        
        /* 4. Switch statement with large 128-bit cases */
        switch ((uint64_t)(i & 0x7)) {  /* Reduced to 8 cases for simplicity */
            case 0:
                loop_sum += 1;
                break;
            case 1:
                loop_sum += 2;
                break;
            case 2:
                loop_sum += 3;
                break;
            case 3:
                loop_sum += 4;
                break;
            case 4:
                loop_sum += 5;
                break;
            case 5:
                loop_sum += 6;
                break;
            case 6:
                loop_sum += 7;
                break;
            case 7:
                loop_sum += 8;
                break;
        }
    }
    
    /* 5. Structure with wide bit-fields */
    printf("Using structure with wide bit-fields...\n");
    struct wide_bitfield wbf = {0};
    
    /* Set fields using 128-bit constants */
    wbf.field_a = constants[1] & (((unsigned __int128)1 << 70) - 1);
    wbf.field_b = constants[2] & (((unsigned __int128)1 << 58) - 1);
    wbf.field_c = constants[3] & (((unsigned __int128)1 << 80) - 1);
    wbf.field_d = constants[4] & (((unsigned __int128)1 << 50) - 1);
    
    /* Perform operations that might trigger comparisons */
    if (wbf.field_a > wbf.field_b) {
        wbf.field_c = wbf.field_a - wbf.field_b;
    } else {
        wbf.field_c = wbf.field_b - wbf.field_a;
    }
    
    /* 6. Additional 128-bit arithmetic and comparisons */
    unsigned __int128 large_product = constants[5] * constants[6];
    unsigned __int128 large_sum = constants[7] + constants[0];
    
    /* Force comparisons of these results */
    int cmp_results[4] = {0};
    cmp_results[0] = (large_product > large_sum) ? 1 : 
                    ((large_product < large_sum) ? -1 : 0);
    cmp_results[1] = (constants[2] > constants[3]) ? 1 : 
                    ((constants[2] < constants[3]) ? -1 : 0);
    cmp_results[2] = (wbf.field_c > wbf.field_d) ? 1 : 
                    ((wbf.field_c < wbf.field_d) ? -1 : 0);
    cmp_results[3] = (loop_sum > constants[1]) ? 1 : 
                    ((loop_sum < constants[1]) ? -1 : 0);
    
    /* Final checksum to prevent optimization */
    unsigned __int128 final_checksum = 0;
    for (int i = 0; i < num_constants; i++) {
        final_checksum ^= constants[i];
    }
    final_checksum ^= loop_sum;
    final_checksum ^= wbf.field_a;
    final_checksum ^= wbf.field_b;
    final_checksum ^= wbf.field_c;
    final_checksum ^= wbf.field_d;
    final_checksum ^= large_product;
    final_checksum ^= large_sum;
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(final_checksum >> 64),
           (unsigned long long)(final_checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Comparison results: %d %d %d %d\n", 
           cmp_results[0], cmp_results[1], cmp_results[2], cmp_results[3]);
    printf("Array checksum: 0x%02x\n", checksum);
    
    return 0;
}
