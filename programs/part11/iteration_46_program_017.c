/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int f1 : 4;
        unsigned int f2 : 8;
        unsigned int f3 : 12;
        unsigned int f4 : 8;
    } bf = {0};
    
    /* Variables to create complex expressions */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with expressions */
    bf.f1 = (a & 0xF) + (b & 0x7);          /* Should generate ZERO_EXTRACT */
    bf.f2 = (b >> 4) & 0xFF;                /* Bit extraction pattern */
    bf.f3 = ((a & 0xFFF) ^ (c & 0xFFF)) | 1; /* Complex bitfield expression */
    bf.f4 = __builtin_popcount(a & 0xFF);   /* Builtin on sub-word data */
    
    /* Read back to prevent elimination */
    volatile unsigned int readback = bf.f1 + bf.f2 + bf.f3 + bf.f4;
    (void)readback;
}

/* Test 2: SUBREG generation through type narrowing */
void test_subreg_ops(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register variables to encourage register operations */
    register int32_t r32_1 = 0x12345678;
    register int32_t r32_2 = 0x9ABCDEF0;
    register int64_t r64 = 0x1122334455667788ULL;
    
    /* Narrowing assignments that should create SUBREG in SET_DEST */
    v8 = (int8_t)(r32_1 + r32_2);           /* int32 -> int8 with SUBREG */
    v16 = (int16_t)(r32_1 * 2);             /* int32 -> int16 */
    v32 = (int32_t)(r64 >> 16);             /* int64 -> int32 */
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100, c2 = 50;
    volatile char vc;
    vc = c1 + c2;                           /* char + char -> char with potential overflow */
    
    /* Read back */
    volatile int32_t sum = v8 + v16 + v32 + vc;
    (void)sum;
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
void test_complex_mem_ops(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] __attribute__((aligned(64)));
    int *restrict ptr = (int*)arr;
    
    /* Complex address calculations */
    for (int i = 0; i < 16; i++) {
        /* Multiple index computations */
        int idx1 = (i * 3 + 7) & 63;
        int idx2 = (i * 5 + 11) & 7;
        
        /* Complex addressing: arr[idx1][idx2] */
        arr[idx1][idx2] = i * 100 + 42;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx1 * 8 + idx2 + 4) = i * 200 + 84;
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s __attribute__((aligned(64)));
    
    int *base = &s.header;
    for (int i = 0; i < 8; i++) {
        /* Access through pointer with offset */
        *(base + 1 + i * 2) = i * 300;      /* s.data[i*2] */
    }
    
    /* Compute checksum */
    volatile int checksum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += arr[i][j];
        }
    }
    checksum += s.header + s.footer;
    (void)checksum;
}

/* Test 4: Combined patterns */
void test_combined_ops(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 16;
        unsigned int count : 8;
        int16_t values[16];
        unsigned int status : 4;
    } combined = {0};
    
    /* Register source for narrowing */
    register int32_t src_reg = 0x89ABCDEF;
    
    /* Combined assignment: bitfield + narrowed store to array */
    combined.flags = (src_reg & 0xFFFF) | 0x1000;
    
    /* Complex index calculation */
    for (int i = 0; i < 8; i++) {
        int idx = (i * 7 + 3) & 15;
        
        /* Narrow int32 to int16 with complex addressing */
        combined.values[idx] = (int16_t)(src_reg + i * 1000);
        
        /* Update bitfield in same loop */
        combined.count = (combined.count + 1) & 0xFF;
    }
    
    /* Final bitfield assignment with builtin */
    combined.status = __builtin_parity(combined.flags) & 0xF;
    
    /* Compute verification sum */
    volatile int sum = combined.flags + combined.count + combined.status;
    for (int i = 0; i < 16; i++) {
        sum += combined.values[i];
    }
    (void)sum;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_asm_ops(void) {
    int array[32] __attribute__((aligned(64)));
    volatile int index = 7;
    
    /* Complex addressing in asm output */
    asm volatile (
        "# Force complex MEM address\n"
        : "=m" (array[index * 3 + 5])  /* Complex index calculation */
        :
        : "memory"
    );
    
    /* Bitfield through asm */
    volatile struct {
        unsigned int field : 8;
    } bf = {0};
    
    int value = 0xAA;
    asm volatile (
        "# Bitfield store\n"
        : "=m" (bf.field)
        : "r" (value)
        : "memory"
    );
    
    (void)array[0];
    (void)bf.field;
}

int main(void) {
    int total_checksum = 0;
    
    /* Execute all tests */
    test_bitfield_ops();
    total_checksum += 1;
    
    test_subreg_ops();
    total_checksum += 2;
    
    test_complex_mem_ops();
    total_checksum += 3;
    
    test_combined_ops();
    total_checksum += 4;
    
    test_asm_ops();
    total_checksum += 5;
    
    /* Print result to prevent optimization */
    printf("Test completed with checksum: %d\n", total_checksum);
    
    return total_checksum == 15 ? 0 : 1;
}
