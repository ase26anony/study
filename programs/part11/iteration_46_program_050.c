#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bitfields = {0};
    
    /* Initialize source values */
    unsigned int a = 0xABCD;
    unsigned int b = 0x1234;
    unsigned int c = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bitfields.field1 = (a & 0xF) + (b & 0x7);          /* 4-bit field */
    bitfields.field2 = ((a >> 4) & 0xFF) ^ (c & 0xFF); /* 8-bit field */
    bitfields.field3 = (b >> 2) | (c >> 4);            /* 12-bit field */
    bitfields.field4 = __builtin_parity(a) + __builtin_popcount(b & 0xF); /* 3-bit field */
    
    /* Read back to prevent elimination */
    volatile unsigned int read1 = bitfields.field1;
    volatile unsigned int read2 = bitfields.field2;
    (void)read1;
    (void)read2;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register sources (hint to compiler) */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = 0x11223344;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* int -> short */
    vs2 = (short)(r1 + r2);             /* arithmetic then narrowing */
    vs3 = (short)(r1 * r2);             /* multiplication then narrowing */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100;
    char c2 = 50;
    vc1 = c1 + c2;                      /* char + char -> char (potential overflow) */
    vc2 = (c1 * 2) - c2;                /* more complex char arithmetic */
    
    /* Read back */
    volatile short read_s = vs1 + vs2 + vs3;
    volatile char read_c = vc1 + vc2;
    (void)read_s;
    (void)read_c;
}

/* Test 3: Complex memory addressing to generate MEM with non-trivial address */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array */
    int arr2d[16][16];
    
    /* Struct with array */
    struct {
        int data[32];
        int offset;
    } s = {0};
    
    /* Pointer with restrict to avoid aliasing assumptions */
    int *restrict ptr1 = arr2d[0];
    int *restrict ptr2 = s.data;
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index: i*3 + 7 */
        int idx1 = i * 3 + 7;
        
        /* Multi-dimensional access with stride */
        int idx2 = (i % 4) * 16 + (i / 4);
        
        /* Pointer arithmetic with multiple offsets */
        int *addr1 = ptr1 + idx1 + s.offset;
        int *addr2 = ptr2 + idx2 + i;
        
        /* Register source values */
        register int rval1 = i * 0x1111;
        register int rval2 = i * 0x2222 + 0x3333;
        
        /* Stores with complex addressing */
        *addr1 = rval1;                    /* Base + idx1 + offset */
        arr2d[i][idx1 % 16] = rval2;       /* 2D array access */
        ptr2[idx2] = rval1 ^ rval2;        /* Array with computed index */
        
        /* Struct member through pointer */
        struct { int data[32]; int offset; } *sptr = &s;
        sptr->data[idx1 % 32] = rval1 + rval2;
    }
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            sum += arr2d[i][j];
        }
    }
    for (int i = 0; i < 32; i++) {
        sum += s.data[i];
    }
    volatile int checksum = sum;
    (void)checksum;
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int status : 4;
        short values[16];
        int counters[8];
    } combined = {0};
    
    /* Initialize some values */
    register int rbase = 0x12345678;
    register short rshort = 0xABCD;
    
    /* Combined assignment 1: Bitfield with complex expression */
    combined.flags = (rbase & 0xFF) | ((rbase >> 8) & 0x7F);
    combined.status = __builtin_popcount(rbase & 0xFFF) & 0xF;
    
    /* Combined assignment 2: Array with computed index and narrowing */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 5 + 3) % 16;
        
        /* Register source, narrowed store */
        register int rval = rbase + i * 0x1000;
        combined.values[idx] = (short)rval;  /* SUBREG potential */
        
        /* Another complex address calculation */
        int *addr = combined.counters + idx % 8;
        *addr = rval >> 16;                  /* MEM with address expr */
    }
    
    /* Inline assembly to directly influence RTL generation */
    int dummy = 0;
    asm volatile (
        "# Force complex memory operand\n"
        : "=m" (combined.values[5])  /* Memory output with addressing */
        : 
        : "memory"
    );
    
    /* Read back everything */
    volatile unsigned int read_flags = combined.flags;
    volatile unsigned int read_status = combined.status;
    volatile short read_first = combined.values[0];
    volatile int read_counter = combined.counters[0];
    (void)read_flags;
    (void)read_status;
    (void)read_first;
    (void)read_counter;
}

/* Test 5: STRICT_LOW_PART patterns (often from byte/word stores) */
void test_strict_low_part(void) {
    /* Operations that might generate STRICT_LOW_PART */
    volatile short vs;
    volatile unsigned char vuc;
    
    /* These operations on some architectures generate STRICT_LOW_PART
       for sub-word stores that need to preserve upper bits */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    
    /* Byte store after computation */
    vuc = (unsigned char)(r1 + r2);
    
    /* Short store with masking */
    vs = (short)((r1 & 0xFFFF) + (r2 & 0xFFFF));
    
    /* Increment with truncation */
    char c = 100;
    for (int i = 0; i < 10; i++) {
        c = c + 1;  /* May generate byte store with STRICT_LOW_PART */
    }
    vuc = c;
    
    /* Read back */
    volatile int read_sum = vs + vuc;
    (void)read_sum;
}

int main(void) {
    int final_checksum = 0;
    
    /* Run all tests */
    test_bitfield_operations();
    final_checksum += 1;
    
    test_subword_operations();
    final_checksum += 2;
    
    test_complex_memory_addressing();
    final_checksum += 3;
    
    test_combined_patterns();
    final_checksum += 4;
    
    test_strict_low_part();
    final_checksum += 5;
    
    /* Print checksum to ensure all code executes */
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
