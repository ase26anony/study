/* wide-int-compare.c
 * Targets GCC's double_int::cmp function for coverage
 * Compile with: gcc -std=gnu11 -O2 -m32 -fdump-tree-optimized wide-int-compare.c -o wide-int-compare
 */

#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
static char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* 1. Wide Integer Constant Folding */
static unsigned __int128 large_constants[] = {
    (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL,
    (unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL,
    (unsigned __int128)1ULL << 80,
    (unsigned __int128)0xFFFFFFFFFFFFFFFFULL * 0xFFFFFFFFULL,
    (unsigned __int128)0xAAAAAAAAAAAAAAAALL << 32,
    (unsigned __int128)0x5555555555555555LL * 0x1000000000000000LL,
    ~(unsigned __int128)0,  /* Max 128-bit value */
    (unsigned __int128)0x8000000000000000ULL << 64,  /* 2^127 */
};

#define ARRAY_SIZE (sizeof(large_constants)/sizeof(large_constants[0]))

/* 2. Array sorting with 128-bit comparisons */
static void sort_128bit_array(unsigned __int128 arr[], int size) {
    /* Bubble sort to force many comparisons */
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* These comparisons should trigger double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* 3. Loop with 128-bit counter and switch */
static void loop_with_wide_counter(unsigned __int128 start, 
                                   unsigned __int128 end,
                                   unsigned __int128 step) {
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case constants */
        switch (i & 0x7) {  /* Use lower bits for manageable switch */
            case 0:
                checksum += i * 3;
                break;
            case 1:
                checksum += i / 2;
                break;
            case 2:
                checksum += i << 2;
                break;
            case 3:
                checksum += i | 0x12345;
                break;
            case 4:
                checksum += i & 0xFEDCBA9876543210ULL;
                break;
            case 5:
                checksum += i ^ 0xAAAAAAAAAAAAAAAALL;
                break;
            case 6:
                checksum += i % 1000;
                break;
            case 7:
                checksum += ~i;
                break;
        }
        
        /* 4. Array indexing with large offsets */
        size_t offset = (size_t)(i % (sizeof(huge_array) / 4));
        huge_array[offset] = (char)(i & 0xFF);
        checksum += huge_array[offset];
        
        /* 5. Bit-field structure manipulation */
        struct wide_bitfield wbf;
        wbf.a = (i >> 10) & ((1ULL << 70) - 1);
        wbf.b = (i >> 5) & ((1ULL << 58) - 1);
        wbf.c = i & ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL);
        wbf.d = (i >> 120) & 0xFF;
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
}

/* 6. Complex arithmetic producing 128-bit results */
static unsigned __int128 complex_128bit_math(unsigned __int128 a, 
                                            unsigned __int128 b) {
    unsigned __int128 result = 0;
    
    /* Various operations that require 128-bit precision */
    result = a + b;
    result = result * 0x123456789ABCDEFULL;
    result = result << 33;
    result = result - (a / 7);
    result = result | (b << 64);
    result = result & ~((unsigned __int128)0xFFFF << 48);
    
    return result;
}

int main(void) {
    /* Initialize array with computed values */
    unsigned __int128 computed[ARRAY_SIZE * 2];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        computed[i] = large_constants[i];
        computed[i + ARRAY_SIZE] = complex_128bit_math(
            large_constants[i], 
            large_constants[(i + 1) % ARRAY_SIZE]
        );
    }
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(computed, ARRAY_SIZE * 2);
    
    /* Run loop with wide counter */
    unsigned __int128 loop_start = (unsigned __int128)1ULL << 60;
    unsigned __int128 loop_end = loop_start + 1000;
    unsigned __int128 loop_step = 7;
    
    loop_with_wide_counter(loop_start, loop_end, loop_step);
    
    /* Additional comparisons in conditionals */
    unsigned __int128 max_val = 0;
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        if (computed[i] > max_val) {
            max_val = computed[i];
        }
        if (i > 0 && computed[i] < computed[i-1]) {
            checksum += 0xDEADBEEF;
        }
    }
    
    checksum += max_val;
    
    /* Boundary checks with wide integers */
    unsigned __int128 boundary = (unsigned __int128)1ULL << 127;
    if (max_val < boundary) {
        checksum += 1;
    }
    if (max_val > boundary / 2) {
        checksum += 2;
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
