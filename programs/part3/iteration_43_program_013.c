/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdint.h>
#include <string.h>

/* Force compiler to generate specific RTL patterns */

/* Pattern 1: ZERO_EXTRACT with volatile bit-fields and MEM */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *arr, int idx1, int idx2) {
    struct S {
        volatile unsigned int f1:5;
        volatile unsigned int f2:3;
        volatile unsigned int f3:8;
    } s;
    
    /* ZERO_EXTRACT pattern: volatile bit-field assignment */
    s.f1 = 1;
    s.f2 = 2;
    s.f3 = 3;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[idx1 * 10 + idx2];
    
    /* Combine: use bit-field value in memory access */
    arr[(s.f1 + idx1) * 10 + (s.f2 + idx2)] = s.f3;
}

/* Pattern 2: STRICT_LOW_PART and SUBREG */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile short *ps, volatile char *pc) {
    int i = 0x12345678;
    short s = 0xABCD;
    char c = 0xEF;
    
    /* SUBREG pattern: type punning with different sizes */
    short *psi = (short*)&i;
    *psi = s;  /* Access int as short - generates SUBREG */
    
    /* STRICT_LOW_PART pattern: inline assembly modifying byte register */
    asm volatile (
        "addb $1, %0\n\t"
        "subb $2, %0"
        : "=q"(c)    /* =q constraint for byte-addressable register */
        : "0"(c)     /* matching input constraint */
        : "cc"
    );
    
    /* More SUBREG: access via different pointer types */
    char *pci = (char*)&i;
    pci[1] = c;  /* Modify middle byte of int */
    
    /* Store results to volatile to prevent elimination */
    *ps = *psi;
    *pc = c;
}

/* Pattern 3: Mixed patterns with ternary and complex addressing */
static void __attribute__((noinline))
pattern_mixed(volatile int *base, int cond, int idx) {
    struct BF {
        volatile unsigned int flag:1;
        volatile unsigned int value:7;
    } bf;
    
    /* Initialize bit-field */
    bf.flag = cond & 1;
    bf.value = idx & 0x7F;
    
    /* Complex MEM addressing with ternary based on bit-field */
    volatile int *addr = bf.flag ? 
                        &base[idx * 2] : 
                        &base[idx * 3 + 1];
    
    /* ZERO_EXTRACT from bit-field combined with MEM access */
    int temp = *addr;
    bf.value = (temp >> 3) & 0x7F;  /* Extract bits into bit-field */
    
    /* SUBREG through pointer cast */
    short *short_ptr = (short*)addr;
    *short_ptr = bf.value;  /* Store bit-field value as short */
    
    /* Another MEM with index calculation */
    base[(bf.value + idx) % 16] = temp;
}

/* Pattern 4: Loop-based pattern generation */
static void __attribute__((noinline))
pattern_loop_based(volatile int *arr, int size) {
    struct Mixed {
        volatile unsigned int bits:4;
        volatile int full;
    } m;
    
    for (volatile int i = 0; i < size && i < 8; i++) {
        /* ZERO_EXTRACT in loop */
        m.bits = i & 0xF;
        
        /* MEM with complex addressing in loop */
        volatile int *elem = &arr[i * 4 + m.bits];
        
        /* SUBREG access within loop */
        char *byte_ptr = (char*)&m.full;
        byte_ptr[i % 4] = *elem & 0xFF;
        
        /* Conditional STRICT_LOW_PART-like operation */
        if (m.bits & 1) {
            char c = byte_ptr[0];
            asm volatile (
                "orb $0x10, %0"
                : "=q"(c)
                : "0"(c)
                : "cc"
            );
            byte_ptr[0] = c;
        }
    }
}

/* Pattern 5: Nested structure with bit-fields and arrays */
static void __attribute__((noinline))
pattern_nested(volatile int *matrix, int rows, int cols) {
    struct Outer {
        struct {
            volatile unsigned int a:3;
            volatile unsigned int b:5;
        } inner;
        volatile int buffer[4];
    } outer;
    
    /* Initialize bit-fields */
    outer.inner.a = rows & 0x7;
    outer.inner.b = cols & 0x1F;
    
    /* MEM with 2D array-like addressing */
    for (volatile int r = 0; r < rows && r < 4; r++) {
        for (volatile int c = 0; c < cols && c < 4; c++) {
            /* Complex addressing */
            int idx = (r * outer.inner.b + c * outer.inner.a) % (rows * cols);
            
            /* ZERO_EXTRACT to bit-field from memory */
            outer.inner.a = matrix[idx] & 0x7;
            
            /* SUBREG access to buffer */
            short *short_buf = (short*)outer.buffer;
            short_buf[r * 2 + c] = matrix[idx] & 0xFFFF;
            
            /* MEM store with modified value */
            matrix[idx] = outer.buffer[r] + short_buf[r * 2 + c];
        }
    }
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    volatile int iteration_counter = 0;
    volatile int dummy_sum = 0;
    
    /* Use argc to bound loops for compilation safety */
    int loop_bound = (argc > 1) ? 5 : 3;
    
    /* Initialize test data */
    volatile int array[100];
    volatile short short_var;
    volatile char char_var;
    volatile int matrix[16][16];
    
    /* Initialize arrays to avoid undefined behavior in compilation */
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Main loop exercising all patterns */
    for (iteration_counter = 0; iteration_counter < loop_bound; iteration_counter++) {
        int idx1 = iteration_counter % 10;
        int idx2 = (iteration_counter * 2) % 10;
        
        /* Call each pattern function */
        pattern_zero_extract_mem(array, idx1, idx2);
        pattern_strict_low_part_subreg(&short_var, &char_var);
        pattern_mixed(array, iteration_counter, idx1);
        pattern_loop_based(array, 10);
        pattern_nested((int*)matrix, 4, 4);
        
        /* Prevent dead code elimination */
        dummy_sum += array[idx1] + short_var + char_var;
    }
    
    /* Final volatile use to prevent optimization */
    asm volatile ("" : : "r"(dummy_sum));
    
    return 0;
}
