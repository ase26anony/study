/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function to force ZERO_EXTRACT in various contexts */
unsigned int test_zero_extract(struct BitFieldStruct *bfs, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Pattern 1: Bit-field extraction from structure */
    for (int i = 0; i < n; i++) {
        /* Multiple bit-field accesses that should generate ZERO_EXTRACT */
        sum += bfs[i].field1;
        sum += bfs[i].field2 << 5;
        sum += bfs[i].field3 << 12;
        
        /* Comparison with bit-field */
        if (bfs[i].field4 == (unsigned int)(g_volatile_seed & 0x3FF)) {
            sum += 1000;
        }
    }
    
    /* Pattern 2: Explicit bit-field extraction from integers */
    for (int i = 0; i < n; i++) {
        /* This should generate ZERO_EXTRACT: (arr[i] >> 8) & 0xFF */
        unsigned int extracted = (arr[i] & 0xFF00) >> 8;
        sum += extracted;
        
        /* Another pattern: extract multiple non-contiguous fields */
        unsigned int field_a = (arr[i] >> 3) & 0x7;    /* bits 3-5 */
        unsigned int field_b = (arr[i] >> 10) & 0x3F;  /* bits 10-15 */
        sum += field_a * field_b;
    }
    
    /* Pattern 3: Bit-field assignment */
    struct BitFieldStruct local_bf;
    for (int i = 0; i < n; i++) {
        /* Assignment to bit-field members */
        local_bf.field1 = (arr[i] >> 0) & 0x1F;
        local_bf.field2 = (arr[i] >> 5) & 0x7F;
        local_bf.field3 = (arr[i] >> 12) & 0x3FF;
        
        /* Use the result to prevent elimination */
        sum += local_bf.field1 + local_bf.field2 + local_bf.field3;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
short test_strict_low_part(short *s_arr, char *c_arr, int n) {
    short result = 0;
    
    /* Pattern 1: Partial register updates through pointers */
    for (int i = 0; i < n; i++) {
        /* Writing to short pointer - may generate STRICT_LOW_PART */
        volatile short *vs_ptr = &s_arr[i];
        *vs_ptr = (short)(g_volatile_seed + i);
        result += *vs_ptr;
    }
    
    /* Pattern 2: Char operations that get promoted */
    for (int i = 0; i < n; i++) {
        /* Char operations in integer context */
        char temp = c_arr[i];
        temp = (temp + 1) & 0x7F;  /* Keep in char range */
        
        /* Assignment back through volatile pointer */
        volatile char *vc_ptr = &c_arr[i];
        *vc_ptr = temp;
        result += temp;
    }
    
    /* Pattern 3: Mixed-size operations */
    for (int i = 0; i < n; i++) {
        int temp_int = s_arr[i];  /* Promote to int */
        temp_int = temp_int * 2 + 1;
        
        /* Store back only low 16 bits */
        s_arr[i] = (short)temp_int;
        result += s_arr[i];
    }
    
    /* Pattern 4: Inline assembly for byte register access (x86 specific) */
    for (int i = 0; i < n && i < 4; i++) {
        unsigned char byte_val = c_arr[i];
        unsigned char result_byte;
        
        /* Inline assembly that operates on byte registers */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "incb %%al\n\t"
            "movb %%al, %0"
            : "=r" (result_byte)
            : "r" (byte_val)
            : "%al"
        );
        
        c_arr[i] = result_byte;
        result += result_byte;
    }
    
    return result;
}

/* ========== SUBREG patterns ========== */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

int test_subreg(union TypePun *unions, int *int_arr, int n) {
    int sum = 0;
    
    /* Pattern 1: Union-based type punning */
    for (int i = 0; i < n; i++) {
        /* Access different views of the same data */
        unions[i].full = int_arr[i];
        
        /* SUBREG accesses through union members */
        sum += unions[i].halves[0];  /* Low 16 bits */
        sum += unions[i].halves[1];  /* High 16 bits */
        sum += unions[i].bytes[2];   /* Third byte */
        
        /* Modify through one view, read through another */
        unions[i].parts.low = (unions[i].parts.low + 1) & 0xFFFF;
        sum += unions[i].full;
    }
    
    /* Pattern 2: Casting between different integer sizes */
    for (int i = 0; i < n; i++) {
        /* Operations that involve size changes */
        int32_t val32 = int_arr[i];
        int16_t val16 = (int16_t)(val32 & 0xFFFF);
        int8_t val8 = (int8_t)(val32 & 0xFF);
        
        /* Mix operations of different sizes */
        sum += val32;
        sum += val16 * 2;
        sum += val8 * 3;
        
        /* Store back through different-sized pointer */
        *(volatile int16_t*)(&int_arr[i]) = val16;
    }
    
    /* Pattern 3: SIMD-like operations (manual unpacking) */
    for (int i = 0; i < n - 1; i += 2) {
        /* Treat two ints as four shorts */
        uint32_t packed = (uint32_t)int_arr[i] | ((uint32_t)int_arr[i+1] << 16);
        
        /* Extract individual 16-bit elements */
        uint16_t elem0 = packed & 0xFFFF;
        uint16_t elem1 = packed >> 16;
        
        /* Process and repack */
        elem0 = (elem0 + 1) & 0xFFFF;
        elem1 = (elem1 - 1) & 0xFFFF;
        
        packed = elem0 | (elem1 << 16);
        int_arr[i] = packed & 0xFFFF;
        int_arr[i+1] = packed >> 16;
        
        sum += elem0 + elem1;
    }
    
    return sum;
}

/* ========== Complex memory address patterns ========== */
int test_complex_memory(int *base_arr, int index, int offset) {
    int result = 0;
    
    /* Pattern with complex addressing that goes through SUBREG/ZERO_EXTRACT */
    for (int i = 0; i < 10; i++) {
        /* Array access with index calculation */
        volatile int *ptr = &base_arr[(index + i) & 0xF];
        
        /* Access through pointer with offset */
        int val = *(ptr + offset);
        
        /* Bit-field extraction from memory value */
        unsigned short low_half = (val >> 0) & 0xFFFF;
        unsigned short high_half = (val >> 16) & 0xFFFF;
        
        /* Modify and write back partial result */
        low_half = (low_half + 1) & 0xFFFF;
        *(volatile short*)ptr = low_half;  /* STRICT_LOW_PART store */
        
        result += val + low_half + high_half;
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    const int N = 100;
    int checksum = 0;
    
    /* Initialize test data */
    struct BitFieldStruct *bfs = malloc(N * sizeof(struct BitFieldStruct));
    unsigned int *int_arr = malloc(N * sizeof(unsigned int));
    short *short_arr = malloc(N * sizeof(short));
    char *char_arr = malloc(N * sizeof(char));
    union TypePun *unions = malloc(N * sizeof(union TypePun));
    int *base_arr = malloc(256 * sizeof(int));
    
    /* Seed with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        int val = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Initialize bit-field structure */
        bfs[i].field1 = val & 0x1F;
        bfs[i].field2 = (val >> 5) & 0x7F;
        bfs[i].field3 = (val >> 12) & 0x3FF;
        bfs[i].field4 = (val >> 22) & 0x3FF;
        
        /* Initialize integer array */
        int_arr[i] = val;
        
        /* Initialize short and char arrays */
        short_arr[i] = (short)(val & 0xFFFF);
        char_arr[i] = (char)(val & 0xFF);
        
        /* Initialize union */
        unions[i].full = val;
    }
    
    /* Initialize base array for complex memory tests */
    for (int i = 0; i < 256; i++) {
        base_arr[i] = (i * 987654321 + 13579) & 0xFFFFFFFF;
    }
    
    /* Run all tests to trigger different RTL patterns */
    checksum += test_zero_extract(bfs, int_arr, N);
    checksum += test_strict_low_part(short_arr, char_arr, N);
    checksum += test_subreg(unions, int_arr, N);
    checksum += test_complex_memory(base_arr, g_volatile_seed, 3);
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(bfs);
    free(int_arr);
    free(short_arr);
    free(char_arr);
    free(unions);
    free(base_arr);
    
    return checksum != 0 ? 0 : 1;
}
