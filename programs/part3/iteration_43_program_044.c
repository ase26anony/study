/* Target RTL patterns for resource.cc coverage */
#include <stdint.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct = {0};
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bit_struct.field1 = 1;
    bit_struct.field2 = idx1 & 0x7;
    bit_struct.field3 = (idx1 * idx2) & 0xFF;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[(idx1 * 7 + idx2 * 3) % 64];
    
    /* Combine: use bit-field result in memory access */
    arr[(bit_struct.field3 + idx2) % 64] = val + bit_struct.field1;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline)) 
pattern_strict_low_part_subreg(int base) {
    volatile char c = (char)(base & 0xFF);
    volatile short s = (short)(base & 0xFFFF);
    volatile int i = base;
    
    /* STRICT_LOW_PART pattern via inline assembly */
    /* Modify only low byte of register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)  /* =q constraint for byte-addressable register */
        : "0"(c)   /* matching constraint for input */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "addw $2, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern via type punning */
    /* Access int as smaller types */
    short *ps = (short*)&i;
    char *pc = (char*)&i;
    
    /* These generate SUBREG accesses in RTL */
    *ps = (short)(s + 1);
    pc[2] = (char)(c + 2);
    
    /* Mixed-size operations encourage SUBREG */
    i = (i & 0xFFFF0000) | ((*ps) & 0xFFFF);
}

/* Function C: Complex expression mixing patterns */
static void __attribute__((noinline))
pattern_complex_mix(volatile int *mem, int idx, int cond) {
    /* Struct with volatile bit-fields at different positions */
    struct {
        volatile unsigned int a : 4;
        volatile unsigned int b : 12;
        volatile unsigned int c : 16;
    } bits = {0};
    
    /* Complex addressing for MEM */
    int *ptr1 = mem + idx;
    int *ptr2 = mem + (idx * 2) % 32;
    
    /* Ternary selecting different bit-field operations */
    volatile int *selected = (cond & 1) ? ptr1 : ptr2;
    
    /* ZERO_EXTRACT from bit-field assignment */
    bits.a = (*selected) & 0xF;
    bits.b = ((*selected) >> 4) & 0xFFF;
    
    /* MEM access through selected pointer with offset */
    volatile int temp = selected[(bits.a + bits.b) % 16];
    
    /* More type punning for SUBREG */
    unsigned char *byte_ptr = (unsigned char*)&temp;
    byte_ptr[1] = bits.a;
    byte_ptr[3] = bits.b & 0xFF;
    
    /* Final assignment that may involve multiple RTL transforms */
    *selected = temp + bits.c;
}

/* Helper with array operations for MEM patterns */
static void __attribute__((noinline))
pattern_mem_complex(int size) {
    volatile int array[32][32];
    static volatile int counter = 0;
    
    /* Nested loops with volatile counters */
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 4; j++) {
            /* Complex array indexing - MEM with addressing modes */
            array[i][j] = array[(i + 1) % 4][(j + 2) % 4] + 
                         array[(i * 2) % 4][(j * 3) % 4];
            
            /* Mix with bit-field operations */
            struct {
                volatile unsigned int f : 6;
            } bf = {0};
            bf.f = (array[i][j] + counter++) & 0x3F;
            
            /* More MEM with pointer arithmetic */
            int *row = array[i];
            row[j] = row[(j + bf.f) % 4];
        }
    }
}

/* Main driver that calls all patterns */
int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? 8 : 4;  /* Prevent infinite loops */
    volatile int array[64];
    volatile int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop to trigger resource tracking */
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Call pattern functions with varying arguments */
        pattern_zero_extract_mem((int*)array, iter, iter * 2);
        pattern_strict_low_part_subreg(iter * 100 + 123);
        pattern_complex_mix((int*)array, iter % 32, iter);
        pattern_mem_complex(iter % 8 + 4);
        
        /* Dummy operation to prevent elimination */
        sum += array[iter % 64];
        
        /* Additional volatile operations to force RTL generation */
        volatile int temp = iter;
        asm volatile ("" : "+r"(temp) : : "memory");
    }
    
    /* Prevent dead code elimination of sum */
    asm volatile ("" : : "r"(sum) : "memory");
    
    return 0;
}
