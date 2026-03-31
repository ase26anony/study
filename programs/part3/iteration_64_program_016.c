/* wide-int-test.c - Exercise 128-bit integer comparisons in GCC */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
} wbf;

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit values */
void sort_128bit(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            /* This comparison triggers double_int::cmp */
            if (arr[j] > arr[j+1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

/* Process switch with large 128-bit case values */
unsigned __int128 process_switch(unsigned __int128 val) {
    switch (val) {
        /* Large case values that require 128-bit comparison */
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            return val + 1;
        case ((unsigned __int128)0xABCDEF0123456789ULL << 64) | 0x9876543210FEDCBAULL:
            return val * 2;
        case ((unsigned __int128)0xDEADBEEFCAFEBABEULL << 64) | 0xBAADF00D12345678ULL:
            return val >> 3;
        case ((unsigned __int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL:
            return val << 2;
        default:
            return val ^ 0xFFFFFFFFFFFFFFFFULL;
    }
}

int main(void) {
    /* Initialize with large 128-bit constants */
    unsigned __int128 values[8];
    
    /* Generate 128-bit constants using multiplication that overflows 64-bit */
    values[0] = ((unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL);
    values[1] = ((unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL);
    values[2] = ((unsigned __int128)0xDEADBEEFCAFEBABEULL << 64) | 0xBAADF00D12345678ULL;
    values[3] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 32);
    values[4] = ((unsigned __int128)1 << 127) - 1;  /* Max signed 128-bit */
    values[5] = (unsigned __int128)-1;  /* Max unsigned 128-bit */
    values[6] = values[0] + values[1];
    values[7] = values[2] * 3;
    
    /* Sort array - triggers many 128-bit comparisons */
    sort_128bit(values, 8);
    
    /* Array indexing with large offsets derived from 128-bit values */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] * i;
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / 8));
        
        /* Access array using calculated offset */
        huge_array[safe_offset] = (char)(values[i] & 0xFF);
        checksum += (unsigned __int128)huge_array[safe_offset];
    }
    
    /* Loop with 128-bit counter and bounds */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 100;
    unsigned __int128 step = (values[1] - values[0]) / 8;
    
    if (step == 0) step = 1;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* This loop comparison triggers double_int::cmp */
        
        /* Process through switch with large case values */
        unsigned __int128 result = process_switch(i);
        checksum += result;
        
        /* Manipulate wide bit-field structure */
        wbf.a = (i >> 0) & ((1ULL << 70) - 1);
        wbf.b = (i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (i >> 200) & ((1ULL << 56) - 1);
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
        
        /* Break early to avoid excessive iterations */
        if (i > start + step * 10) break;
    }
    
    /* Additional arithmetic operations producing 128-bit results */
    unsigned __int128 prod = values[0] * values[1];
    unsigned __int128 sum = values[2] + values[3];
    unsigned __int128 diff = values[4] - values[5];
    unsigned __int128 shift = values[6] << 5;
    
    /* More comparisons */
    if (prod < sum) checksum += 1;
    if (diff > shift) checksum += 2;
    if (values[7] >= values[0]) checksum += 4;
    
    /* Final checksum output to prevent optimization */
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
