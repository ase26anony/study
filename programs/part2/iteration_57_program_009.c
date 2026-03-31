/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int* global_ptr = &global_int;

/* Helper functions for nested calls */
int func1(void) { return global_int * 2; }
double func2(void) { return global_double * 2.0; }
int* func3(void) { return global_ptr; }
int func4(int x) { return x + global_int; }
double func5(double x) { return x + global_double; }

/* Test 1: Many operands with mixed constraints */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4;
    double d1 = 1.1, d2 = 2.2;
    char c1 = 'A', c2 = 'B';
    short s1 = 100, s2 = 200;
    long l1 = 1000, l2 = 2000;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "mov %[i3], %%ebx\n\t"
        "sub %[i4], %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        /* Force memory operations */
        "mov %[d1], %%ecx\n\t"
        "mov %[d2], %%edx\n\t"
        /* Mix in different sized registers */
        "mov %[c1], %%cl\n\t"
        "mov %[s1], %%dx\n\t"
        "mov %[l1], %%esi\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), 
          [o3] "=m" (out3), [o4] "=r" (out4)
        : [i1] "r" (in1), [i2] "r" (in2), 
          [i3] "i" (in3), [i4] "i" (in4),
          [d1] "r" (*(int*)&d1), [d2] "r" (*(int*)&d2),
          [c1] "r" ((int)c1), [s1] "r" ((int)s1),
          [l1] "r" (l1), [l2] "m" (l2),
          [c2] "i" ((int)c2), [s2] "i" ((int)s2)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2;
    double dresult;
    int* presult;
    
    /* Function calls as operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "mov %[f1], %%eax\n\t"
        "add %[f2], %%eax\n\t"
        "mov %%eax, %[r1]\n\t"
        "mov %[f3], %%ebx\n\t"
        "mov %%ebx, %[r2]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2),
          [dr] "=r" (*(int*)&dresult), [pr] "=r" (presult)
        : [f1] "r" (func1()), [f2] "r" (func4(10)),
          [f3] "r" (func3()), [f4] "r" (func2()),
          [f5] "r" (func5(1.5))
        : "eax", "ebx", "memory"
    );
    
    return result1 + result2 + (int)dresult + (int)(long)presult;
}

/* Test 3: Explicit register variables with complex addressing */
int test_explicit_registers(void) {
    register int r1 asm ("r12") = 100;
    register int r2 asm ("r13") = 200;
    register double dr asm ("r14") = 3.14;
    int out1, out2, out3;
    
    /* Force moves between explicit registers and regular ones */
    __asm__ __volatile__ (
        "mov %[reg1], %%eax\n\t"
        "add %[reg2], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "mov %[dreg], %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        /* Complex addressing with array and pointer arithmetic */
        "lea %[array], %%ecx\n\t"
        "add $128, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %%edx, %[o3]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3)
        : [reg1] "r" (r1), [reg2] "r" (r2), [dreg] "r" (*(int*)&dr),
          [array] "r" (global_array)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return out1 + out2 + out3;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c = 'X';
    short s = 1234;
    int i = 56789;
    long l = 987654321;
    float f = 2.71828f;
    double d = 1.41421;
    int out_int;
    double out_double;
    char out_char;
    
    /* Force mode changes through casts in asm operands */
    __asm__ __volatile__ (
        "mov %[cval], %%al\n\t"
        "movsx %%al, %%eax\n\t"
        "add %[ival], %%eax\n\t"
        "mov %%eax, %[oint]\n\t"
        /* Mix float and int in same operation */
        "mov %[fval], %%ebx\n\t"
        "mov %[dval], %%ecx\n\t"
        "add %%ebx, %%ecx\n\t"
        "mov %%ecx, %[odbl]\n\t"
        /* Use different parts of registers */
        "mov %[sval], %%dx\n\t"
        "mov %%dl, %[ochar]\n\t"
        : [oint] "=r" (out_int), [odbl] "=r" (*(int*)&out_double),
          [ochar] "=r" (out_char)
        : [cval] "r" ((int)c), [sval] "r" ((int)s),
          [ival] "r" (i), [lval] "r" (l),
          [fval] "r" (*(int*)&f), [dval] "r" (*(int*)&d)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return out_int + (int)out_double + (int)out_char;
}

/* Test 5: Secondary reload triggers with specific constraints */
int test_secondary_reloads(void) {
    int in1 = 100, in2 = 200;
    int out1, out2;
    double din = 3.14;
    double dout;
    
    /* Try to force accumulator-specific constraints */
    __asm__ __volatile__ (
        /* Force use of accumulator */
        "mov %[in1], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Force memory operand that needs reloading */
        "mov %[din], %%ebx\n\t"
        "add $0x1000, %%ebx\n\t"
        "mov %%ebx, %[dout]\n\t"
        /* Complex constraint combination */
        "imul %[in2], %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        : [out1] "=a" (out1), [out2] "=r" (out2),
          [dout] "=r" (*(int*)&dout)
        : [in1] "r" (in1), [in2] "i" (in2),
          [din] "m" (*(int*)&din)
        : "eax", "ebx", "memory"
    );
    
    return out1 + out2 + (int)dout;
}

/* Test 6: Chain of volatile asm blocks with interdependencies */
int test_chain_reloads(void) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int t1, t2, t3, t4;
    
    /* Chain 1: Compute and immediately use result */
    __asm__ __volatile__ (
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "mov %%eax, %[r1]\n\t"
        : [r1] "=r" (t1)
        : [a] "r" (v1), [b] "r" (v2)
        : "eax", "memory"
    );
    
    /* Chain 2: Use previous result */
    __asm__ __volatile__ (
        "mov %[prev], %%ebx\n\t"
        "add %[c], %%ebx\n\t"
        "mov %%ebx, %[r2]\n\t"
        : [r2] "=r" (t2)
        : [prev] "r" (t1), [c] "r" (v3)
        : "ebx", "memory"
    );
    
    /* Chain 3: More complex with multiple dependencies */
    __asm__ __volatile__ (
        "mov %[p1], %%ecx\n\t"
        "add %[p2], %%ecx\n\t"
        "add %[d], %%ecx\n\t"
        "mov %%ecx, %[r3]\n\t"
        : [r3] "=r" (t3)
        : [p1] "r" (t1), [p2] "r" (t2), [d] "r" (v4)
        : "ecx", "memory"
    );
    
    /* Chain 4: Final computation with all intermediates */
    __asm__ __volatile__ (
        "mov %[x1], %%edx\n\t"
        "add %[x2], %%edx\n\t"
        "add %[x3], %%edx\n\t"
        "mov %%edx, %[r4]\n\t"
        : [r4] "=r" (t4)
        : [x1] "r" (t1), [x2] "r" (t2), [x3] "r" (t3)
        : "edx", "memory"
    );
    
    return t1 + t2 + t3 + t4;
}

/* Test 7: Array indexing with non-constant offsets */
int test_array_indexing(void) {
    int array[100];
    int i, j, k;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Complex addressing in asm */
    for (i = 0; i < 10; i++) {
        int idx1 = func1() % 50;
        int idx2 = func4(i) % 50;
        int temp;
        
        __asm__ __volatile__ (
            "mov %[arr], %%esi\n\t"
            "mov %[idx1], %%eax\n\t"
            "shl $2, %%eax\n\t"
            "add %%esi, %%eax\n\t"
            "mov (%[eax]), %%ebx\n\t"
            "mov %[idx2], %%ecx\n\t"
            "shl $2, %%ecx\n\t"
            "add %%esi, %%ecx\n\t"
            "mov (%[ecx]), %%edx\n\t"
            "add %%ebx, %%edx\n\t"
            "mov %%edx, %[out]\n\t"
            : [out] "=r" (temp)
            : [arr] "r" (array), [idx1] "r" (idx1), [idx2] "r" (idx2)
            : "eax", "ebx", "ecx", "edx", "esi", "memory"
        );
        
        sum += temp;
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 128);
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_chain_reloads();
    checksum += test_array_indexing();
    
    printf("Checksum: %d\n", checksum);
    
    /* Return deterministic checksum */
    return checksum & 0xFF;  /* Return lower 8 bits */
}
