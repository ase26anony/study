/* test_resource_coverage.c
 * Designed to trigger mark_referenced_resources for:
 * - ZERO_EXTRACT (volatile bit-field assignments)
 * - STRICT_LOW_PART (inline assembly with byte constraints)
 * - SUBREG (type punning between different sizes)
 * - MEM (complex addressing modes)
 */

#include <stddef.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline, noipa))

/* Function A: ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *base, int idx1, int idx2) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 7;
        volatile unsigned int f3 : 3;
    } bs;
    
    /* Initialize */
    bs.f1 = 1;
    bs.f2 = 2;
    
    /* Complex MEM addressing with pointer arithmetic */
    volatile int *ptr = base + idx1 * 8 + idx2;
    
    /* Assignment that may generate ZERO_EXTRACT */
    bs.f3 = *ptr & 0x7;
    
    /* More complex MEM access with multiple indices */
    volatile int val = *(ptr + idx1) + *(ptr - idx2);
    
    /* Another bit-field assignment */
    bs.f1 = val & 0x1F;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile short *ps, volatile char *pc) {
    int combined = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* STRICT_LOW_PART: inline assembly modifying only low byte */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(temp_char)
        : "0"(temp_char)
        : "cc"
    );
    
    /* SUBREG: Type punning between int and short */
    int *pi = (int *)ps;
    *pi = (*pi & 0xFFFF0000) | (temp_short & 0xFFFF);
    
    /* More SUBREG: Access int as chars */
    char *pcc = (char *)&combined;
    pcc[0] = *pc;
    pcc[1] = temp_char;
    
    /* Another STRICT_LOW_PART operation */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(pcc[2])
        : "0"(pcc[2])
        : "cc"
    );
}

/* Function C: Mixed patterns with ternary operator */
NOINLINE static void func_c(volatile int *arr, int i, int j, int cond) {
    /* Struct with bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int flag : 1;
        volatile unsigned int value : 10;
    } status;
    
    /* Complex addressing with ternary selection */
    volatile int *selected = cond ? 
        (arr + i * 16 + j) : 
        (arr + j * 8 + i);
    
    /* MEM access with complex index */
    volatile int data = selected[i % 4];
    
    /* Bit-field assignment (ZERO_EXTRACT) based on MEM value */
    status.value = data & 0x3FF;
    status.flag = (data >> 10) & 0x1;
    
    /* Additional SUBREG access */
    short *sp = (short *)&data;
    sp[1] = (short)(status.value);
}

/* Helper with complex loop containing all patterns */
NOINLINE static void complex_loop(volatile int *mem, int iterations) {
    volatile int counter = 0;
    
    /* Arrays for MEM patterns */
    volatile int array_2d[8][8];
    volatile short short_array[32];
    volatile char char_array[64];
    
    for (int i = 0; i < iterations; i++) {
        /* Update volatile counter to prevent optimization */
        counter++;
        
        /* Call pattern functions with varying arguments */
        func_a(mem + counter, i & 7, (i >> 3) & 7);
        
        func_b(short_array + (i & 31), 
               char_array + (i & 63));
        
        func_c(&array_2d[0][0], 
               i & 7, 
               (i + 1) & 7, 
               i & 1);
        
        /* Complex MEM addressing inside loop */
        volatile int *ptr = mem + 
            (i * 3) % 32 + 
            ((i * 5) % 16) * 8;
        
        /* Mixed-size accesses causing SUBREG */
        *(volatile short *)ptr = (short)counter;
        *(volatile char *)(ptr + 1) = (char)(counter >> 8);
        
        /* Bit-field in loop (ZERO_EXTRACT) */
        struct {
            volatile unsigned int loop_bit : 4;
        } loop_var;
        loop_var.loop_bit = i & 0xF;
    }
}

/* Main function with volatile controls */
int main(int argc, char **argv) {
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    int iterations = argc > 1 ? 8 : 4;
    
    /* Volatile memory area */
    volatile int memory_pool[128];
    
    /* Initialize with pattern */
    for (int i = 0; i < 128; i++) {
        memory_pool[i] = i * 3 + 1;
    }
    
    /* Run the complex loop */
    complex_loop(memory_pool, iterations);
    
    /* Additional direct pattern calls */
    volatile short s = 0;
    volatile char c = 0;
    func_b(&s, &c);
    
    volatile int arr[64];
    func_c(arr, 1, 2, 1);
    
    /* Dummy result to prevent elimination */
    volatile int dummy_sum = 0;
    for (int i = 0; i < 64; i++) {
        dummy_sum += arr[i % 64];
    }
    
    return dummy_sum != 0; /* Non-deterministic but valid return */
}
