/* wide-int-compare.c - Targets GCC's double_int::cmp function */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values */
void sort_128bit_array(unsigned __int128 arr[], int n) {
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

/* Process switch with large 128-bit case values */
void process_with_switch(unsigned __int128 value) {
    /* Switch with large 128-bit case labels */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            checksum += value * 3;
            break;
        case ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL:
            checksum += value * 5;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL:
            checksum += value * 7;
            break;
        case ((unsigned __int128)0x1000000000000000ULL << 64) | 0x0000000000000001ULL:
            checksum += value * 11;
            break;
        default:
            checksum += value;
            break;
    }
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL,
        ((unsigned __int128)0x1000000000000000ULL << 64) | 0x0000000000000001ULL,
        ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL,
        ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        0x123456789ABCDEF0ULL * (unsigned __int128)0x100000001ULL,
        ((unsigned __int128)1 << 127) - 1,
        ((unsigned __int128)1 << 127),
        ((unsigned __int128)1 << 126) + ((unsigned __int128)1 << 64)
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* Perform arithmetic operations that require 128-bit precision */
    unsigned __int128 results[num_constants * 2];
    for (int i = 0; i < num_constants; i++) {
        /* Operations that generate 128-bit results */
        results[i*2] = constants[i] + ((unsigned __int128)i << 60);
        results[i*2 + 1] = constants[i] * 3 - ((unsigned __int128)1 << 62);
    }
    
    /* Sort the results - triggers many 128-bit comparisons */
    sort_128bit_array(results, num_constants * 2);
    
    /* Array indexing with large offsets derived from 128-bit values */
    for (int i = 0; i < num_constants * 2; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce modulo array size */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] ^= (char)(results[i] & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = ((unsigned __int128)0x1000000000000000ULL << 64);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* This comparison in loop condition triggers double_int::cmp */
        
        /* Process with switch statement */
        process_with_switch(i);
        
        /* Access struct with wide bit-fields */
        struct WideBitfield wbf;
        wbf.a = (i >> 58) & ((1ULL << 70) - 1);
        wbf.b = (i >> 0) & ((1ULL << 58) - 1);
        wbf.c = (i * 3) & (((unsigned __int128)1 << 120) - 1);
        wbf.d = (i * 5) & 0xFF;
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons in conditional statements */
    unsigned __int128 x = ((unsigned __int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 y = ((unsigned __int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
    
    if (x < y) checksum += 1;
    if (x > y) checksum += 2;
    if (x == y) checksum += 4;
    
    /* Complex expression with multiple comparisons */
    unsigned __int128 z = x + y;
    if (z > x && z > y) checksum += 8;
    if ((z / 2) < x || (z / 2) < y) checksum += 16;
    
    /* Output checksum to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
