#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bf = {0};
    
    /* Variables to create complex expressions */
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0x1);      /* Could generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ (c & 0xFF);
    bf.field12 = (a + b) & 0xFFF;           /* 12-bit mask */
    
    /* Use __builtin_parity on sub-word data */
    unsigned char byte_val = (a >> 8) & 0xFF;
    bf.field4 = __builtin_parity(byte_val) & 0xF;
}

/* Test 2: SUBREG operations through type narrowing */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2;
    volatile char vc1, vc2;
    
    /* Register sources of different sizes */
    register int ri1 = 0x12345678;
    register int ri2 = 0x9ABCDEF0;
    register short rs1 = 0x1234;
    register char rc1 = 0x56;
    
    /* Explicit narrowing casts - may create SUBREG in SET_DEST */
    vs1 = (short)ri1;                    /* int -> short */
    vs2 = (short)(ri1 + ri2);            /* expression then narrowing */
    vc1 = (char)rs1;                     /* short -> char */
    vc2 = (char)(rc1 + 0x10);            /* char arithmetic with truncation */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100, c2 = 100;
    volatile char vc3 = c1 + c2;         /* char addition with overflow truncation */
    
    /* Mixed-type operations */
    short s1 = 1000;
    char c3 = 50;
    volatile short vs3 = s1 + c3;        /* promotion then demotion */
}

/* Test 3: Complex memory addressing for MEM_P(x) */
void test_complex_memory_addressing(void) {
    /* Multi-dimensional array */
    int arr2d[16][16];
    int * restrict ptr1d = (int*)arr2d;  /* restrict helps keep addressing */
    
    /* Complex index calculations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear index calculation */
            int idx = i * 13 + j * 7 + 3;  /* Prime multipliers for complexity */
            
            /* Register source */
            register int src = i * 100 + j;
            
            /* Store with complex addressing */
            arr2d[i][j] = src;                    /* 2D array access */
            ptr1d[idx % 256] = src + 1;           /* 1D with computed index */
        }
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s;
    
    int * restrict p = s.data;
    for (int i = 0; i < 16; i++) {
        /* Pointer arithmetic with multiple offsets */
        register int val = i * i;
        p[i * 2 + 1] = val;                      /* struct member with offset */
        *(p + i + 5) = val + 1;                  /* pointer arithmetic */
    }
}

/* Test 4: Combined patterns */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        short values[16];
        unsigned int status : 4;
    } combined = {0};
    
    /* Pointer to the array */
    volatile short *val_ptr = combined.values;
    
    /* Register variables for sources */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    
    /* Combined test 1: Bitfield assignment with expression */
    combined.flags = ((r1 & 0xFF) + (r2 & 0xFF)) & 0xFF;
    
    /* Combined test 2: Array element with narrowing cast */
    for (int i = 0; i < 8; i++) {
        /* Complex index */
        int idx = (i * 7 + 3) % 16;
        
        /* Narrowing assignment to array element */
        val_ptr[idx] = (short)(r1 + i);          /* SUBREG in MEM context */
        
        /* Another bitfield assignment */
        combined.status = (i & 0xF);
    }
    
    /* Inline assembly to directly influence RTL */
    int array[16];
    int complex_index = 5 * 3 + 2;  /* Non-trivial computation */
    
    /* asm with memory output constraint */
    asm volatile (
        "# Force memory operand with addressing"
        : "=m" (array[complex_index])  /* Complex addressing in output */
        :
        : "memory"
    );
    
    /* Another asm with bitfield-like constraint */
    unsigned int dummy;
    asm volatile (
        "# Potential bitfield operation"
        : "=r" (dummy)
        : "0" (combined.flags)
    );
}

/* Main function executing all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern tests...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    test_subreg_operations();
    test_complex_memory_addressing();
    test_combined_patterns();
    
    /* Create some live values to prevent optimization */
    volatile int anti_opt = 0;
    
    /* Use values in a way compiler can't eliminate */
    struct {
        unsigned int f : 4;
    } live_bf = {0};
    
    live_bf.f = 7;
    anti_opt = live_bf.f;
    
    short live_short = 100;
    live_short = (short)(live_short + anti_opt);
    anti_opt += live_short;
    
    int live_arr[4] = {1, 2, 3, 4};
    live_arr[anti_opt % 4] = live_short;
    anti_opt += live_arr[0];
    
    checksum = anti_opt;
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
