/* Test program to trigger uncovered lines in resource.cc (lines 282-290) */
#include <stdint.h>
#include <stdlib.h>

/* Force compiler to generate specific RTL patterns */

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_a(volatile int *arr, int idx1, int idx2) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    /* MEM pattern with complex addressing */
    volatile int val = arr[idx1 * 10 + idx2];
    
    /* ZERO_EXTRACT pattern via volatile bit-field assignment */
    bit_struct.field1 = val & 0x1F;
    bit_struct.field2 = (val >> 5) & 0x7;
    
    /* More MEM patterns with pointer arithmetic */
    volatile int *ptr = arr + idx1;
    bit_struct.field3 = *ptr & 0xFF;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bit_struct.field3));
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_b(volatile int counter) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = (char)counter;
    volatile short s = (short)counter;
    
    /* STRICT_LOW_PART pattern via inline assembly */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with different operation */
    asm volatile (
        "orb $0x0F, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG pattern via type punning */
    int i = counter;
    short *ps = (short*)&i;  /* Type punning - may generate SUBREG */
    *ps = (short)(counter + 1);
    
    /* More SUBREG patterns with different sizes */
    long long ll = counter;
    int *pi = (int*)&ll;
    *pi = counter * 2;
    
    /* Mixed-size access for SUBREG */
    volatile int vi = counter;
    volatile char *pc = (volatile char*)&vi;
    pc[1] = (char)(counter >> 8);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(c), "r"(s), "r"(i), "r"(ll));
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_c(volatile int *arr1, volatile int *arr2, int selector) {
    /* Complex expression mixing patterns */
    struct {
        volatile unsigned int bf1 : 4;
        volatile unsigned int bf2 : 12;
    } bs1, bs2;
    
    /* Ternary selecting different bit-field addresses */
    volatile struct {
        volatile unsigned int bf : 6;
    } *bf_ptr = selector ? (void*)&bs1 : (void*)&bs2;
    
    /* MEM with complex indexing and ZERO_EXTRACT */
    volatile int idx = selector & 0x3;
    bf_ptr->bf = arr1[idx] & 0x3F;
    
    /* Another MEM access */
    volatile int temp = arr2[selector % 4];
    
    /* Additional ZERO_EXTRACT */
    bs1.bf1 = temp & 0xF;
    bs2.bf2 = (temp >> 4) & 0xFFF;
    
    /* SUBREG via cast and assignment */
    long long big_val = (long long)temp * 100;
    int *small_ptr = (int*)&big_val;
    volatile int small_val = small_ptr[selector & 1];
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bf_ptr->bf), "r"(small_val));
}

/* Helper function to create more complex MEM addressing */
static void __attribute__((noinline))
complex_mem_access(volatile int (*arr)[10][10], int i, int j, int k) {
    /* Multi-dimensional array with volatile indices */
    volatile int val1 = (*arr)[i][j];
    volatile int val2 = (*arr)[j][k];
    volatile int val3 = (*arr)[k][i];
    
    /* Pointer arithmetic creating complex MEM addresses */
    volatile int *ptr1 = &(*arr)[i][j];
    volatile int *ptr2 = ptr1 + (i * j);
    
    /* Chain of MEM accesses */
    volatile int result = *ptr1 + *ptr2 + val1 + val2 + val3;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result));
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Initialize data structures with volatile members */
    volatile int array1[100];
    volatile int array2[50];
    volatile int multi_array[10][10][10];
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        array1[i] = i * 3;
        if (i < 50) array2[i] = i * 7;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                multi_array[i][j][k] = i + j * 2 + k * 3;
            }
        }
    }
    
    volatile int sum = 0;
    
    /* Main loop to trigger resource tracking across passes */
    for (volatile int i = 0; i < iterations; i++) {
        int idx1 = i % 10;
        int idx2 = (i * 7) % 10;
        int idx3 = (i * 3) % 10;
        
        /* Call pattern functions with volatile-derived arguments */
        pattern_a((int*)array1, idx1, idx2);
        pattern_b(i);
        pattern_c((int*)array1, (int*)array2, i);
        complex_mem_access((volatile int (*)[10][10])multi_array, 
                          idx1, idx2, idx3);
        
        /* Accumulate to prevent elimination */
        sum += array1[i % 100] + array2[i % 50];
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile("" : : "r"(sum));
    
    return 0;
}
