/* wide-int-test.c - Test program for double_int comparison coverage */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:70;
    unsigned __int128 d:58;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values to force many comparisons */
void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* Each comparison triggers double_int::cmp */
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
    /* Switch with large case values forces sorting/comparison during compilation */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL):
            checksum += 1;
            break;
        case ((unsigned __int128)0x9876543210ABCDEFULL << 64 | 0x0123456789ABCDEFULL):
            checksum += 2;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFEULL):
            checksum += 3;
            break;
        case ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64 | 0xBBBBBBBBBBBBBBBBULL):
            checksum += 4;
            break;
        case ((unsigned __int128)0x5555555555555555ULL << 64 | 0x6666666666666666ULL):
            checksum += 5;
            break;
        default:
            checksum += value & 0xFF;
            break;
    }
}

int main(void) {
    /* Initialize 128-bit constants exceeding 64-bit range */
    unsigned __int128 constants[] = {
        /* Values > 2^64 - 1 */
        ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((unsigned __int128)0x9876543210ABCDEFULL << 64) | 0x0123456789ABCDEFULL,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL,
        ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0xBBBBBBBBBBBBBBBBULL,
        ((unsigned __int128)0x5555555555555555ULL << 64) | 0x6666666666666666ULL,
        /* Generate via arithmetic operations */
        ((unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL) << 32,
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL + 1),  /* Exactly 2^64 */
        ((unsigned __int128)1 << 127) - 1,  /* Max signed 128-bit */
        ((unsigned __int128)1 << 127),      /* 2^127 */
        ((unsigned __int128)0xDEADBEEFULL << 80) | 0xCAFEBABE,
    };
    
    const int num_constants = sizeof(constants) / sizeof(constants[0]);
    
    /* 1. Sort array of 128-bit values - forces many comparisons */
    sort_128bit_array(constants, num_constants);
    
    /* 2. Array indexing with large offsets using 128-bit calculations */
    for (int i = 0; i < num_constants; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = constants[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] = (char)(checksum & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* 3. Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = ((unsigned __int128)0x100000000ULL << 32);  /* 2^64 */
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 100;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Each loop iteration compares i < end using 128-bit comparison */
        
        /* 4. Switch statement with large 128-bit cases */
        process_with_switch(i);
        
        /* 5. Structure with wide bit-field manipulation */
        struct WideBitfield wbf;
        wbf.a = i >> 58;
        wbf.b = i & (((unsigned __int128)1 << 58) - 1);
        wbf.c = (i * 3) >> 58;
        wbf.d = (i * 7) & (((unsigned __int128)1 << 58) - 1);
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
        
        /* Additional arithmetic producing 128-bit results */
        unsigned __int128 prod = i * ((unsigned __int128)0x123456789ABCDEF0ULL);
        unsigned __int128 shifted = prod << 3;
        unsigned __int128 summed = shifted + constants[i % num_constants];
        
        /* Force comparison of arithmetic results */
        if (summed > prod) {
            checksum += 1;
        }
        if (shifted < constants[0]) {
            checksum += 2;
        }
    }
    
    /* 6. Additional constant folding tests */
    unsigned __int128 fold_test = 
        ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL + 1) * 
        ((unsigned __int128)0x100000000ULL);
    
    unsigned __int128 shift_test = ((unsigned __int128)1 << 127) >> 1;
    
    /* Compare folded constants */
    if (fold_test > shift_test) {
        checksum += fold_test - shift_test;
    }
    
    /* Final output to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
