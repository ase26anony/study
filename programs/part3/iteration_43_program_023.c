/* Test program to trigger uncovered lines in GCC's resource.cc */
#include <stdint.h>
#include <string.h>

/* Force noinline to prevent optimization from removing our patterns */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bfs;
    
    /* Array for MEM patterns with complex addressing */
    static volatile int mem_array[32][16];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bfs.field1 = (*counter & 0x1F);        /* Should generate ZERO_EXTRACT */
    bfs.field2 = (*counter >> 5) & 0x7F;   /* Another ZERO_EXTRACT */
    
    /* MEM pattern with complex addressing */
    int idx1 = *counter & 0x1F;
    int idx2 = (*counter >> 2) & 0xF;
    
    /* Complex MEM access with pointer arithmetic */
    volatile int val = mem_array[idx1][idx2];  /* MEM with addressing mode */
    
    /* Combine both: MEM of ZERO_EXTRACT address */
    bfs.field3 = val & 0x7;  /* Another ZERO_EXTRACT */
    
    /* Prevent dead code elimination */
    *counter += bfs.field1 + bfs.field2 + bfs.field3;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    /* Variables for SUBREG patterns */
    int32_t int32_val = *counter;
    int16_t int16_val;
    int8_t int8_val;
    
    /* SUBREG pattern: type punning through pointer casting */
    /* Cast to smaller type pointer and assign */
    int16_t *p16 = (int16_t *)&int32_val;
    *p16 = (*counter & 0xFFFF);  /* Should generate SUBREG */
    
    /* Another SUBREG pattern with char */
    int8_t *p8 = (int8_t *)&int32_val;
    p8[1] = (*counter >> 8) & 0xFF;  /* SUBREG access */
    
    /* STRICT_LOW_PART pattern using inline assembly */
    /* Modify only low byte of a register */
    uint16_t asm_var = *counter & 0xFFFF;
    
    /* Inline assembly that should generate STRICT_LOW_PART */
    /* "=q" constraint for byte-addressable register */
    asm volatile (
        "addb $1, %0\n\t"      /* Modify low byte only */
        : "=q"(asm_var)        /* =q for byte register */
        : "0"(asm_var)         /* Input in same register */
        : "cc"                 /* Clobbers condition codes */
    );
    
    /* More SUBREG: access parts of larger variable */
    int64_t int64_val = (int64_t)*counter * 1000;
    int32_t *p32_from_64 = (int32_t *)&int64_val;
    int32_val = p32_from_64[0];  /* SUBREG from 64-bit to 32-bit */
    
    /* Prevent dead code elimination */
    *counter += asm_var + int32_val;
}

/* Function C: Mix multiple patterns in complex expression */
NOINLINE static void func_c(volatile int *counter, volatile int selector) {
    /* Array of structs with bit-fields */
    struct mixed_struct {
        volatile unsigned int flags : 4;
        volatile int value;
    } ms_array[8];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        ms_array[i].flags = i & 0xF;
        ms_array[i].value = i * 100;
    }
    
    /* Complex expression with ternary selecting address */
    struct mixed_struct *selected;
    
    /* Ternary operator with MEM addressing */
    selected = (selector & 1) ? &ms_array[0] : &ms_array[4];
    
    /* ZERO_EXTRACT on selected struct */
    selected->flags = (*counter & 0xF);  /* ZERO_EXTRACT */
    
    /* MEM access through pointer with offset */
    volatile int *val_ptr = &selected->value;
    int offset = (*counter & 3) * sizeof(int);
    
    /* Complex MEM access - comment out for runtime safety but keep for compilation */
    /* volatile int complex_mem = *(volatile int *)((char *)val_ptr + offset); */
    
    /* Alternative safe version that still generates MEM patterns */
    volatile int safe_mem;
    if (offset < 8 * sizeof(int)) {
        safe_mem = ms_array[offset / sizeof(int)].value;  /* MEM with indexing */
    }
    
    /* SUBREG pattern in the same function */
    uint32_t combined = (*counter << 16) | (selector & 0xFFFF);
    uint16_t *p_combined = (uint16_t *)&combined;
    p_combined[0] = safe_mem & 0xFFFF;  /* SUBREG store */
    
    /* Prevent dead code elimination */
    *counter += selected->flags + safe_mem;
}

/* Helper with loop to increase RTL complexity */
NOINLINE static void complex_loop(volatile int iterations) {
    volatile int arr[4][4] = {{0}};
    volatile short short_arr[8] = {0};
    
    for (volatile int i = 0; i < iterations && i < 4; i++) {
        for (volatile int j = 0; j < iterations && j < 4; j++) {
            /* MEM patterns with 2D array indexing */
            arr[i][j] = i * j;
            
            /* SUBREG patterns with mixed sizes */
            int *p_int = (int *)&short_arr[i];
            *p_int = (i << 16) | j;  /* SUBREG store to short array as int */
            
            /* ZERO_EXTRACT-like using bit operations */
            volatile struct {
                unsigned int low_bits : 3;
                unsigned int high_bits : 5;
            } bits;
            
            bits.low_bits = (i + j) & 0x7;
            bits.high_bits = (i * j) & 0x1F;
        }
    }
}

int main(int argc, char *argv[]) {
    volatile int counter = 0;
    volatile int selector = 1;
    
    /* Use argc to bound loops for compilation safety */
    int loop_limit = (argc > 1) ? 10 : 5;
    
    /* Initialize some volatile data */
    volatile int init_array[16];
    for (int i = 0; i < 16; i++) {
        init_array[i] = i * i;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* Update selector for func_c */
        selector = (iter & 1) ? 1 : 2;
        
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, selector);
        
        /* Additional complex loop */
        complex_loop(iter + 1);
        
        /* Mix in some direct MEM/SUBREG operations */
        {
            /* Direct SUBREG pattern */
            long long big_val = counter * 1000LL;
            int *p_big = (int *)&big_val;
            int part = p_big[iter & 1];  /* SUBREG access */
            
            /* MEM with addressing calculation */
            volatile int *mem_ptr = &init_array[(iter * 3) & 0xF];
            counter += *mem_ptr + part;
        }
    }
    
    /* Final dummy result to prevent optimization */
    volatile int result = counter;
    
    return result & 0xFF;  /* Return non-zero to be safe */
}
