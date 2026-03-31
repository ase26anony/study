/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_ops(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Source values in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0x1);      /* 4-bit field with computation */
    bf.field8 = (b >> 4) & 0xFF;            /* 8-bit field with shift */
    bf.field12 = ((a & 0xFFF) ^ (c & 0xFFF)) | 0x1; /* 12-bit field with bitwise ops */
    
    /* Use builtins that operate on bitfields */
    bf.field4 = __builtin_parity(bf.field8) & 0xF;
    
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations for SUBREG */
void test_subreg_ops(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)(r32 >> 8);       /* 32-bit to 16-bit with shift */
    v32 = (int32_t)r64;              /* 64-bit to 32-bit truncation */
    
    /* Arithmetic with implicit narrowing */
    register int16_t r16a = 30000;
    register int16_t r16b = 10000;
    v16 = r16a + r16b;               /* Potential overflow, kept as 16-bit */
    
    /* Combined operation with narrowing */
    v8 = (int8_t)((r32 & 0xFF) + (r32 >> 8 & 0xFF));
    
    sink = v8 + v16 + v32;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] = {0};
    int * restrict ptr = &arr[0][0];  /* restrict helps keep address computation */
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        register int val = i * 0x1111;
        
        /* Various complex addressing modes */
        arr[i*3 + 1][i & 7] = val;                     /* 2D with arithmetic */
        *(ptr + i*8 + (i % 3)) = val >> 8;             /* Pointer arithmetic */
        arr[0][(i*5 + 7) & 7] = arr[0][i & 7] + val;   /* Load + store with complex index */
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s = {0};
    
    int * restrict dptr = s.data;
    for (int i = 0; i < 16; i++) {
        register int idx = (i * 13 + 7) & 31;  /* Non-linear index */
        dptr[idx] = i * 0x2222;                /* Struct array with complex index */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sum += s.header + s.footer;
    for (int i = 0; i < 32; i++) sum += s.data[i];
    
    sink = sum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 16;
        int16_t values[16];
        unsigned int status : 4;
    } combined = {0};
    
    /* Pointer to array within struct */
    int16_t * restrict vptr = combined.values;
    
    register int32_t r1 = 0x12345678;
    register int32_t r2 = 0x9ABCDEF0;
    
    /* Combined assignment: bitfield + array with complex addressing */
    combined.flags = (r1 & 0xFFFF) ^ (r2 & 0xFFFF);  /* ZERO_EXTRACT potential */
    
    for (int i = 0; i < 8; i++) {
        register int idx = (i * 7 + 3) & 15;  /* Complex index */
        vptr[idx] = (int16_t)(r1 + i * 0x100); /* SUBREG potential */
    }
    
    combined.status = (r2 >> 16) & 0xF;  /* Another bitfield */
    
    /* Inline assembly with complex memory output */
    int temp_array[8] = {0};
    for (int i = 0; i < 8; i++) {
        register int idx = (i * 3 + 1) & 7;
        /* Assembly with memory output constraint */
        asm volatile (
            "# Force memory store with complex address"
            : "=m" (temp_array[idx])  /* Complex addressing in constraint */
            : 
            : "memory"
        );
        temp_array[idx] = i * 0x1111;
    }
    
    /* Compute checksum */
    int sum = combined.flags + combined.status;
    for (int i = 0; i < 16; i++) sum += combined.values[i];
    for (int i = 0; i < 8; i++) sum += temp_array[i];
    
    sink = sum;
}

/* Test 5: Additional patterns for specific architectures */
void test_arch_specific(void) {
    /* Operations that may generate STRICT_LOW_PART on some architectures */
    volatile uint32_t v32;
    register uint64_t r64 = 0xFEDCBA9876543210ULL;
    
    /* Low-part extraction */
    v32 = (uint32_t)r64;  /* May use STRICT_LOW_PART on 64-bit targets */
    
    /* Byte-wise operations */
    volatile uint8_t bytes[4];
    register uint32_t word = 0xA5A5A5A5;
    
    for (int i = 0; i < 4; i++) {
        bytes[i] = (word >> (i * 8)) & 0xFF;  /* Byte extraction */
    }
    
    /* Bit manipulation builtins */
    volatile int popcnt_result;
    register uint16_t r16 = 0xBEEF;
    popcnt_result = __builtin_popcount(r16);  /* May involve bit extraction */
    
    sink = v32 + bytes[0] + bytes[1] + bytes[2] + bytes[3] + popcnt_result;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Execute all tests */
    test_bitfield_ops();
    total += sink;
    
    test_subreg_ops();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_arch_specific();
    total += sink;
    
    /* Final result to prevent optimization */
    printf("Checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
