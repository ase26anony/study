/* double_int_coverage.c - Exercise GCC's 128-bit integer comparison logic */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31];  /* 2GB array */

/* Structure with wide bit-fields spanning >64 bits */
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
            /* This comparison triggers double_int::cmp */
            if (arr[j] > arr[j + 1]) {
                unsigned __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* Generate large 128-bit constants */
unsigned __int128 generate_large_constant(uint64_t seed) {
    /* Create constants > 2^64 */
    unsigned __int128 val = (unsigned __int128)0x123456789ABCDEF0ULL;
    val = val * 0x100000001ULL + seed;
    val = val << 32;
    val = val | 0xFEDCBA9876543210ULL;
    return val;
}

int main(void) {
    /* Initialize with large 128-bit constants */
    unsigned __int128 values[8];
    
    /* Generate values requiring 128-bit representation */
    for (int i = 0; i < 8; i++) {
        values[i] = generate_large_constant(i * 0x1000);
        
        /* Perform arithmetic that forces 128-bit precision */
        values[i] = values[i] + ((unsigned __int128)1 << 70);
        values[i] = values[i] * 3;
        values[i] = values[i] >> (i % 16);
    }
    
    /* Sort the array - triggers many 128-bit comparisons */
    sort_128bit_array(values, 8);
    
    /* Array indexing with large offsets */
    for (int i = 0; i < 8; i++) {
        /* Calculate offset using 128-bit arithmetic */
        unsigned __int128 offset = values[i] % (sizeof(huge_array) - 1024);
        
        /* Access array with calculated offset */
        huge_array[(size_t)offset] = (char)(i + 'A');
        
        /* Update checksum */
        checksum += (unsigned __int128)huge_array[(size_t)offset] * offset;
    }
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = generate_large_constant(0x100);
    unsigned __int128 end = start + 1000;
    unsigned __int128 step = 123;
    
    struct WideBitfield wbf = {0};
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i & 0x7)) {  /* Use lower bits for switch */
            case 0:
                wbf.a = i & (((unsigned __int128)1 << 70) - 1);
                break;
            case 1:
                wbf.b = i & (((unsigned __int128)1 << 58) - 1);
                break;
            case 2:
                wbf.c = i & (((unsigned __int128)1 << 120) - 1);
                break;
            case 3:
                wbf.d = i & 0xFF;
                break;
            default:
                /* Complex comparison in default case */
                if (i > (start + end) / 2) {
                    wbf.a = (wbf.a + 1) & (((unsigned __int128)1 << 70) - 1);
                }
                break;
        }
        
        /* More comparisons in loop body */
        if (i < start + 500) {
            checksum += wbf.a;
        } else if (i > end - 500) {
            checksum += wbf.b;
        } else {
            checksum += wbf.c + wbf.d;
        }
        
        /* Additional 128-bit comparison */
        unsigned __int128 threshold = generate_large_constant(0x200);
        if (i > threshold) {
            checksum += i - threshold;
        }
    }
    
    /* Final checksum calculation */
    for (int i = 0; i < 8; i++) {
        checksum += values[i];
    }
    
    checksum += (unsigned __int128)wbf.a << 0;
    checksum += (unsigned __int128)wbf.b << 10;
    checksum += (unsigned __int128)wbf.c << 20;
    checksum += (unsigned __int128)wbf.d << 30;
    
    /* Output to prevent optimization */
    printf("Checksum (lower 64 bits): %llu\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
