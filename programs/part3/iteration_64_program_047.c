/* wide-int-test.c - Test program to exercise GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning more than 64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:120;
    unsigned __int128 d:8;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for 128-bit integers */
void sort_128bit_array(unsigned __int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            /* This comparison should trigger double_int::cmp */
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
    /* Switch with large 128-bit case constants */
    switch (value) {
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            checksum += value * 3;
            break;
        case ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL:
            checksum += value * 5;
            break;
        case ((unsigned __int128)0xAAAAAAAAAAAAAAAALL << 64) | 0x5555555555555555ULL:
            checksum += value * 7;
            break;
        case ((unsigned __int128)0xDEADBEEFDEADBEEFULL << 64) | 0xCAFEBABECAFEBABEULL:
            checksum += value * 11;
            break;
        default:
            checksum += value;
            break;
    }
}

int main(void) {
    /* Initialize with constants exceeding 64-bit range */
    unsigned __int128 values[10];
    
    /* Generate 128-bit constants using multiplication that requires 128-bit precision */
    values[0] = ((unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL);
    values[1] = ((unsigned __int128)0xFEDCBA9876543210ULL * 0xFFFFFFFFULL);
    values[2] = ((unsigned __int128)0xDEADBEEFCAFEBABEULL << 64) | 0x123456789ABCDEF0ULL;
    values[3] = values[0] + values[1];
    values[4] = values[1] - values[2];
    values[5] = values[0] * 3;
    values[6] = values[1] >> 32;
    values[7] = values[2] << 16;
    values[8] = ~((unsigned __int128)0);
    values[9] = ((unsigned __int128)1 << 127) - 1;
    
    /* Sort the array - each comparison should trigger double_int::cmp */
    sort_128bit_array(values, 10);
    
    /* Access large array using 128-bit offsets */
    for (int i = 0; i < 10; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array size */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / sizeof(huge_array[0]));
        huge_array[(size_t)offset] = (char)(values[i] & 0xFF);
        checksum += huge_array[(size_t)offset];
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 100;
    unsigned __int128 step = 10;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* This comparison in loop condition should trigger double_int::cmp */
        process_with_switch(i);
        
        /* Manipulate struct with wide bit-fields */
        struct WideBitfield wbf;
        wbf.a = i & (((unsigned __int128)1 << 70) - 1);
        wbf.b = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        wbf.c = (i >> 128) | ((i & 0xFFFF) << 112);
        wbf.d = (i >> 240) & 0xFF;
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons in conditional statements */
    if (values[9] > values[8]) {
        checksum += 1000;
    }
    
    if (values[0] < values[1]) {
        checksum += 2000;
    }
    
    if (values[5] == values[6]) {
        checksum += 3000;
    }
    
    /* Complex expression with multiple comparisons */
    unsigned __int128 x = values[3];
    unsigned __int128 y = values[4];
    
    if ((x > y) && (x - y > 1000)) {
        checksum += x - y;
    }
    
    /* Array indexing with 128-bit calculations */
    unsigned __int128 base = values[2];
    unsigned __int128 stride = 17;
    
    for (int i = 0; i < 5; i++) {
        unsigned __int128 idx = base + (unsigned __int128)i * stride;
        if (idx < sizeof(huge_array)) {
            huge_array[(size_t)idx % sizeof(huge_array)] ^= (char)i;
            checksum += huge_array[(size_t)idx % sizeof(huge_array)];
        }
    }
    
    /* Final output to prevent optimization */
    printf("Checksum (lower 64 bits): %llu\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    printf("Checksum (upper 64 bits): %llu\n", (unsigned long long)(checksum >> 64));
    
    return 0;
}
