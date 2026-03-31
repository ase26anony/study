/* wide-int-test.c - Test program for double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>

/* Large global array for offset calculations */
char huge_array[1ULL << 31] = {0};

/* Structure with wide bit-fields spanning > 64 bits */
struct wide_bitfield {
    unsigned __int128 a: 70;
    unsigned __int128 b: 58;
    unsigned __int128 c: 72;
    unsigned __int128 d: 56;
} wbf;

/* Function to prevent dead code elimination */
static unsigned __int128 checksum = 0;

/* Simple bubble sort for unsigned __int128 array */
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

/* Function to access array with 128-bit offsets */
void access_with_large_offsets(unsigned __int128 offsets[], int n) {
    for (int i = 0; i < n; i++) {
        /* Calculate offset within bounds */
        size_t offset = (size_t)(offsets[i] % (sizeof(huge_array) - 1));
        huge_array[offset] ^= (char)(i & 0xFF);
        checksum += (unsigned __int128)huge_array[offset];
    }
}

int main(void) {
    /* Initialize with constants exceeding 64 bits */
    unsigned __int128 values[10];
    
    /* Large constants that require 128-bit representation */
    values[0] = (unsigned __int128)0x123456789ABCDEF0ULL * 0x100000001ULL;
    values[1] = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    values[2] = values[0] + values[1];
    values[3] = values[0] * 3;
    values[4] = values[1] >> 32;
    values[5] = values[2] - values[0];
    values[6] = values[3] << 2;
    values[7] = ~values[0];
    values[8] = values[4] | values[5];
    values[9] = values[6] & values[7];
    
    /* Sort array - triggers many comparisons */
    sort_128bit_array(values, 10);
    
    /* Access array with 128-bit derived offsets */
    access_with_large_offsets(values, 10);
    
    /* Loop with 128-bit counter */
    unsigned __int128 start = values[0];
    unsigned __int128 end = values[0] + 1000;
    unsigned __int128 step = (end - start) / 10;
    
    for (unsigned __int128 i = start; i < end; i += step) {
        /* Switch with large 128-bit case values */
        switch ((uint64_t)(i & 0xFFFFFFFFFFFFFFFULL)) {
            case (uint64_t)(values[0] & 0xFFFFFFFFFFFFFFFULL):
                checksum += i * 2;
                break;
            case (uint64_t)(values[1] & 0xFFFFFFFFFFFFFFFULL):
                checksum += i * 3;
                break;
            case (uint64_t)(values[2] & 0xFFFFFFFFFFFFFFFULL):
                checksum += i * 5;
                break;
            case (uint64_t)(values[3] & 0xFFFFFFFFFFFFFFFULL):
                checksum += i * 7;
                break;
            default:
                checksum += i;
                break;
        }
        
        /* Manipulate wide bit-field structure */
        wbf.a = (unsigned __int128)i & ((1ULL << 70) - 1);
        wbf.b = (unsigned __int128)(i >> 70) & ((1ULL << 58) - 1);
        wbf.c = (unsigned __int128)(i >> 128) & ((1ULL << 72) - 1);
        wbf.d = (unsigned __int128)(i >> 200) & ((1ULL << 56) - 1);
        
        checksum += wbf.a + wbf.b + wbf.c + wbf.d;
    }
    
    /* Additional arithmetic that produces 128-bit results */
    unsigned __int128 product = values[0] * values[1];
    unsigned __int128 sum = values[0] + values[1];
    unsigned __int128 diff = values[0] - values[1];
    
    /* More comparisons */
    if (product > sum) checksum += 1;
    if (diff < sum) checksum += 2;
    if (values[0] == values[1]) checksum += 3;
    if (values[0] != values[1]) checksum += 4;
    
    /* Array indexing with 128-bit calculations */
    for (int i = 0; i < 10; i++) {
        unsigned __int128 offset = values[i] * i;
        size_t safe_offset = (size_t)(offset % (sizeof(huge_array) - 1));
        huge_array[safe_offset] += (char)(checksum & 0xFF);
        checksum += (unsigned __int128)huge_array[safe_offset];
    }
    
    /* Output checksum to prevent optimization */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
