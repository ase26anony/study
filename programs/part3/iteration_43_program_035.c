/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stddef.h>
#include <string.h>

/* Force noinline to prevent optimization from removing pattern functions */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT + MEM combination */
NOINLINE static void pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct BitFieldStruct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    };
    
    /* Array with pointer arithmetic for MEM with complex addressing */
    static volatile int mem_array[32][16];
    
    struct BitFieldStruct bfs = {0};
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bfs.field1 = (*counter & 0x1F);      /* Should generate ZERO_EXTRACT */
    bfs.field2 = (*counter >> 5) & 0x7F;
    
    /* MEM pattern with complex addressing */
    int idx1 = *counter % 32;
    int idx2 = (*counter * 7) % 16;
    
    /* This should generate MEM with address computation */
    volatile int val = mem_array[idx1][idx2];
    
    /* Combine patterns: use bit-field value in memory access */
    mem_array[idx1][idx2] = bfs.field1 + bfs.field3;
    
    /* Prevent dead code elimination */
    *counter += val;
}

/* Pattern 2: STRICT_LOW_PART + SUBREG combination */
NOINLINE static void pattern_strict_low_part_subreg(volatile int *counter) {
    volatile short s_val = (short)*counter;
    volatile int i_val = *counter;
    
    /* STRICT_LOW_PART pattern using inline assembly */
    /* Modify only the low byte of a register */
    unsigned char byte_val = (unsigned char)*counter;
    
    /* Inline assembly that should generate STRICT_LOW_PART */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(byte_val) 
        : "0"(byte_val)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different-sized accesses */
    /* Cast pointer to larger type to pointer to smaller type */
    int *p_int = &i_val;
    short *p_short = (short *)p_int;
    
    /* This should generate SUBREG accesses */
    *p_short = (short)(*counter + byte_val);
    
    /* Another SUBREG pattern with char access */
    char *p_char = (char *)p_int;
    p_char[1] = byte_val;
    
    /* Prevent dead code elimination */
    *counter += i_val + s_val;
}

/* Pattern 3: Mixed patterns with ternary operator */
NOINLINE static void pattern_mixed_complex(volatile int *counter) {
    /* Array for MEM patterns */
    static volatile int mixed_array[64];
    
    /* Struct with bit-fields for ZERO_EXTRACT */
    struct MixedStruct {
        volatile unsigned int flag : 1;
        volatile unsigned int value : 8;
        volatile unsigned int pad : 23;
    };
    
    struct MixedStruct ms = {0};
    volatile int *ptr;
    
    /* Complex expression with ternary selecting address */
    int condition = *counter & 1;
    
    /* This should generate interesting addressing modes */
    ptr = condition ? &mixed_array[*counter % 32] : 
                     &mixed_array[32 + (*counter % 32)];
    
    /* ZERO_EXTRACT assignment */
    ms.flag = condition;
    ms.value = (*counter & 0xFF);
    
    /* MEM access with the selected pointer */
    *ptr = ms.value;
    
    /* Additional SUBREG pattern */
    long long big_val = (long long)*counter * 1000;
    int *small_ptr = (int *)&big_val;
    
    /* Access different parts of the long long */
    small_ptr[0] += ms.value;  /* Should involve SUBREG */
    small_ptr[1] += *counter;
    
    /* Prevent dead code elimination */
    *counter += *ptr + small_ptr[0];
}

/* Pattern 4: Nested patterns in loops */
NOINLINE static void pattern_nested_loop(volatile int *counter) {
    volatile int local_counter = *counter;
    
    /* Multi-dimensional array for complex MEM addressing */
    static volatile int matrix[8][8][8];
    
    /* Loop to generate repeated patterns */
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 4; j++) {
            /* ZERO_EXTRACT on a bit-field in struct */
            struct LoopStruct {
                volatile unsigned int a : 2;
                volatile unsigned int b : 4;
                volatile unsigned int c : 2;
            } ls;
            
            ls.a = i & 3;
            ls.b = j & 0xF;
            ls.c = (i + j) & 3;
            
            /* MEM with multi-dimensional indexing */
            int idx = (i * 4 + j) & 7;
            matrix[i][j][idx] = ls.b;
            
            /* STRICT_LOW_PART via inline assembly on char */
            unsigned char cval = (unsigned char)(i * 16 + j);
            asm volatile (
                "orb $0x1, %0\n\t"
                : "=q"(cval)
                : "0"(cval)
                : "cc"
            );
            
            /* SUBREG: access part of int as short */
            int temp = matrix[i][j][idx];
            short *sp = (short *)&temp;
            sp[0] += cval;  /* Should generate SUBREG */
            
            matrix[i][j][idx] = temp;
            
            local_counter += cval;
        }
    }
    
    *counter += local_counter;
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    volatile int counter = 0;
    volatile int iterations = 10;
    
    /* Use argc to bound iterations if provided */
    if (argc > 1) {
        iterations = (argv[1][0] - '0');
        if (iterations <= 0) iterations = 5;
    }
    
    /* Initialize some data */
    volatile int data_array[100];
    for (volatile int i = 0; i < 100; i++) {
        data_array[i] = i * 3;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        counter = i;
        
        /* Call each pattern function with counter */
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_mixed_complex(&counter);
        pattern_nested_loop(&counter);
        
        /* Complex MEM addressing in main loop */
        int idx1 = counter % 10;
        int idx2 = (counter * 7) % 10;
        volatile int *ptr = &data_array[idx1 * 10 + idx2];
        
        /* Force use of all patterns' results */
        *ptr += counter;
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile ("" : : "r"(counter) : "memory");
    }
    
    /* Final dummy operation to prevent elimination */
    volatile int result = 0;
    for (volatile int i = 0; i < 100; i++) {
        result += data_array[i];
    }
    
    /* Return something based on result to prevent optimization */
    return result > 0 ? 0 : 1;
}
