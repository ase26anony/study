/* wide-int-comparisons.c - Exercise 128-bit integer comparisons in GCC */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 a:70;
    unsigned __int128 b:58;
    unsigned __int128 c:80;
    unsigned __int128 d:50;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Generate large 128-bit constants */
#define LARGE_CONST1 (((unsigned __int128)0x123456789ABCDEF0ULL) << 64 | 0xFEDCBA9876543210ULL)
#define LARGE_CONST2 (((unsigned __int128)0x9876543210ABCDEFULL) << 64 | 0x0123456789ABCDEFULL)
#define LARGE_CONST3 (((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64 | 0x0000000000000000ULL)
#define LARGE_CONST4 (((unsigned __int128)0x8000000000000000ULL) << 64 | 0x0000000000000000ULL)
#define LARGE_CONST5 (((unsigned __int128)0x7FFFFFFFFFFFFFFFULL) << 64 | 0xFFFFFFFFFFFFFFFFULL)

/* Simple bubble sort for 128-bit integers */
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

/* Function using switch with large 128-bit cases */
static unsigned __int128 process_with_switch(unsigned __int128 value) {
    switch (value) {
        /* Large case labels requiring 128-bit comparisons */
        case LARGE_CONST1:
            return value >> 1;
        case LARGE_CONST2:
            return value << 1;
        case LARGE_CONST3:
            return ~value;
        case LARGE_CONST4:
            return value | 0x1;
        case LARGE_CONST5:
            return value & ~((unsigned __int128)0xFF);
        default:
            return value + 1;
    }
}

int main(void) {
    /* Initialize array with 128-bit values requiring comparisons */
    unsigned __int128 values[10];
    
    /* Generate values using arithmetic that requires 128-bit precision */
    values[0] = LARGE_CONST1;
    values[1] = LARGE_CONST2;
    values[2] = LARGE_CONST3;
    values[3] = LARGE_CONST4;
    values[4] = LARGE_CONST5;
    
    /* Create more values through arithmetic operations */
    values[5] = values[0] + values[1];
    values[6] = values[2] - values[3];
    values[7] = values[4] * 3;
    values[8] = values[0] << 2;
    values[9] = values[1] >> 1;
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(values, 10);
    
    /* Array indexing with large offsets using 128-bit calculations */
    for (int i = 0; i < 10; i++) {
        /* Calculate offset using 128-bit arithmetic, then reduce to array bounds */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) / 2);
        huge_array[(size_t)offset] = (char)(values[i] & 0xFF);
        checksum += values[i];
    }
    
    /* Loop with 128-bit counter and boundary comparisons */
    unsigned __int128 start = LARGE_CONST2;
    unsigned __int128 end = LARGE_CONST2 + 1000;
    unsigned __int128 step = 100;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* This loop condition triggers 128-bit comparison */
        
        /* Use switch statement inside loop */
        unsigned __int128 processed = process_with_switch(i);
        checksum += processed;
        
        /* Access struct with wide bit-fields */
        struct WideBitfield wbf;
        wbf.a = i & (((unsigned __int128)1 << 70) - 1);
        wbf.b = (i >> 70) & (((unsigned __int128)1 << 58) - 1);
        wbf.c = processed & (((unsigned __int128)1 << 80) - 1);
        wbf.d = (processed >> 80) & (((unsigned __int128)1 << 50) - 1);
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional comparisons in conditional logic */
    if (values[0] < values[1]) {
        checksum += 1;
    }
    if (values[2] > values[3]) {
        checksum += 2;
    }
    if (values[4] <= values[5]) {
        checksum += 3;
    }
    if (values[6] >= values[7]) {
        checksum += 4;
    }
    if (values[8] == values[9]) {
        checksum += 5;
    }
    if (values[0] != values[9]) {
        checksum += 6;
    }
    
    /* Output checksum to prevent optimization */
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
