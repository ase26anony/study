/* double_int_coverage.c - Targets GCC's double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning >64 bits */
struct WideBitfield {
    unsigned __int128 low_part : 70;
    unsigned __int128 high_part : 58;
    unsigned __int128 full : 128;
};

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Generate large 128-bit constants */
#define LARGE_CONST1 (((unsigned __int128)0x123456789ABCDEF0ULL) << 64 | 0xFEDCBA9876543210ULL)
#define LARGE_CONST2 (((unsigned __int128)0x9876543210FEDCBAULL) << 64 | 0x0123456789ABCDEFULL)
#define LARGE_CONST3 (((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64 | 0x0000000000000000ULL)
#define LARGE_CONST4 (((unsigned __int128)0x8000000000000000ULL) << 64 | 0x0000000000000000ULL)
#define LARGE_CONST5 (((unsigned __int128)0x7FFFFFFFFFFFFFFFULL) << 64 | 0xFFFFFFFFFFFFFFFFULL)

/* Simple bubble sort for 128-bit values - forces many comparisons */
void sort_128bit_array(unsigned __int128 arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            /* This comparison will use double_int::cmp internally */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Array access with 128-bit offset calculations */
char access_with_offset(unsigned __int128 offset) {
    /* Modulo to stay within bounds */
    size_t safe_offset = (size_t)(offset % (sizeof(huge_array) / sizeof(huge_array[0])));
    return huge_array[safe_offset];
}

int main(void) {
    /* Initialize array of 128-bit values requiring comparisons */
    unsigned __int128 values[10];
    
    /* Generate values using 128-bit arithmetic that exceeds 64-bit range */
    values[0] = LARGE_CONST1;
    values[1] = LARGE_CONST2;
    values[2] = LARGE_CONST3;
    values[3] = LARGE_CONST4;
    values[4] = LARGE_CONST5;
    
    /* Create more values through arithmetic operations */
    values[5] = values[0] + values[1];           /* Addition */
    values[6] = values[2] - values[3];           /* Subtraction */
    values[7] = values[4] << 3;                  /* Left shift */
    values[8] = values[0] * 0x100000001ULL;      /* Multiplication */
    values[9] = ~values[1];                      /* Bitwise NOT */
    
    /* Sort the array - this performs many 128-bit comparisons */
    sort_128bit_array(values, 10);
    
    /* Access array using 128-bit offsets */
    for (int i = 0; i < 10; i++) {
        char c = access_with_offset(values[i]);
        checksum += (unsigned __int128)c * values[i];
    }
    
    /* Loop with 128-bit counter - forces comparisons in loop control */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[9];
    unsigned __int128 step = (end - start) / 100;
    
    if (step == 0) step = 1;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i >> 64)) {  /* Use high 64 bits for switch */
            case 0x123456789ABCDEF0ULL:
                checksum += i * 2;
                break;
            case 0x9876543210FEDCBAULL:
                checksum += i * 3;
                break;
            case 0xFFFFFFFFFFFFFFFFULL:
                checksum += i * 4;
                break;
            case 0x8000000000000000ULL:
                checksum += i * 5;
                break;
            default:
                checksum += i;
                break;
        }
        
        /* Access bit-field structure */
        struct WideBitfield wbf;
        wbf.low_part = (i >> 32) & ((1ULL << 70) - 1);
        wbf.high_part = (i >> 102) & ((1ULL << 58) - 1);
        wbf.full = (wbf.high_part << 70) | wbf.low_part;
        
        checksum += wbf.full;
        
        /* Array indexing with 128-bit calculation */
        unsigned __int128 offset = i * 0x100000001ULL;
        char element = access_with_offset(offset);
        checksum += (unsigned __int128)element * offset;
        
        /* Early exit to avoid infinite loop */
        if (i > start + step * 10) break;
    }
    
    /* Additional comparisons in conditional statements */
    if (values[0] < values[1]) checksum += 1;
    if (values[1] > values[2]) checksum += 2;
    if (values[2] <= values[3]) checksum += 3;
    if (values[3] >= values[4]) checksum += 4;
    if (values[4] == values[5]) checksum += 5;
    if (values[5] != values[6]) checksum += 6;
    
    /* Output checksum to prevent optimization */
    printf("Checksum (high 64 bits): 0x%016llX\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits):  0x%016llX\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
