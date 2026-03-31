/* gcc -O2 -m32 -fno-strict-aliasing -funroll-loops -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_a(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct = {0};
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bit_struct.field1 = 1;
    bit_struct.field2 = idx1 & 0x7;
    bit_struct.field3 = idx2 & 0xFF;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[(idx1 * 7 + idx2 * 3) % 64];
    
    /* Use both to prevent elimination */
    bit_struct.field1 = val & 0x1F;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline)) 
pattern_b(int *base_ptr) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = 42;
    volatile short s = 1000;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c) 
        : "0"(c) 
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with short */
    asm volatile (
        "subw $5, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    int i = 0x12345678;
    short *ps = (short*)&i;
    *ps = 0xABCD;  /* SUBREG access to part of integer */
    
    /* More SUBREG: access char part */
    char *pc = (char*)&i;
    pc[1] = 0xEF;
    
    /* Use results */
    *base_ptr = c + s + i;
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_c(volatile int *arr, int idx, int cond) {
    /* Struct with volatile bit-fields */
    struct {
        volatile unsigned int flags : 4;
        volatile unsigned int value : 12;
    } data = {0};
    
    /* Complex expression with ternary selecting address */
    volatile int *ptr = cond ? 
        (volatile int*)((char*)arr + idx * sizeof(int)) : 
        (volatile int*)((char*)arr + (idx % 8) * sizeof(int));
    
    /* MEM with indexing */
    volatile int x = ptr[0] + ptr[1];
    
    /* ZERO_EXTRACT assignment based on condition */
    data.flags = (cond ? 0xA : 0x5);
    data.value = x & 0xFFF;
    
    /* Additional MEM with pointer arithmetic */
    volatile int y = *(volatile int*)((char*)ptr + 4);
    
    /* Use results */
    data.value ^= y;
}

/* Helper with array operations for MEM patterns */
static void __attribute__((noinline))
mem_intensive(int *arr, int size) {
    volatile int sum = 0;
    
    /* Complex array access pattern generating MEM RTL */
    for (volatile int i = 0; i < size - 1; i++) {
        /* Multi-dimensional access pattern */
        int idx = (i * 7) % size;
        int idx2 = (i * 3) % size;
        
        /* MEM with complex addressing */
        arr[idx] = arr[idx2] + arr[(idx + idx2) % size];
        
        /* More pointer arithmetic */
        int *p1 = arr + idx;
        int *p2 = arr + idx2;
        *p1 = *p2 + *(p1 + 1);
        
        sum += arr[i];
    }
    
    /* Prevent elimination */
    arr[0] = sum;
}

int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Volatile data to force resource tracking */
    volatile int counter = 0;
    volatile int array[128];
    volatile int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 128; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop combining all patterns */
    for (volatile int i = 0; i < iterations; i++) {
        int idx1 = (i * 13) % 128;
        int idx2 = (i * 17) % 128;
        int cond = i & 1;
        
        /* Call pattern functions */
        pattern_a((int*)array, idx1, idx2);
        pattern_b((int*)&array[idx1]);
        pattern_c(array, idx2, cond);
        
        /* MEM-intensive operations */
        mem_intensive((int*)array, 64);
        
        /* Accumulate results to prevent elimination */
        result += array[i % 64];
        counter++;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int final = result + counter;
    
    /* The program doesn't need correct runtime semantics,
       but this prevents compiler from removing everything */
    return final != 0 ? 0 : 1;
}
