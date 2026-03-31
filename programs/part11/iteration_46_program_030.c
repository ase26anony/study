/* Test program to trigger uncovered lines in resource.cc */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations to generate ZERO_EXTRACT */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct to force ZERO_EXTRACT in SET_DEST */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
    } bf = {0};
    
    /* Variables to create complex expressions */
    unsigned int a = 0x12345678;
    unsigned int b = 0x9ABCDEF0;
    unsigned int c = 0x13579BDF;
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    bf.field1 = (a & 0xF) + (b & 0xF);          /* 4-bit field */
    bf.field2 = ((a >> 8) & 0xFF) ^ ((b >> 8) & 0xFF); /* 8-bit field */
    bf.field3 = ((a + b) >> 4) & 0xFFF;         /* 12-bit field */
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word types */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register variables to force register operations */
    register int32_t r32_1 = 0x12345678;
    register int32_t r32_2 = 0x9ABCDEF0;
    register int32_t r32_3;
    
    /* Complex expressions that get computed in registers */
    r32_3 = r32_1 * 3 + r32_2 / 5;
    
    /* Narrowing assignments that should generate SUBREG in SET_DEST */
    v16 = (int16_t)r32_1;               /* 32-bit to 16-bit */
    v8 = (int8_t)(r32_2 & 0xFF);        /* 32-bit to 8-bit */
    v32 = r32_3;                        /* Keep as reference */
    
    /* Force use of builtins that might generate interesting patterns */
    sink = __builtin_popcount(v16) + __builtin_parity(v8);
}

/* Test 3: Complex memory addressing to trigger MEM_P path */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to prevent aliasing assumptions */
    int32_t arr1[256] __attribute__((aligned(16)));
    int16_t arr2[512] __attribute__((aligned(8)));
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex addressing patterns */
    for (int i = 0; i < 100; i++) {
        /* Non-linear index calculations */
        int idx1 = (i * 7 + 13) % 256;
        int idx2 = (i * 11 + 17) % 512;
        
        /* Complex source expressions */
        register int32_t src1 = arr1[idx1] * 3 - 7;
        register int32_t src2 = arr1[(idx1 + 5) % 256] + 11;
        
        /* Stores with complex addressing - should generate MEM with non-trivial address */
        arr1[(idx1 * 3 + 7) % 256] = src1 + src2;
        
        /* Narrowing store with complex address */
        arr2[(idx2 * 5 + 9) % 512] = (int16_t)(src1 - src2);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i];
    }
    for (int i = 0; i < 512; i++) {
        sum += arr2[i];
    }
    sink = sum;
}

/* Test 4: Combined patterns in struct context */
void test_combined_patterns(void) {
    /* Struct with mixed types */
    struct mixed {
        volatile unsigned int flags : 16;
        volatile int16_t data[32];
        volatile int32_t counter;
    } m;
    
    /* Initialize */
    m.counter = 0;
    for (int i = 0; i < 32; i++) {
        m.data[i] = i * 2;
    }
    
    /* Combined operations */
    for (int i = 0; i < 16; i++) {
        /* Bitfield assignment (potential ZERO_EXTRACT) */
        m.flags = (m.flags << 1) | ((i & 1) ? 1 : 0);
        
        /* Complex pointer arithmetic for memory store */
        int16_t *ptr = &m.data[0];
        
        /* Store with pointer arithmetic (complex addressing) */
        register int32_t temp = m.counter * i + 7;
        ptr[(i * 3 + 5) % 32] = (int16_t)(temp & 0xFFFF); /* SUBREG + MEM */
        
        /* Update counter */
        m.counter += i;
    }
    
    sink = m.flags + m.counter;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_inline_asm(void) {
    int32_t array[64] __attribute__((aligned(16)));
    int16_t short_array[128] __attribute__((aligned(8)));
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 5;
    }
    
    /* Inline asm with complex memory addressing */
    for (int i = 0; i < 10; i++) {
        int idx = (i * 13 + 7) % 64;
        
        /* Force memory store with complex addressing */
        asm volatile (
            "# Force complex memory store\n"
            : "=m" (array[idx * 2 % 64])  /* Complex index calculation */
            : 
            : "memory"
        );
        
        /* Another asm with different pattern */
        asm volatile (
            "# Another memory pattern\n"
            : "=m" (short_array[(idx * 3 + 11) % 128])
            :
            : "memory"
        );
    }
    
    /* Compute sum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += array[i];
    }
    sink = sum;
}

/* Test 6: STRICT_LOW_PART generation through byte operations */
void test_strict_low_part(void) {
    /* Operations that might generate STRICT_LOW_PART */
    volatile uint32_t v32;
    volatile uint16_t v16;
    volatile uint8_t v8;
    
    register uint32_t r1 = 0x12345678;
    register uint32_t r2 = 0x9ABCDEF0;
    
    /* Byte extraction and assignment */
    v8 = (uint8_t)((r1 + r2) & 0xFF);
    
    /* Half-word operations with masking */
    v16 = (uint16_t)((r1 * r2) & 0xFFFF);
    
    /* Full word with byte extraction */
    v32 = (r1 & 0xFF00FF00) | ((r2 >> 8) & 0x00FF00FF);
    
    sink = v8 + v16 + v32;
}

int main(void) {
    int total = 0;
    
    /* Run all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subreg_operations();
    total += sink;
    
    test_complex_memory_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_inline_asm();
    total += sink;
    
    test_strict_low_part();
    total += sink;
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
