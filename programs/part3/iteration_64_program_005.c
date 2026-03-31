/* wide-int-test.c - Test program for double_int comparisons */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:80;
    unsigned __int128 d:74;
};

/* Function to compute simple checksum */
static unsigned long long checksum = 0;

/* Bubble sort for unsigned __int128 array */
void sort_128bit_array(unsigned __int128 *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison triggers double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                checksum ^= (unsigned long long)(temp >> 64);
            }
        }
    }
}

/* Function to access array with 128-bit offsets */
void access_with_large_offsets(unsigned __int128 *values, int count) {
    for (int i = 0; i < count; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = values[i] * 3 + 7;
        
        /* Modulo to stay within bounds */
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
        
        /* Access array - compiler will generate bounds checking */
        huge_array[safe_offset] ^= (char)(offset & 0xFF);
        checksum += huge_array[safe_offset];
    }
}

int main(void) {
    /* Initialize with 128-bit constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
        (unsigned __int128)0xFEDCBA9876543210ULL << 64 | 0x13579BDF2468ACE0ULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL,
        (unsigned __int128)0x8000000000000000ULL << 64,
        (unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFFULL,
        (unsigned __int128)0x5555555555555555ULL << 64 | 0xAAAAAAAAAAAAAAAALL,
        (unsigned __int128)0x10000000000000000ULL,  /* Exactly 2^64 */
        (unsigned __int128)0x0ULL,
        (unsigned __int128)0x1ULL << 127,  /* Maximum signed 128-bit */
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64  /* Minimum with high bits set */
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations producing 128-bit results */
    unsigned __int128 computed[20];
    int comp_idx = 0;
    
    for (int i = 0; i < num_constants; i++) {
        /* Various operations that require 128-bit arithmetic */
        computed[comp_idx++] = constants[i] + 1;
        computed[comp_idx++] = constants[i] * 3;
        computed[comp_idx++] = constants[i] << 2;
        computed[comp_idx++] = ~constants[i];
        
        /* Cross operations between different constants */
        for (int j = i + 1; j < num_constants && j < i + 3; j++) {
            computed[comp_idx++] = constants[i] + constants[j];
            computed[comp_idx++] = constants[i] - constants[j];
            computed[comp_idx++] = constants[i] ^ constants[j];
        }
    }
    
    const int num_computed = comp_idx;
    
    /* 1. Sort the computed values - triggers many comparisons */
    sort_128bit_array(computed, num_computed);
    
    /* 2. Access array with large offsets */
    access_with_large_offsets(computed, num_computed);
    
    /* 3. Loop with 128-bit counter */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 64) | 0x80000000ULL;
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* 4. Switch with large 128-bit case values */
        switch (i & 0xF) {  /* Use lower bits for manageable switch */
            case 0:
                wbf.a = i >> 10;
                break;
            case 1:
                wbf.b = i >> 5;
                break;
            case 2:
                wbf.c = i << 2;
                break;
            case 3:
                wbf.d = ~i;
                break;
            default:
                wbf.a ^= i;
                wbf.b ^= i >> 1;
                break;
        }
        
        /* Additional comparisons in loop control */
        if (i > start + 500) {
            wbf.c += i;
        }
        
        checksum += (unsigned long long)(wbf.a ^ wbf.b ^ wbf.c ^ wbf.d);
    }
    
    /* 5. More complex comparisons */
    unsigned __int128 max_val = 0;
    unsigned __int128 min_val = ~(unsigned __int128)0;
    
    for (int i = 0; i < num_computed; i++) {
        /* Find max and min - more comparisons */
        if (computed[i] > max_val) {
            max_val = computed[i];
        }
        if (computed[i] < min_val) {
            min_val = computed[i];
        }
        
        /* Compare against various thresholds */
        unsigned __int128 threshold1 = (unsigned __int128)0x8000000000000000ULL << 64;
        unsigned __int128 threshold2 = (unsigned __int128)0x4000000000000000ULL << 64;
        
        if (computed[i] > threshold1) {
            checksum += 1;
        } else if (computed[i] > threshold2) {
            checksum += 2;
        } else {
            checksum += 3;
        }
    }
    
    /* Use bit-field structure */
    wbf.a = max_val >> 58;
    wbf.b = min_val >> 70;
    wbf.c = max_val ^ min_val;
    wbf.d = max_val - min_val;
    
    checksum += (unsigned long long)(wbf.a + wbf.b + wbf.c + wbf.d);
    
    /* Final output to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    printf("Max: 0x%016llx%016llx\n", 
           (unsigned long long)(max_val >> 64), 
           (unsigned long long)max_val);
    printf("Min: 0x%016llx%016llx\n", 
           (unsigned long long)(min_val >> 64), 
           (unsigned long long)min_val);
    
    return 0;
}
