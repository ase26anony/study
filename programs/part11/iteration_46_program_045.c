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
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bf = {0};
    
    /* Source variables in registers */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field1 = (a & 0xF) + (b & 0x1);          /* Should generate ZERO_EXTRACT */
    bf.field2 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* Complex bitfield store */
    bf.field3 = __builtin_popcount(a) + (b >> 8); /* Builtin + shift */
    bf.field4 = (__builtin_parity(b) << 2) | 0x1;
    
    /* Read back to prevent elimination */
    sink = bf.field1 + bf.field2 + bf.field3 + bf.field4;
}

/* Test 2: SUBREG patterns through type narrowing */
void test_subreg_patterns(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32;
    
    /* Register sources of different sizes */
    register int32_t r32 = 0x12345678;
    register int64_t r64 = 0x9ABCDEF012345678ULL;
    
    /* Explicit narrowing casts that may generate SUBREG */
    v8 = (int8_t)(r32 + 0x100);      /* 32-bit to 8-bit with overflow */
    v16 = (int16_t)(r32 * 2);        /* 32-bit to 16-bit with computation */
    v32 = (int32_t)(r64 >> 16);      /* 64-bit to 32-bit */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 50;
    volatile char vc;
    vc = c1 + c2;                    /* char + char -> int, then store to char */
    
    /* Combined operation */
    v16 = (int16_t)((r32 & 0xFFFF) + (vc * 2));
    
    sink = v8 + v16 + v32 + vc;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int arr[64][8] __attribute__((aligned(64)));
    int *restrict ptr = &arr[0][0];
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        /* Multiple index computations */
        int idx1 = i * 3 + 7;
        int idx2 = (i << 2) | 0x3;
        int idx3 = i * i / 2 + 1;
        
        /* Store with complex addressing patterns */
        arr[idx1 % 64][idx2 % 8] = i * 100;          /* 2D array access */
        *(ptr + idx1 + idx2) = i * 200;              /* Pointer arithmetic */
        arr[idx3 % 64][(i * 5) % 8] = i * 300;       /* Mixed computations */
    }
    
    /* Struct with array member accessed via pointer */
    struct {
        int data[32];
        short extra[16];
    } s, *sp = &s;
    
    for (int i = 0; i < 8; i++) {
        int complex_idx = (i * 7 + 3) & 0x1F;
        sp->data[complex_idx] = i * 400;            /* Pointer-to-member */
        sp->extra[i * 2] = (short)(i * 500);        /* Combined with narrowing */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sink = sum + s.data[0] + s.extra[0];
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
        unsigned int status : 4;
    } combined;
    
    /* Array with complex access */
    int base_array[128];
    register int r1 = 0x1234, r2 = 0x5678, r3 = 0x9ABC;
    
    /* Combined assignment 1: Bitfield with computation */
    combined.flags = ((r1 & 0xF) << 4) | (__builtin_popcount(r2) & 0xF);
    
    /* Combined assignment 2: Array with narrowing and complex index */
    int idx = (r1 * 3 + r2 / 2) & 0xF;
    combined.values[idx] = (short)((r1 + r2) & 0xFFFF);
    
    /* Combined assignment 3: Complex memory store with narrowing */
    int mem_idx = ((r3 >> 4) * 5 + 7) & 0x7F;
    base_array[mem_idx] = (int)((r1 * r2) & 0xFFFFFFFF);
    
    /* Combined assignment 4: Another bitfield */
    combined.status = __builtin_parity(r3) | ((r2 >> 8) & 0x7);
    
    /* Inline assembly to directly influence RTL generation */
    int dummy = 0;
    asm volatile (
        "# Force complex memory operand\n"
        : "=m" (base_array[(r1 & 0x3F) + 8])  /* Complex addressing */
        : 
        : "memory"
    );
    
    sink = combined.flags + combined.values[0] + base_array[0] + combined.status + dummy;
}

/* Test 5: Additional edge cases */
void test_edge_cases(void) {
    /* Volatile bitfield in union for potential STRICT_LOW_PART */
    union {
        volatile uint32_t full;
        volatile struct {
            uint32_t low16 : 16;
            uint32_t high16 : 16;
        } parts;
    } u;
    
    u.parts.low16 = 0xABCD;      /* May generate STRICT_LOW_PART */
    u.parts.high16 = 0x1234;
    
    /* Array of bitfields */
    struct {
        unsigned int bits : 3;
    } bitarray[8];
    
    for (int i = 0; i < 8; i++) {
        bitarray[i].bits = (i * 3) & 0x7;  /* Multiple bitfield stores */
    }
    
    /* Mixed-size accesses through pointers */
    uint16_t *short_ptr = (uint16_t*)&u.full;
    volatile uint16_t vshort;
    vshort = *short_ptr;         /* Load through different type */
    
    sink = u.full + bitarray[0].bits + vshort;
}

int main(void) {
    int total = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subreg_patterns();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_edge_cases();
    total += sink;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
