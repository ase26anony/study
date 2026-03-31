/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct S { 
        volatile unsigned int f1:5;
        volatile unsigned int f2:3;
        volatile unsigned int f3:8;
    } s;
    
    /* Array with complex indexing for MEM patterns */
    volatile int arr[10][10];
    
    /* Force multiple ZERO_EXTRACT patterns */
    s.f1 = (*counter) & 0x1F;
    s.f2 = (*counter >> 5) & 0x07;
    s.f3 = (*counter >> 8) & 0xFF;
    
    /* Complex MEM access with pointer arithmetic */
    int i = (*counter) % 10;
    int j = (*counter * 3) % 10;
    volatile int v = arr[i][j];
    
    /* More complex addressing */
    volatile int *ptr = &arr[0][0];
    ptr += i * 10 + j;
    volatile int v2 = *ptr;
    
    /* Prevent dead code elimination */
    *counter += s.f1 + v + v2;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile char c = (*counter) & 0xFF;
    volatile short s = (*counter) & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART via inline assembly modifying byte part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)  /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different operation */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG patterns through type punning */
    int *pi = &i;
    short *ps = (short *)pi;  /* Cast to smaller type */
    *ps = (*counter) & 0xFFFF;  /* SUBREG store */
    
    /* More SUBREG: access different parts of integer */
    char *pc = (char *)pi;
    pc[1] = (*counter >> 8) & 0xFF;  /* Another SUBREG */
    pc[3] = (*counter >> 24) & 0xFF;
    
    /* Mixed-size operations */
    s = (short)(i >> 16);  /* Potential SUBREG extract */
    i = (int)s << 8;       /* Potential SUBREG insert */
    
    *counter += c + s + i;
}

/* Function C: Complex expression mixing patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *arr_base) {
    /* Union for type punning - SUBREG patterns */
    union U {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = *counter;
    
    /* Ternary selecting different access patterns */
    volatile int *ptr = (*counter & 1) ? 
                       (volatile int *)&u.c[1] :  /* SUBREG access */
                       (volatile int *)arr_base;  /* MEM access */
    
    /* Complex addressing with multiple indices */
    int idx1 = (*counter) % 5;
    int idx2 = (*counter * 7) % 5;
    
    /* MEM with complex addressing mode */
    volatile int val = *(ptr + idx1 * 5 + idx2);
    
    /* Bit-field in struct for ZERO_EXTRACT */
    struct T {
        volatile unsigned int bits:4;
        volatile unsigned int more_bits:12;
    } t;
    
    /* Conditional ZERO_EXTRACT assignment */
    t.bits = (val & 0x0F) | ((*counter) & 0x01);
    t.more_bits = (val >> 4) & 0x0FFF;
    
    /* More pointer arithmetic for MEM */
    volatile int *p = arr_base;
    for (int k = 0; k < 3; k++) {
        p += (idx1 + k) * idx2;  /* Complex addressing */
        volatile int tmp = *p;    /* MEM reference */
        t.more_bits += tmp & 0x7;
    }
    
    *counter += t.bits + t.more_bits + val;
}

/* Function D: Additional patterns with loops */
NOINLINE static void func_d(volatile int *counter, volatile int *mem) {
    /* Array of structs with bit-fields */
    struct BF {
        volatile unsigned int a:2;
        volatile unsigned int b:6;
        volatile unsigned int c:4;
    } bf_arr[8];
    
    /* Initialize with ZERO_EXTRACT patterns */
    for (int i = 0; i < 8; i++) {
        bf_arr[i].a = (*counter + i) & 0x3;
        bf_arr[i].b = (*counter + i * 2) & 0x3F;
        bf_arr[i].c = (*counter + i * 3) & 0xF;
    }
    
    /* Complex MEM access in loop */
    volatile int sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Pointer with offset calculation */
        volatile int *p = mem + i * 2 + (*counter & 7);
        sum += *p;  /* MEM reference */
        
        /* Update bit-field based on MEM value */
        bf_arr[i].b = (*p) & 0x3F;
    }
    
    /* More SUBREG patterns */
    int large = *counter * 100;
    short *sp = (short *)&large;
    sp[0] = sum & 0xFFFF;  /* SUBREG store */
    sp[1] = (sum >> 16) & 0xFFFF;
    
    *counter += large + sum;
}

int main(int argc, char **argv) {
    /* Volatile to force resource tracking */
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Array for MEM patterns */
    volatile int mem_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        mem_array[i] = i * 3;
    }
    
    /* Loop to generate repeated RTL patterns */
    /* Use argc to bound the loop for analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, mem_array);
        func_d(&counter, mem_array);
        
        /* Complex addressing mode */
        volatile int idx = counter % 50;
        volatile int *complex_ptr = &mem_array[idx * 2 + iter];
        result += *complex_ptr;
        
        /* Additional volatile operations */
        counter += iter;
    }
    
    /* Final dummy use to prevent elimination */
    volatile int final = result + counter;
    
    /* The program doesn't need correct runtime semantics,
     * but this prevents it from being completely optimized away */
    return final & 1;
}
