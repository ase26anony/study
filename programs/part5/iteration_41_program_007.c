/* test_resource_coverage.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical variables */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to force ZERO_EXTRACT in various contexts */
unsigned int test_zero_extract(unsigned int *arr, int size) {
    unsigned int sum = 0;
    struct BitFieldStruct bf;
    
    /* 1. Bit-field extraction from structure */
    for (int i = 0; i < size; i++) {
        bf.field1 = (arr[i] >> 0) & 0x1F;
        bf.field2 = (arr[i] >> 5) & 0x7F;
        bf.field3 = (arr[i] >> 12) & 0x7;
        bf.field4 = (arr[i] >> 15) & 0x1FFFF;
        
        /* Force use of bit-fields in computation */
        sum += bf.field1 + (bf.field2 << 5) + (bf.field3 << 12) + (bf.field4 << 15);
    }
    
    /* 2. Explicit bit-field extraction with mask and shift */
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        unsigned int low_bits = (arr[i] >> 3) & 0xF;      /* Extract bits 3-6 */
        unsigned int mid_bits = (arr[i] >> 8) & 0xFF;     /* Extract bits 8-15 */
        unsigned int high_bits = (arr[i] >> 16) & 0xFFFF; /* Extract bits 16-31 */
        
        sum += low_bits + (mid_bits << 8) + (high_bits << 16);
    }
    
    /* 3. Bit-field comparison (forces extraction for comparison) */
    for (int i = 0; i < size; i++) {
        bf.field1 = arr[i] & 0x1F;
        if (bf.field1 == 0x10) {  /* Comparison with constant */
            sum += 0x100;
        }
        if (bf.field2 > 0x20) {   /* Another comparison */
            sum += 0x200;
        }
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
short test_strict_low_part(short *sarr, char *carr, int size) {
    short total = 0;
    
    /* 1. Partial register updates through char/short variables */
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        char c = carr[i];
        short s = sarr[i];
        
        /* Mix operations to prevent optimization */
        c = (c + g_volatile_seed) & 0xFF;      /* Force 8-bit result */
        s = (s * 3 + i) & 0xFFFF;              /* Force 16-bit result */
        
        total += s + c;
    }
    
    /* 2. Volatile pointer writes (partial stores) */
    volatile short *vsptr = (volatile short *)sarr;
    volatile char *vcptr = (volatile char *)carr;
    
    for (int i = 0; i < size && i < 10; i++) {
        *vsptr++ = (short)(i * 100);      /* 16-bit store */
        *vcptr++ = (char)(i * 7);         /* 8-bit store */
    }
    
    /* 3. Inline assembly for byte register operations (x86 specific) */
    #if defined(__i386__) || defined(__x86_64__)
    for (int i = 0; i < size && i < 5; i++) {
        unsigned char byte_val;
        /* Force use of byte register */
        asm volatile (
            "movb %1, %0\n\t"
            "addb $1, %0"
            : "=q" (byte_val)    /* "q" constraint selects byte register */
            : "r" ((unsigned char)carr[i])
            : "cc"
        );
        total += byte_val;
    }
    #endif
    
    return total;
}

/* ========== SUBREG patterns ========== */
union TypePunningUnion {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

int test_subreg(union TypePunningUnion *unions, int size) {
    int result = 0;
    
    /* 1. Union-based type punning */
    for (int i = 0; i < size; i++) {
        /* Access different views of the same data - should generate SUBREG */
        unions[i].full = i * 0x01020304;
        result += unions[i].halves[0];          /* Low 16 bits */
        result += unions[i].halves[1];          /* High 16 bits */
        result += unions[i].bytes[2];           /* Third byte */
    }
    
    /* 2. Casting between different integer sizes */
    for (int i = 0; i < size; i++) {
        uint32_t val = unions[i].full;
        
        /* These casts should generate SUBREG operations */
        uint16_t low_half = (uint16_t)(val & 0xFFFF);
        uint16_t high_half = (uint16_t)((val >> 16) & 0xFFFF);
        uint8_t first_byte = (uint8_t)(val & 0xFF);
        
        result += low_half + high_half + first_byte;
    }
    
    /* 3. Packed structure access simulation */
    for (int i = 0; i < size; i++) {
        /* Simulate packed data in register */
        uint32_t packed = (unions[i].parts.low << 16) | unions[i].parts.high;
        
        /* Extract parts - should use SUBREG */
        uint16_t extracted_low = (uint16_t)(packed >> 16);
        uint16_t extracted_high = (uint16_t)(packed & 0xFFFF);
        
        result += extracted_low * 3 + extracted_high * 5;
    }
    
    return result;
}

/* ========== Complex memory references ========== */
int test_complex_memory(int *base_arr, short *offset_arr, int size) {
    int sum = 0;
    
    /* Create complex addressing modes that combine with the patterns above */
    for (int i = 0; i < size; i++) {
        /* Array indexing with offset - creates MEM with complex address */
        int idx = (offset_arr[i] & 0xFF) % size;  /* Use bit-field extract */
        
        /* Access memory with complex address */
        int *ptr = &base_arr[idx];
        
        /* Combine with type punning */
        union TypePunningUnion u;
        u.full = *ptr;
        
        /* Use bit-field extraction from memory value */
        unsigned int field = (u.full >> 8) & 0xFF;  /* ZERO_EXTRACT from memory */
        
        /* Partial write back to memory */
        u.bytes[1] = (u.bytes[1] + field) & 0xFF;  /* STRICT_LOW_PART style */
        
        /* Write back through pointer */
        *ptr = u.full;
        
        sum += *ptr;
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    int size = 100;
    unsigned int *arr;
    short *sarr;
    char *carr;
    union TypePunningUnion *unions;
    int *base_arr;
    short *offset_arr;
    
    /* Allocate and initialize test data */
    arr = (unsigned int *)malloc(size * sizeof(unsigned int));
    sarr = (short *)malloc(size * sizeof(short));
    carr = (char *)malloc(size * sizeof(char));
    unions = (union TypePunningUnion *)malloc(size * sizeof(union TypePunningUnion));
    base_arr = (int *)malloc(size * sizeof(int));
    offset_arr = (short *)malloc(size * sizeof(short));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        arr[i] = (i * 1103515245 + 12345) & 0xFFFFFFFF;
        sarr[i] = (short)((i * 32719) & 0xFFFF);
        carr[i] = (char)((i * 73) & 0xFF);
        unions[i].full = (i * 0x89ABCDEF) & 0xFFFFFFFF;
        base_arr[i] = i * 1000;
        offset_arr[i] = (short)(i * 3);
    }
    
    /* Run all tests to trigger different RTL patterns */
    unsigned int sum1 = test_zero_extract(arr, size);
    short sum2 = test_strict_low_part(sarr, carr, size);
    int sum3 = test_subreg(unions, size);
    int sum4 = test_complex_memory(base_arr, offset_arr, size);
    
    /* Combine results to prevent dead code elimination */
    int final_result = sum1 + sum2 + sum3 + sum4;
    
    /* Use result to affect control flow */
    if (final_result > 0) {
        printf("Test completed successfully. Result: %d\n", final_result);
    } else {
        printf("Test completed. Result: %d\n", final_result);
    }
    
    /* Cleanup */
    free(arr);
    free(sarr);
    free(carr);
    free(unions);
    free(base_arr);
    free(offset_arr);
    
    return (final_result > 0) ? 0 : 1;
}
