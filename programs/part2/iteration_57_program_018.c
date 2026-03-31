/* reload_stress.c
 * A program designed to stress GCC's reload mechanism and trigger
 * the initialization block in push_reload (lines 1381-1399 of reload.cc).
 *
 * Compilation recommendations:
 *   gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -c reload_stress.c
 *   gcc -O2 -funroll-loops -fno-optimize-sibling-calls -m32 -c reload_stress.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_int = 12345;
volatile double global_double = 3.14159;
volatile char global_char = 'X';
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Function that returns a value, forcing evaluation before assembly */
int get_next_int(void) {
    static int counter = 0;
    return counter++ + global_int;
}

double compute_value(int x) {
    return x * 0.5 + global_double;
}

/* Explicit register variables - may require moving between registers */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_double asm ("xmm0"); /* Even with -mno-sse, this creates interesting cases */

/* Test 1: Many operands with mixed constraints to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4, out5;
    int in1 = get_next_int();
    int in2 = global_int + 1;
    int in3 = global_array[10];
    int in4 = in1 * in2;
    double d1 = global_double;
    double d2 = compute_value(in1);
    char c1 = global_char;
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        "mov %[d1], %%eax \n\t"           /* Force mode change: double to int reg */
        "add %[in1], %%eax \n\t"
        "mov %%eax, %[out1] \n\t"
        "lea (%[in2], %[in3], 2), %%ebx \n\t"
        "mov %%ebx, %[out2] \n\t"
        "imul %[in4], %%eax \n\t"
        "mov %%eax, %[out3] \n\t"
        "movzbl %[c1], %%ecx \n\t"        /* Zero extend char */
        "add %%ecx, %%eax \n\t"
        "mov %%eax, %[out4] \n\t"
        "mov %[reg1], %%edx \n\t"         /* Use explicit register variable */
        "add %%edx, %%eax \n\t"
        "mov %%eax, %[out5] \n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4),
          [out5] "=r" (out5)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [d1] "r" ((int)d1),  /* Cast double to int for r constraint */
          [c1] "r" ((int)c1), [reg1] "r" (reg_var1)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in operands and complex addressing */
int test_nested_calls(void) {
    int result1, result2;
    int *ptr1 = &global_array[get_next_int() % 50];
    int *ptr2 = &global_array[get_next_int() % 50 + 25];
    
    /* Function calls in input operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "mov (%[p1]), %%eax \n\t"
        "add (%[p2]), %%eax \n\t"
        "add %[val1], %%eax \n\t"
        "mov %%eax, %[res1] \n\t"
        "imul %[val2], %%eax \n\t"
        "mov %%eax, %[res2] \n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [p1] "r" (ptr1), [p2] "r" (ptr2),
          [val1] "r" (get_next_int()),      /* Function call as operand */
          [val2] "r" (get_next_int() % 100) /* Another function call */
        : "eax", "memory"
    );
    
    /* Chain of volatile assembly blocks with interdependent operands */
    int temp;
    __asm__ __volatile__ (
        "mov %[r1], %%eax \n\t"
        "inc %%eax \n\t"
        "mov %%eax, %[t] \n\t"
        : [t] "=r" (temp)
        : [r1] "r" (result1)
        : "eax"
    );
    
    __asm__ __volatile__ (
        "add %[t], %[r2] \n\t"
        : [r2] "+r" (result2)
        : [t] "r" (temp)
        : "cc"
    );
    
    return result1 + result2 + temp;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    short s1 = 1000;
    short s2 = 2000;
    char c1 = 64;
    int i1 = 50000;
    double d1 = 2.71828;
    float f1 = 1.41421f;
    long long ll1 = 0x123456789ABCDEF0LL;
    
    int out_int;
    short out_short;
    char out_char;
    long long out_ll;
    
    /* Mixed types in same assembly statement */
    __asm__ __volatile__ (
        "movswl %[s1], %%eax \n\t"        /* Sign extend short to int */
        "add %[i1], %%eax \n\t"
        "movzbl %[c1], %%ebx \n\t"        /* Zero extend char */
        "add %%ebx, %%eax \n\t"
        "mov %%eax, %[oi] \n\t"
        "mov %[s2], %[os] \n\t"           /* Direct short move */
        "movb %[c1], %[oc] \n\t"          /* Direct char move */
        "movq %[ll1], %[oll] \n\t"        /* 64-bit move */
        : [oi] "=r" (out_int), [os] "=r" (out_short),
          [oc] "=r" (out_char), [oll] "=r" (out_ll)
        : [s1] "r" (s1), [s2] "r" (s2), [c1] "r" (c1),
          [i1] "r" (i1), [ll1] "r" (ll1),
          "m" (d1), "m" (f1)              /* Memory constraints for floats */
        : "eax", "ebx", "memory"
    );
    
    /* Force double to be used in integer context */
    int double_as_int;
    __asm__ __volatile__ (
        "movl %[d1], %[dai] \n\t"         /* Treat double bits as int */
        : [dai] "=r" (double_as_int)
        : [d1] "m" (d1)
        : "memory"
    );
    
    return out_int + out_short + out_char + (int)out_ll + double_as_int;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int result = 0;
    int value1 = get_next_int();
    int value2 = get_next_int();
    
    /* Try to force value into specific register classes */
    __asm__ __volatile__ (
        "mov %[v1], %%eax \n\t"           /* Explicit eax constraint */
        "test %%eax, %%eax \n\t"          /* Sets flags */
        "setg %%al \n\t"                  /* Result in al based on flags */
        "movzbl %%al, %[res] \n\t"
        : [res] "=r" (result)
        : [v1] "r" (value1)
        : "eax", "cc"
    );
    
    /* Complex case with multiple specific registers */
    int a, b, c;
    __asm__ __volatile__ (
        "mov %[v2], %%ebx \n\t"
        "mov %%ebx, %%eax \n\t"           /* ebx -> eax */
        "add $1, %%eax \n\t"
        "mov %%eax, %[a] \n\t"
        "mov %%ebx, %[b] \n\t"
        "lea (%%ebx, %%eax, 2), %[c] \n\t"
        : [a] "=r" (a), [b] "=r" (b), [c] "=r" (c)
        : [v2] "r" (value2)
        : "eax", "ebx", "cc"
    );
    
    /* Assembly with immediate constraints that may need reloads */
    __asm__ __volatile__ (
        "add $0x12345678, %[r] \n\t"
        : [r] "+r" (result)
        :
        : "cc"
    );
    
    return result + a + b + c;
}

/* Test 5: Array indexing with non-constant offsets */
int test_array_indexing(void) {
    int sum = 0;
    int index1 = get_next_int() % 50;
    int index2 = get_next_int() % 50;
    int index3 = get_next_int() % 50;
    
    /* Complex addressing modes that must be computed into registers */
    __asm__ __volatile__ (
        "mov %[idx1], %%eax \n\t"
        "mov %[arr](,%%eax,4), %%ebx \n\t"  /* Scale index by 4 */
        "mov %[idx2], %%ecx \n\t"
        "add %[arr](,%%ecx,4), %%ebx \n\t"
        "mov %[idx3], %%edx \n\t"
        "add %[arr](,%%edx,4), %%ebx \n\t"
        "mov %%ebx, %[sum] \n\t"
        : [sum] "=r" (sum)
        : [arr] "r" (global_array),        /* Array base */
          [idx1] "r" (index1),
          [idx2] "r" (index2),
          [idx3] "r" (index3)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Pointer arithmetic in operands */
    int *ptr = global_array + index1;
    int offset = index2 * sizeof(int);
    int val;
    
    __asm__ __volatile__ (
        "mov (%[ptr], %[off]), %[val] \n\t"
        : [val] "=r" (val)
        : [ptr] "r" (ptr), [off] "r" (offset)
        : "memory"
    );
    
    return sum + val;
}

/* Main function that runs all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize register variables */
    reg_var1 = 0xABCD;
    reg_var2 = 0x1234;
    
    /* Run tests multiple times to increase reload opportunities */
    for (int i = 0; i < 10; i++) {
        checksum += test_many_operands();
        checksum += test_nested_calls();
        checksum += test_mixed_types();
        checksum += test_secondary_reloads();
        checksum += test_array_indexing();
        
        /* Modify globals to change dependencies */
        global_int += i;
        global_double += 0.1;
        global_char += 1;
    }
    
    /* Final assembly barrier */
    __asm__ __volatile__ ("" : : : "memory");
    
    /* Use checksum to prevent dead code elimination */
    return checksum & 0xFF;  /* Return lower bits to avoid overflow */
}
