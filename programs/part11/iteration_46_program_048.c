/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bf = {0};
    
    /* Variables to force register usage */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field1 = (a & 0xF) + (b & 0x7);          /* 4-bit field */
    bf.field2 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit field */
    bf.field3 = (b >> 8) | (c >> 4);            /* 12-bit field */
    bf.field4 = __builtin_parity(a) + __builtin_popcount(b & 0xFF); /* 3-bit field */
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32 = 0x12345678;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0xABCDEF01;
    register int64_t r64 = 0x1122334455667788ULL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    v8 = (int8_t)(r32 + 0x100);          /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)((r32 >> 8) & 0xFFFF); /* 32-bit to 16-bit with masking */
    
    /* Implicit narrowing through arithmetic */
    int8_t c1 = 100, c2 = 50;
    volatile int8_t result = c1 + c2;    /* May generate SUBREG for overflow truncation */
    
    /* Mixed-size operations */
    v16 = (int16_t)(v32 & 0xFFFF);       /* 32-bit mem to 16-bit mem through register */
    
    /* Read back */
    sink = v8 + v16 + result;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        register int val = i * 0x1111;  /* Force register source */
        
        /* Various complex address computations */
        arr[i * 3 + 7][i & 7] = val;                     /* 2D with arithmetic */
        *(ptr + i * 8 + (i % 3)) = val >> 8;             /* Pointer arithmetic */
        arr[0][(i * 13) % 8] = (int16_t)val;             /* Narrowing store with complex index */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s = {0};
    
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        register int x = i * 0x2222;
        data_ptr[i * 2] = x;                            /* Pointer to struct member */
        s.data[i * 2 + 1] = (int16_t)(x >> 4);          /* Struct member with narrowing */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sink = sum + s.header + s.footer;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int status : 4;
        int16_t values[16];
        int32_t data[8];
    } combined = {0};
    
    /* Register sources */
    register int32_t r1 = 0x89ABCDEF;
    register int32_t r2 = 0x76543210;
    
    /* Combined assignment: bitfield + complex array access */
    combined.flags = (r1 & 0xFF) | ((r2 >> 8) & 0xFF);  /* ZERO_EXTRACT potential */
    
    /* Complex addressing with narrowing */
    for (int i = 0; i < 8; i++) {
        register int idx = (i * 5 + 3) & 0xF;           /* Non-linear index */
        combined.values[idx] = (int16_t)(r1 + i * 0x100); /* SUBREG + complex MEM */
        combined.data[i] = r2 - i * 0x1000;             /* Complex addressing */
    }
    
    /* Additional bitfield with computation */
    combined.status = __builtin_popcount(r1 & 0xFF) + 
                     __builtin_parity(r2 & 0xFF);
    
    /* Inline assembly for direct RTL influence */
    int temp_array[16] = {0};
    for (int i = 0; i < 8; i++) {
        register int idx = i * 2 + 1;
        /* asm with memory output and complex addressing */
        asm volatile (
            "# Force memory store with complex address"
            : "=m" (temp_array[idx])  /* Complex addressing through idx */
            : 
            : "memory"
        );
        temp_array[idx] = i * 0x100;
    }
    
    /* Compute checksum */
    int sum = combined.flags + combined.status;
    for (int i = 0; i < 16; i++) sum += combined.values[i];
    for (int i = 0; i < 8; i++) sum += combined.data[i];
    for (int i = 0; i < 16; i++) sum += temp_array[i];
    
    sink = sum;
}

/* Test 5: Additional patterns for specific architectures */
void test_architecture_specific(void) {
    /* Operations that may generate STRICT_LOW_PART on some architectures */
    volatile uint32_t v32;
    volatile uint16_t v16;
    volatile uint8_t v8;
    
    register uint32_t r = 0xDEADBEEF;
    
    /* Multiple sub-word stores that might combine */
    v8 = (uint8_t)r;
    v16 = (uint16_t)(r >> 8);
    v32 = r;  /* Full word store for contrast */
    
    /* Bitfield extract and store */
    struct {
        volatile uint32_t low : 16;
        volatile uint32_t high : 16;
    } split = {0};
    
    split.low = r & 0xFFFF;
    split.high = (r >> 16) & 0xFFFF;
    
    /* Array with stride access */
    int matrix[4][16] = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            register int val = i * 16 + j;
            matrix[i][j * 3 % 16] = (int16_t)val;  /* Complex index with narrowing */
        }
    }
    
    sink = v8 + v16 + v32 + split.low + split.high + matrix[0][0];
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total_checksum += sink;
    
    test_subreg_operations();
    total_checksum += sink;
    
    test_complex_addressing();
    total_checksum += sink;
    
    test_combined_patterns();
    total_checksum += sink;
    
    test_architecture_specific();
    total_checksum += sink;
    
    /* Final output to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
