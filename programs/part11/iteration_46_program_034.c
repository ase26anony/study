/* test_resource_marking.c */
#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
static void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Variables to use in expressions */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Multiple assignments to bitfields with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0x7);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* Complex expression */
    bf.field12 = __builtin_popcount(a) + (b & 0xFFF); /* Builtin + masking */
    
    /* Use bitfield in computation to prevent elimination */
    unsigned int sum1 = bf.field4 + bf.field8 + bf.field12;
    printf("Bitfield sum: %u\n", sum1);
}

/* Test 2: SUBREG generation through type narrowing */
static void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int16_t vs16;
    volatile int8_t vs8;
    
    /* Register variables to encourage register operations */
    register int32_t r32_1 = 0x12345678;
    register int32_t r32_2 = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - should generate SUBREG in SET_DEST */
    vs16 = (int16_t)(r32_1 + r32_2);           /* Addition then truncation */
    vs8 = (int8_t)(r32_1 * 2);                 /* Multiplication then truncation */
    
    /* Implicit narrowing through arithmetic overflow */
    int16_t s16_1 = 30000;
    int16_t s16_2 = 10000;
    volatile int16_t vs16_2;
    vs16_2 = s16_1 + s16_2;  /* Overflow truncation */
    
    /* Use results to prevent elimination */
    int32_t sum2 = vs16 + vs8 + vs16_2;
    printf("Subreg sum: %d\n", sum2);
}

/* Test 3: Complex memory addressing for MEM_P(x) path */
static void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to avoid aliasing assumptions */
    int32_t arr1[256] __attribute__((aligned(16)));
    int16_t arr2[512] __attribute__((aligned(8)));
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex addressing patterns */
    register int idx;
    register int32_t val = 0;
    
    /* Multi-dimensional style addressing */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            idx = i * 16 + j * 2 + 7;  /* Non-linear index computation */
            val = (i * 100) + (j * 10);
            arr1[idx] = val;  /* Complex address in MEM */
        }
    }
    
    /* Pointer arithmetic with multiple offsets */
    int32_t *ptr1 = arr1;
    for (int i = 0; i < 100; i++) {
        *(ptr1 + i + (i & 0xF)) = i * 2;  /* Multiple offsets */
    }
    
    /* Struct pointer access */
    struct {
        int32_t data[64];
        int16_t more_data[128];
    } s;
    
    int32_t *sptr = s.data;
    for (int i = 0; i < 32; i++) {
        sptr[i * 2] = i * 100 + 50;  /* Strided access */
    }
    
    /* Compute checksum */
    int64_t sum3 = 0;
    for (int i = 0; i < 256; i++) {
        sum3 += arr1[i];
    }
    printf("Memory sum: %ld\n", sum3);
}

/* Test 4: Combined patterns */
static void test_combined_patterns(void) {
    /* Struct with mixed types */
    volatile struct {
        unsigned int flags : 8;
        int16_t values[32];
        int32_t data;
    } combined __attribute__((aligned(16)));
    
    /* Initialize */
    combined.flags = 0;
    combined.data = 0xDEADBEEF;
    
    /* Combined operation: bitfield store + array store with narrowing */
    register int32_t temp = 0x12345678;
    
    /* Bitfield assignment with expression */
    combined.flags = (temp & 0xFF) ^ ((temp >> 8) & 0xFF);
    
    /* Array store with complex index and narrowing */
    for (int i = 0; i < 16; i++) {
        int idx = (i * 3 + 7) & 0x1F;  /* Complex index */
        combined.values[idx] = (int16_t)(temp + i * 100);  /* Narrowing */
    }
    
    /* Inline assembly to directly influence RTL generation */
    int32_t asm_temp = 0xABCD1234;
    int16_t asm_dest;
    
    /* Assembly with memory output constraint - may generate SUBREG/MEM */
    asm volatile (
        "movw %w1, %0\n\t"  /* Narrow 32-bit to 16-bit */
        : "=r" (asm_dest)
        : "r" (asm_temp)
        : "memory"
    );
    
    /* Use results */
    int32_t sum4 = combined.flags + combined.values[0] + combined.values[31] + asm_dest;
    printf("Combined sum: %d\n", sum4);
}

/* Test 5: Additional patterns using builtins and volatile */
static void test_builtin_operations(void) {
    /* Operations that might generate ZERO_EXTRACT for bit manipulation */
    volatile uint16_t v16 = 0x1234;
    volatile uint8_t v8 = 0xAB;
    
    /* Builtins on sub-word data */
    uint32_t popcnt = __builtin_popcount(v16);  /* May extract bits */
    uint32_t parity = __builtin_parity(v8);     /* May extract bits */
    
    /* Bitfield extraction through shifting and masking */
    struct {
        volatile uint32_t low : 10;
        volatile uint32_t high : 10;
    } bits;
    
    uint32_t x = 0x3FF;  /* All bits set for 10-bit field */
    bits.low = x & __builtin_popcount(v16);  /* Complex expression */
    bits.high = (x >> 2) | parity;
    
    /* Use results */
    uint32_t sum5 = popcnt + parity + bits.low + bits.high;
    printf("Builtin sum: %u\n", sum5);
}

int main(void) {
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    test_subreg_operations();
    test_complex_memory_addressing();
    test_combined_patterns();
    test_builtin_operations();
    
    printf("All tests completed.\n");
    return 0;
}
