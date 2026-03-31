/* double_int_coverage.c - Targets GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to generate checksum to prevent dead code elimination */
static unsigned long long compute_checksum(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned long long sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum = (sum * 31) + bytes[i];
    }
    return sum;
}

/* Simple bubble sort for unsigned __int128 values */
static void sort_128bit_array(unsigned __int128 arr[], int n) {
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
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL,
        (unsigned __int128)1ULL << 80,
        (unsigned __int128)0x8000000000000000ULL << 32,
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL * 0xFFFFFFFFULL,
        (unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64,
        (unsigned __int128)0x5555555555555555ULL << 65,
        (unsigned __int128)(-1)  /* Max 128-bit value */
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    for (int i = 0; i < num_constants; i++) {
        /* Operations that generate new 128-bit values */
        results[i * 2] = constants[i] + ((unsigned __int128)i << 60);
        results[i * 2 + 1] = constants[i] * ((unsigned __int128)0x10001ULL);
    }
    
    /* Sort the results - each comparison triggers double_int::cmp */
    sort_128bit_array(results, num_constants * 2);
    
    /* Array indexing with large offsets using 128-bit calculations */
    unsigned long long offset_sum = 0;
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) - 1024);
        huge_array[(size_t)offset] ^= (char)i;
        offset_sum += (unsigned long long)offset;
    }
    
    /* Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 loop_checksum = 0;
    unsigned __int128 start = (unsigned __int128)0x1000000000000000ULL;
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 123;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch statement with large 128-bit case constants */
        switch ((unsigned long long)(i & 0xFF)) {
            case 0x00:
                loop_checksum += i * 2;
                break;
            case 0x10:
                loop_checksum += i / 3;
                break;
            case 0x20:
                loop_checksum += i << 2;
                break;
            case 0x30:
                loop_checksum += i >> 1;
                break;
            default:
                loop_checksum += i;
                break;
        }
        
        /* Structure with wide bit-fields */
        struct wide_bitfield wbf;
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (i >> 200) & ((1ULL << 56) - 1);
        
        /* Manipulate bit-fields */
        wbf.a ^= wbf.b;
        wbf.c |= wbf.d;
        loop_checksum += (unsigned __int128)wbf.a * wbf.c;
    }
    
    /* Additional comparisons in conditional expressions */
    unsigned __int128 max_val = 0;
    for (int i = 0; i < num_constants * 2; i++) {
        /* More comparisons triggering double_int::cmp */
        if (results[i] > max_val) {
            max_val = results[i];
        }
        
        /* Complex conditional with 128-bit comparisons */
        if ((results[i] > constants[0]) && (results[i] < constants[num_constants - 1])) {
            results[i] = (results[i] * 3) / 2;
        }
    }
    
    /* Compute final checksum to ensure no dead code elimination */
    unsigned long long final_checksum = 0;
    final_checksum += compute_checksum(results, sizeof(results));
    final_checksum += compute_checksum(&loop_checksum, sizeof(loop_checksum));
    final_checksum += offset_sum;
    final_checksum += (unsigned long long)max_val;
    final_checksum += (unsigned long long)(max_val >> 64);
    
    /* Use the result to prevent optimization */
    printf("Checksum: %llu\n", final_checksum % 1000000);
    
    return (int)(final_checksum % 256);
}
