/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stddef.h>
#include <string.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT + MEM */
NOINLINE static void pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bf;
    
    /* Array with complex addressing for MEM */
    volatile int arr[16][8];
    volatile int *ptr;
    
    /* Initialize to prevent constant propagation */
    int idx = *counter & 0xF;
    int jdx = (*counter >> 4) & 0x7;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bf.field1 = idx & 0x1F;
    bf.field2 = (idx * 3) & 0x7F;
    bf.field3 = jdx & 0x7;
    
    /* MEM pattern with complex addressing */
    ptr = &arr[idx][jdx];
    *ptr = bf.field1 + bf.field2;
    
    /* More complex MEM addressing with pointer arithmetic */
    volatile int v = arr[(idx + 1) & 0xF][(jdx * 2) & 0x7];
    v = arr[idx][(jdx + *counter) & 0x7];
}

/* Pattern 2: STRICT_LOW_PART + SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(volatile int *counter) {
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART pattern via inline assembly */
    /* Modify only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* Same as output */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG pattern via type punning */
    /* Access int as smaller types */
    short *ps = (short*)&i;
    *ps = s + 1;  /* This generates SUBREG in RTL */
    
    /* More SUBREG patterns */
    char *pc = (char*)&i;
    pc[1] = c;
    pc[3] = (*counter >> 8) & 0xFF;
    
    /* Mixed-size operations */
    i = (i & 0xFFFF0000) | (s & 0xFFFF);
    s = (short)(i >> 8);
}

/* Pattern 3: Complex expression mixing patterns */
NOINLINE static void pattern_complex_mix(volatile int *counter) {
    /* Array with volatile elements */
    volatile int matrix[8][8];
    volatile long results[4];
    
    /* Struct with bit-fields at different positions */
    struct mixed_bf {
        volatile unsigned short a : 4;
        volatile unsigned int b : 12;
        volatile unsigned char c : 2;
    } mbf;
    
    int idx = (*counter >> 2) & 0x7;
    int jdx = (*counter >> 5) & 0x7;
    int kdx = (*counter >> 8) & 0x3;
    
    /* Initialize */
    mbf.a = idx & 0xF;
    mbf.b = (idx * jdx) & 0xFFF;
    mbf.c = kdx & 0x3;
    
    /* Complex addressing with ternary operator */
    volatile int *addr = (idx > 4) ? &matrix[idx][jdx] : &matrix[jdx][idx];
    
    /* Assignment that could generate multiple patterns */
    *addr = mbf.a + mbf.b;
    
    /* More complex: pointer to bit-field member simulation */
    results[kdx] = (long)(*addr) * (mbf.c + 1);
    
    /* SUBREG with different sizes */
    if (*counter & 1) {
        /* Access long as int */
        int *pi = (int*)&results[kdx];
        *pi = (*counter & 0x7FFF);
    }
}

/* Pattern 4: Nested patterns in loops */
NOINLINE static void pattern_nested(volatile int *counter) {
    volatile struct {
        unsigned int flags : 8;
        unsigned int value : 16;
        unsigned int extra : 8;
    } device_reg;
    
    volatile int buffer[32];
    volatile int *bufptr = buffer;
    
    int iterations = (*counter & 0x7) + 1;
    
    for (int n = 0; n < iterations; n++) {
        /* ZERO_EXTRACT pattern in loop */
        device_reg.flags = (n * 37) & 0xFF;
        device_reg.value = (n + *counter) & 0xFFFF;
        device_reg.extra = (n ^ *counter) & 0xFF;
        
        /* MEM with pointer arithmetic */
        bufptr = &buffer[(n * 5) & 0x1F];
        *bufptr = device_reg.value;
        
        /* Inline asm for STRICT_LOW_PART */
        volatile char byte_val = n & 0xFF;
        asm volatile (
            "subb $1, %0\n\t"
            : "=q"(byte_val)
            : "0"(byte_val)
            : "cc"
        );
        
        /* SUBREG access */
        if (n & 1) {
            short *sp = (short*)bufptr;
            *sp = (short)byte_val;
        }
    }
}

/* Main driver that calls all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int iterations = 10;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        iterations = (argv[1][0] & 0x7F); /* Simple hash of first arg char */
        if (iterations == 0) iterations = 5;
    }
    
    /* Initialize some data */
    volatile int data_array[64];
    for (int i = 0; i < 64; i++) {
        data_array[i] = i * 3;
    }
    
    /* Main loop calling pattern functions */
    for (counter = 0; counter < iterations; counter++) {
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_complex_mix(&counter);
        pattern_nested(&counter);
        
        /* Prevent dead code elimination */
        data_array[counter & 0x3F] += counter;
    }
    
    /* Final dummy computation */
    volatile int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data_array[i];
    }
    
    /* The program doesn't need correct semantics for compilation */
    /* This just ensures variables are used */
    return (sum > 0) ? 0 : 1;
}
