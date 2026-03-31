/* test_early_remat.c - Designed to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global arrays for address calculations - ensure they're not optimized away */
volatile int global_array1[256] = {0};
volatile long global_array2[512] = {0};
volatile double global_array3[128] = {0.0};

/* Function A: Loop with invariants and large immediate constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int start, int end, volatile int* data) {
    /* Large immediate constants that can't be encoded in single instructions */
    const long LARGE_CONST1 = 0x7FFFFFFF12345678L;
    const long LARGE_CONST2 = 0xFFFFFFFF87654321L;
    const unsigned long HUGE_ADDR = 0xDEADBEEFCAFEBABEUL;
    
    /* Many local variables with overlapping live ranges */
    register int r1 asm("ebx") = start;
    register int r2 asm("esi") = end;
    register int r3 asm("edi") = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    long addr_temp1, addr_temp2, addr_temp3;
    double fp_temp1, fp_temp2;
    
    /* Loop with invariant address calculations */
    for (int i = start; i < end; i += 2) {
        /* Use invariants in multiple places with different operations */
        addr_temp1 = (long)(&global_array1[i]) + LARGE_CONST1;
        addr_temp2 = (long)(&global_array2[i * 2]) + LARGE_CONST2;
        
        /* Complex address calculations using invariants */
        temp1 = *(int*)(addr_temp1 & 0x7FFFFFFF);
        temp2 = *(int*)(addr_temp2 & 0x7FFFFFFF);
        
        /* More operations creating register pressure */
        temp3 = temp1 * r1 + (int)(HUGE_ADDR >> 32);
        temp4 = temp2 * r2 - (int)(HUGE_ADDR & 0xFFFFFFFF);
        temp5 = temp3 ^ temp4;
        temp6 = temp5 | (int)LARGE_CONST1;
        temp7 = temp6 & (int)LARGE_CONST2;
        temp8 = temp7 << 3;
        
        /* Use floating point to add more register pressure */
        fp_temp1 = global_array3[i % 128] * 3.141592653589793;
        fp_temp2 = fp_temp1 + (double)temp8;
        
        /* Store results creating anti-dependencies */
        data[i] = temp8 + (int)fp_temp2;
        r3 += data[i];
        
        /* Additional invariant use in condition */
        if (i % 16 == (int)(LARGE_CONST1 & 0xF)) {
            temp1 = r1 * r2;
            r1 = (r1 + 1) & 0xFF;
        }
    }
    
    /* Mix all results to prevent dead code elimination */
    return r3 + temp1 + temp2 + (int)fp_temp1;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
long func_asm_clobber(int a, int b, long c) {
    /* Declare many register variables */
    register long reg1 asm("eax");
    register long reg2 asm("ebx");
    register long reg3 asm("ecx");
    register long reg4 asm("edx");
    register long reg5 asm("esi");
    register long reg6 asm("edi");
    
    long result1, result2, result3, result4;
    
    /* Complex inline assembly with multiple outputs and clobbers */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "movl %[input2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl $0x12345678, %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "rdtsc\n\t"  /* Uses eax and edx hard registers */
        "movl %%eax, %[out3]\n\t"
        "movl %%edx, %[out4]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [input1] "r" (a),
          [input2] "r" (b)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use the results in complex expressions */
    reg1 = result1 + c;
    reg2 = result2 * reg1;
    reg3 = result3 ^ reg2;
    reg4 = result4 | reg3;
    reg5 = reg4 << 4;
    reg6 = reg5 >> 2;
    
    /* More inline assembly creating hard register references */
    asm volatile (
        "cpuid\n\t"
        : "=a" (reg1), "=b" (reg2), "=c" (reg3), "=d" (reg4)
        : "a" (0)
        : "memory"
    );
    
    /* Chain hard register references through computations */
    result1 = reg1 + reg2;
    result2 = reg3 * reg4;
    result3 = result1 ^ result2;
    result4 = result3 + reg5 + reg6;
    
    /* Use computed goto to create complex control flow */
    static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    
    goto *labels[result4 & 0x3];
    
label1:
    return result1 + a;
label2:
    return result2 + b;
label3:
    return result3 + c;
label4:
    return result4 + result1 + result2;
}

/* Function C: Complex control flow with switch statements */
__attribute__((noinline, noclone))
double func_complex_control(int iterations, double base) {
    /* Many temporary variables with overlapping lifetimes */
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5;
    
    /* Initialize with values that create register pressure */
    t1 = base * 1.1;
    t2 = base * 2.2;
    t3 = base * 3.3;
    t4 = base * 4.4;
    t5 = base * 5.5;
    
    i1 = (int)(t1 * 100);
    i2 = (int)(t2 * 100);
    i3 = (int)(t3 * 100);
    i4 = (int)(t4 * 100);
    i5 = (int)(t5 * 100);
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < iterations; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            /* Complex switch with many cases */
            switch ((i1 + i2 + outer + inner) % 8) {
                case 0:
                    t6 = t1 + t2;
                    i6 = i1 * i2;
                    l1 = (long)i6 * 0x100000000L;
                    break;
                case 1:
                    t7 = t2 - t3;
                    i7 = i2 / (i3 ? i3 : 1);
                    l2 = (long)i7 + 0x7FFFFFFF;
                    break;
                case 2:
                    t8 = t3 * t4;
                    i8 = i3 ^ i4;
                    l3 = (long)i8 | 0x12345678;
                    break;
                case 3:
                    t9 = t4 / (t5 ? t5 : 1.0);
                    i9 = i4 & i5;
                    l4 = (long)i9 & 0xFEDCBA98;
                    break;
                case 4:
                    t10 = t5 + t1;
                    i10 = i5 | i1;
                    l5 = (long)i10 << 16;
                    break;
                case 5:
                    t1 = t6 * 0.9;
                    i1 = i6 + 1;
                    break;
                case 6:
                    t2 = t7 * 1.1;
                    i2 = i7 - 1;
                    break;
                case 7:
                    t3 = t8 * 0.8;
                    i3 = i8 ^ 0xFF;
                    break;
            }
            
            /* More operations keeping variables live */
            if (inner % 3 == 0) {
                t4 = t9 + (double)l1;
                i4 = i9 + (int)(l2 & 0xFFFF);
            } else if (inner % 3 == 1) {
                t5 = t10 - (double)l3;
                i5 = i10 - (int)(l4 & 0xFFFF);
            } else {
                t6 = (double)l5 * 0.001;
                i6 = (int)((l1 + l2) >> 32);
            }
        }
        
        /* Cross-iteration dependencies */
        t7 = t1 + t2 + t3 + t4 + t5;
        t8 = t6 * t7 * (double)outer;
        
        /* Use global array with large offset */
        global_array3[outer % 128] = t8;
    }
    
    /* Combine all results */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           (double)i1 + (double)i2 + (double)i3 + (double)i4 + (double)i5 +
           (double)l1 + (double)l2 + (double)l3 + (double)l4 + (double)l5;
}

/* Main function that calls all test functions */
int main(int argc, char** argv) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        global_array1[i] = i * 3;
    }
    
    for (int i = 0; i < 512; i++) {
        global_array2[i] = i * 5L;
    }
    
    for (int i = 0; i < 128; i++) {
        global_array3[i] = (double)i * 0.1;
    }
    
    /* Call all test functions with arguments that create register pressure */
    int result1 = func_loop_invariants(0, 100, (int*)global_array1);
    long result2 = func_asm_clobber(argc, result1, 0x7FFFFFFFFFFFFFFFL);
    double result3 = func_complex_control(50, (double)result2 * 0.001);
    
    /* Combine results to prevent optimization */
    int final_result = (int)result3 + (int)result2 + result1;
    
    /* Use results in system call to prevent dead code elimination */
    if (argc > 1) {
        final_result += atoi(argv[1]);
    }
    
    return final_result & 0xFF;  /* Return non-zero result */
}
