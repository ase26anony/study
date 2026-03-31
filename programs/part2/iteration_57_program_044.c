/* reload_stress_test.c
 * 
 * This program is designed to stress GCC's reload mechanism by creating
 * complex inline assembly scenarios that force the register allocator
 * to generate many reloads, particularly targeting the push_reload
 * function's initialization block for new reload entries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies and prevent optimization */
int global_int = 42;
double global_double = 3.14159;
char global_char_array[256];
int *global_ptr = &global_int;

/* Helper functions to use in assembly operands */
int get_random_value(void) {
    return rand() % 100;
}

double compute_double(int x) {
    return x * 1.5;
}

int* get_pointer(void) {
    return &global_int;
}

/* Test 1: Many operands with mixed constraints to exhaust registers */
void test_many_operands(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int out1, out2, out3, out4, out5;
    double din1 = 1.1, din2 = 2.2;
    double dout1, dout2;
    
    /* Use explicit register variables to force specific register allocation */
    register int r12_var asm ("r12") = 100;
    register int r13_var asm ("r13") = 200;
    
    /* Complex assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in3], %[out2]\n\t"
        "imul %[in4], %[out2]\n\t"
        /* Use explicit register variables */
        "add %%r12, %[out1]\n\t"
        "add %%r13, %[out2]\n\t"
        /* Memory operand */
        "mov (%[mem]), %[out3]\n\t"
        /* Immediate operand */
        "mov $999, %[out4]\n\t"
        /* Complex addressing mode */
        "mov (%[ptr],%[idx],4), %[out5]"
        
        : [out1] "=r" (out1), 
          [out2] "=&r" (out2),  /* earlyclobber */
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "0" (in3),      /* matching constraint */
          [in4] "rm" (in4),     /* register or memory */
          [mem] "r" (global_ptr),
          [ptr] "r" (global_char_array),
          [idx] "r" (global_int),
          "r" (r12_var),        /* explicit register input */
          "r" (r13_var)
        : "memory", "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    global_int += out1 + out2 + out3 + out4 + out5;
}

/* Test 2: Nested function calls in assembly operands */
void test_nested_calls(void) {
    int result1, result2;
    double dresult;
    
    /* Function calls as operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "add %%eax, %%ebx\n\t"
        "mov %%ebx, %[res1]\n\t"
        "cvtsi2sd %%ebx, %%xmm0\n\t"
        "movsd %%xmm0, %[dres]"
        
        : [res1] "=r" (result1),
          [dres] "=m" (dresult)
        : "a" (get_random_value()),      /* function result in eax */
          "b" (get_random_value())       /* another function result in ebx */
        : "xmm0", "memory", "cc"
    );
    
    /* More complex: pointer from function call */
    int* ptr = get_pointer();
    int idx = get_random_value();
    
    __asm__ __volatile__ (
        "mov (%[ptr],%[idx],4), %%eax\n\t"
        "add $1, %%eax"
        
        : "=a" (result2)
        : [ptr] "r" (ptr),
          [idx] "r" (idx)
        : "memory"
    );
    
    global_int += result1 + result2 + (int)dresult;
}

/* Test 3: Mixed data types and mode changes */
void test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long long ll1 = 10000000000LL, ll2 = 20000000000LL;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    int out_int;
    char out_char;
    double out_double;
    
    /* Mixing types in same assembly - forces mode conversions */
    __asm__ __volatile__ (
        /* char to int extension */
        "movsbl %[c1], %%eax\n\t"
        /* short to int extension */
        "movswl %[s1], %%ebx\n\t"
        /* float to double conversion */
        "cvtss2sd %[f1], %%xmm0\n\t"
        /* Integer arithmetic */
        "add %%ebx, %%eax\n\t"
        "add %[i1], %%eax\n\t"
        /* Store results */
        "mov %%al, %[outc]\n\t"
        "mov %%eax, %[outi]\n\t"
        "movsd %%xmm0, %[outd]"
        
        : [outi] "=r" (out_int),
          [outc] "=m" (out_char),
          [outd] "=m" (out_double)
        : [c1] "r" ((int)c1),    /* cast forces different mode */
          [s1] "r" ((int)s1),
          [i1] "r" (i1),
          [f1] "x" (f1)
        : "eax", "ebx", "xmm0", "memory", "cc"
    );
    
    /* Double to int truncation */
    int truncated;
    __asm__ __volatile__ (
        "cvtsd2si %[d1], %%eax"
        
        : "=a" (truncated)
        : [d1] "x" (d1)
        : "cc"
    );
    
    global_int += out_int + truncated + (int)out_double;
}

/* Test 4: Secondary reload triggers with specific constraints */
void test_secondary_reloads(void) {
    int value = 12345;
    int result;
    
    /* Try to force a secondary reload by using difficult constraints */
    
    /* Using 'a' constraint (accumulator) when value might not be there */
    __asm__ __volatile__ (
        "add $1, %%eax"
        
        : "=a" (result)
        : "a" (value),          /* Forces move to eax if not already there */
          "m" (global_int)      /* Memory operand adds pressure */
        : "cc"
    );
    
    /* Multiple constraints that are hard to satisfy together */
    double dvalue = 123.456;
    long long llresult;
    
    __asm__ __volatile__ (
        "cvtsd2si %[dval], %%rax\n\t"
        "imul $2, %%rax"
        
        : "=a" (llresult)
        : [dval] "x" (dvalue),   /* XMM register */
          "b" (get_random_value()) /* Another function call result */
        : "cc"
    );
    
    global_int += result + (int)llresult;
}

/* Test 5: Volatile chains with interdependent operands */
void test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x, y, z;
    
    /* Chain of volatile assembly blocks */
    __asm__ __volatile__ (
        "mov %[a], %[x]\n\t"
        "add %[b], %[x]"
        
        : [x] "=r" (x)
        : [a] "r" (a),
          [b] "r" (b)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[c], %[y]\n\t"
        "imul %[x], %[y]"
        
        : [y] "=r" (y)
        : [c] "r" (c),
          [x] "r" (x)    /* Depends on previous result */
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[d], %[z]\n\t"
        "add %[e], %[z]\n\t"
        "add %[y], %[z]"
        
        : [z] "=r" (z)
        : [d] "r" (d),
          [e] "r" (e),
          [y] "r" (y)    /* Depends on previous result */
        : "cc"
    );
    
    /* Memory clobber between operations */
    __asm__ __volatile__ (
        "movl $0x12345678, (%0)"
        
        :
        : "r" (&global_int)
        : "memory"
    );
    
    global_int += x + y + z;
}

/* Test 6: Complex addressing modes with non-constant offsets */
void test_complex_addressing(void) {
    int array[100];
    int *ptr = array;
    int i, j, k;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    int result1, result2;
    
    /* Complex addressing with computation */
    __asm__ __volatile__ (
        /* array[global_int * 2] */
        "mov (%[arr],%[idx],8), %[res1]\n\t"
        /* array[get_random_value() % 100] - function call in index */
        "mov %[idx2], %%ecx\n\t"
        "mov (%[arr],%%rcx,4), %[res2]"
        
        : [res1] "=r" (result1),
          [res2] "=r" (result2)
        : [arr] "r" (array),
          [idx] "r" (global_int),      /* Non-constant index */
          [idx2] "r" (get_random_value() % 100) /* Function call as index */
        : "ecx", "memory"
    );
    
    /* Pointer arithmetic in operand */
    int offset = get_random_value();
    __asm__ __volatile__ (
        "mov (%[ptr],%[off],4), %%eax"
        
        : "=a" (k)
        : [ptr] "r" (ptr),
          [off] "r" (offset)
        : "memory"
    );
    
    global_int += result1 + result2 + k;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = i;
    }
    
    /* Run all tests multiple times to increase reload opportunities */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_many_operands();
        test_nested_calls();
        test_mixed_types();
        test_secondary_reloads();
        test_volatile_chains();
        test_complex_addressing();
        
        checksum += global_int;
    }
    
    /* Final computation using results */
    checksum += (int)global_double;
    
    printf("Checksum: %d\n", checksum);
    
    /* Return deterministic value for testing */
    return checksum % 256;
}
