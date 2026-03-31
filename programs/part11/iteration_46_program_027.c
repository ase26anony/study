/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
static void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int f1 : 4;
        unsigned int f2 : 8;
        unsigned int f3 : 12;
        unsigned int f4 : 3;
    } bf = {0};
    
    /* Variables for bit manipulation */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.f1 = (a & 0xF) + (b & 0x7);          /* 4-bit field */
    bf.f2 = (a >> 4) & 0xFF;                /* 8-bit field */
    bf.f3 = ((b << 4) | (c & 0xF)) & 0xFFF; /* 12-bit field */
    bf.f4 = __builtin_parity(a) & 0x7;      /* Using bit builtin */
    
    /* Read back to prevent elimination */
    volatile unsigned int read_back = bf.f1 + bf.f2 + bf.f3 + bf.f4;
    (void)read_back;
}

/* Test 2: SUBREG generation through type narrowing */
static void test_subreg_ops(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register variables to encourage register operations */
    register int32_t r32_1 = 0x12345678;
    register int32_t r32_2 = 0x9ABCDEF0;
    register int64_t r64 = 0x1122334455667788ULL;
    
    /* Narrowing assignments that may generate SUBREG in SET_DEST */
    v8 = (int8_t)(r32_1 + r32_2);           /* 32-bit -> 8-bit */
    v16 = (int16_t)(r32_1 * 2);             /* 32-bit -> 16-bit */
    v32 = (int32_t)r64;                     /* 64-bit -> 32-bit */
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100, c2 = 50;
    volatile char vc = c1 + c2;             /* char + char -> char (overflow) */
    
    /* Read back */
    volatile int32_t sum = v8 + v16 + v32 + vc;
    (void)sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
static void test_complex_mem_ops(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] __attribute__((aligned(64)));
    
    /* Struct with array member */
    struct data {
        int header;
        int values[16];
        int footer;
    } d __attribute__((aligned(32)));
    
    /* Pointer with restrict to avoid aliasing assumptions */
    int* restrict ptr = &arr[0][0];
    
    /* Complex address computations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index calculation */
        int idx = (i * 3 + 7) & 0x3F;
        
        /* Store with complex addressing - may generate MEM with complex XEXP */
        arr[idx][i & 0x7] = i * 100;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx * 8 + (i & 0x7)) = i * 200;
        
        /* Struct member access through computed index */
        d.values[(i * 5) % 16] = i * 300;
    }
    
    /* Compute checksum to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += arr[i][0] + d.values[i];
    }
    (void)checksum;
}

/* Test 4: Combined patterns */
static void test_combined_ops(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags : 8;
        unsigned int status : 4;
        int16_t data[32];
        int32_t large_data[16];
    } comb __attribute__((aligned(64)));
    
    /* Register variables for source data */
    register int32_t src32 = 0x89ABCDEF;
    register int64_t src64 = 0x0123456789ABCDEFULL;
    
    /* Combined assignment 1: Bitfield + complex source */
    comb.flags = (__builtin_popcount(src32) & 0xFF);
    comb.status = (src32 >> 16) & 0xF;
    
    /* Combined assignment 2: Array with complex index + narrowing */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 7 + 3) & 0x1F;
        
        /* Narrowing store to short array from register int */
        comb.data[idx] = (int16_t)(src32 + i * 1000);
        
        /* Store to int array with complex addressing */
        comb.large_data[(i * 3) % 16] = (int32_t)(src64 >> (i * 4));
    }
    
    /* Inline assembly to directly influence RTL generation */
    int temp_array[32] __attribute__((aligned(32)));
    
    /* Assembly with memory output constraint and complex addressing */
    asm volatile (
        "# Force complex MEM pattern\n"
        : "=m" (temp_array[16 + 8])  /* Complex address: array[24] */
        :
        : "memory"
    );
    
    /* Assembly with bitfield-like constraint (less portable) */
    unsigned int packed;
    asm volatile (
        "# Pack data into bitfields\n"
        : "=r" (packed)
        : "0" (0x12345678)
    );
    
    /* Read back for checksum */
    volatile int sum = comb.flags + comb.status;
    for (int i = 0; i < 8; i++) {
        sum += comb.data[i] + comb.large_data[i];
    }
    (void)sum;
    (void)packed;
}

/* Test 5: Additional patterns for STRICT_LOW_PART */
static void test_strict_low_part(void) {
    /* Operations that might generate STRICT_LOW_PART for partial word updates */
    volatile uint32_t word = 0xFFFFFFFF;
    
    /* Byte store to 32-bit word - may use STRICT_LOW_PART for byte mask */
    uint8_t *byte_ptr = (uint8_t*)&word;
    byte_ptr[1] = 0xAA;  /* Modify only byte 1 */
    
    /* 16-bit store with masking */
    uint16_t *half_ptr = (uint16_t*)&word;
    half_ptr[0] = 0x1234;  /* Modify lower 16 bits */
    
    /* Bitfield struct with volatile members */
    struct {
        volatile uint32_t low : 16;
        volatile uint32_t high : 16;
    } split = {0x1111, 0x2222};
    
    /* Partial updates that might use STRICT_LOW_PART */
    split.low = 0x3333;
    split.high = 0x4444;
    
    /* Read back */
    volatile uint32_t result = word + split.low + split.high;
    (void)result;
}

int main(void) {
    int final_checksum = 0;
    
    /* Execute all tests */
    test_bitfield_ops();
    final_checksum += 1;
    
    test_subreg_ops();
    final_checksum += 2;
    
    test_complex_mem_ops();
    final_checksum += 3;
    
    test_combined_ops();
    final_checksum += 4;
    
    test_strict_low_part();
    final_checksum += 5;
    
    /* Print checksum to ensure all code runs */
    printf("Final checksum: %d\n", final_checksum);
    
    return final_checksum != 15;  /* Should be 1+2+3+4+5 = 15 */
}
