/* wide-int-test.c - Test program for double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning > 64 bits */
struct WideBitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for unsigned __int128 array */
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
    switch (value) {
        /* Large case labels that require 128-bit comparison */
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL):
            checksum ^= value + 1;
            break;
        case ((unsigned __int128)0x9876543210ABCDEFULL << 64 | 0x0123456789ABCDEFULL):
            checksum ^= value + 2;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFEULL):
            checksum ^= value + 3;
            break;
        case ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64 | 0xBBBBBBBBBBBBBBBBULL):
            checksum ^= value + 4;
            break;
        default:
            checksum ^= value;
            break;
    }
}

int main(void) {
    /* Initialize 128-bit constants that exceed 64-bit range */
    unsigned __int128 constants[] = {
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0xBBBBBBBBBBBBBBBBULL,
        ((unsigned __int128)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL,
        ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL,
    };
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Wide Integer Constant Folding */
    unsigned __int128 results[10];
    
    /* Perform arithmetic operations that require 128-bit precision */
    results[0] = constants[0] + constants[1];
    results[1] = constants[2] - constants[3];
    results[2] = constants[4] * 0x100000001ULL;  /* Multiplication with large constant */
    results[3] = constants[5] << 33;             /* Left shift beyond 64 bits */
    results[4] = (constants[0] + constants[1]) * (constants[2] - constants[3]);
    
    /* 2. Sort the results array - triggers many comparisons */
    sort_128bit_array(results, 5);
    
    /* 3. Array indexing with large offsets */
    for (int i = 0; i < 5; i++) {
        /* Calculate offset using 128-bit arithmetic, then modulo to stay in bounds */
        unsigned __int128 offset = results[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] ^= (char)(checksum & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* 4. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = constants[0];
    unsigned __int128 end = constants[0] + 100;
    unsigned __int128 step = 7;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* This loop condition triggers double_int::cmp */
        
        /* 5. Switch statement with large 128-bit cases */
        process_with_switch(i);
        
        /* 6. Structure with wide bit-field offsets */
        struct WideBitfield wbf;
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (i >> 200) & ((1ULL << 56) - 1);
        
        /* Manipulate bit-fields */
        wbf.a ^= wbf.b;
        wbf.c |= wbf.d;
        
        /* Add to checksum */
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons in conditional statements */
    if (constants[0] < constants[1]) {
        checksum += 1;
    }
    if (constants[2] > constants[3]) {
        checksum += 2;
    }
    if (constants[4] <= constants[5]) {
        checksum += 3;
    }
    if (constants[1] >= constants[0]) {
        checksum += 4;
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
