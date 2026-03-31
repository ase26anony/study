/* test-early-remat.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Target: early-remat.cc lines 930-937
 */

#include <stdint.h>
#include <stdio.h>

/* Global arrays to create address calculation opportunities */
static int global_array1[1024];
static int global_array2[1024];
static int global_array3[1024];
static double global_double_array[1024];

/* Large immediate constants that are expensive to materialize */
#define LARGE_CONST1 0x12345678
#define LARGE_CONST2 0x9ABCDEF0
#define LARGE_CONST3 0x55555555
#define LARGE_CONST4 0xAAAAAAAA

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* restrict out) {
    /* Many local variables with overlapping live ranges */
    register int a1 asm("eax") = LARGE_CONST1;
    register int a2 asm("ebx") = LARGE_CONST2;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    
    /* Loop invariants - these should be rematerialized */
    const int* invariant_ptr1 = &global_array1[512];
    const int* invariant_ptr2 = &global_array2[256];
    const long invariant_const = 0x7FFFFFFFFFFFFFFF;
    
    /* Complex loop with many live values */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        b1 = *invariant_ptr1 + i;
        b2 = *invariant_ptr2 - i;
        
        /* Use expensive constants repeatedly */
        b3 = a1 * b1 + LARGE_CONST3;
        b4 = a2 * b2 + LARGE_CONST4;
        
        /* More operations creating register pressure */
        b5 = b1 * b3 + (int)(invariant_const >> 32);
        b6 = b2 * b4 + (int)(invariant_const & 0xFFFFFFFF);
        
        /* Cross-dependent calculations */
        b7 = b5 * b6 + *invariant_ptr1;
        b8 = b6 * b7 + *invariant_ptr2;
        
        /* Use all variables to keep them live */
        c1 = b1 + b3 + b5 + b7;
        c2 = b2 + b4 + b6 + b8;
        c3 = c1 * c2 + a1;
        c4 = c2 * c3 + a2;
        
        /* Store results, creating more live ranges */
        out[i] = c3 + c4 + (int)invariant_const;
        
        /* More operations to increase pressure */
        c5 = c3 * c4 + LARGE_CONST1;
        c6 = c4 * c5 + LARGE_CONST2;
        c7 = c5 * c6 + LARGE_CONST3;
        c8 = c6 * c7 + LARGE_CONST4;
        c9 = c7 * c8;
        c10 = c8 * c9;
        
        /* Use results to prevent optimization */
        a1 = (a1 + c9) & 0xFFFF;
        a2 = (a2 + c10) & 0xFFFF;
    }
    
    return a1 + a2 + b1 + b2 + c1 + c2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    int result1, result2, result3;
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result1)
        : "r" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More operations creating register pressure */
    register int r1 asm("eax") = result1;
    register int r2 asm("ebx") = LARGE_CONST1;
    register int r3 asm("ecx") = LARGE_CONST2;
    
    /* Complex calculations with register variables */
    for (int i = 0; i < 100; i++) {
        /* Use rdtsc which uses specific hard registers */
        uint64_t tsc;
        asm volatile ("rdtsc" : "=a" (tsc), "=d" (tsc >> 32));
        
        /* Mix results with register variables */
        r1 = (r1 * (tsc & 0xFF)) + r2;
        r2 = (r2 * ((tsc >> 8) & 0xFF)) + r3;
        r3 = (r3 * ((tsc >> 16) & 0xFF)) + r1;
        
        /* Multi-output inline assembly */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movl %2, %%ebx\n\t"
            "imull %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %3\n\t"
            : "=r" (result2), "=r" (result3)
            : "r" (r1), "r" (r2)
            : "eax", "ebx", "edx"
        );
        
        r1 = result2;
        r2 = result3;
    }
    
    return r1 + r2 + r3;
}

/* Function C: Complex control flow with switch statements */
__attribute__((noinline, noclone))
int function_c(int mode, int iterations) {
    /* Many temporary variables with overlapping lives */
    int t1 = LARGE_CONST1, t2 = LARGE_CONST2, t3 = LARGE_CONST3, t4 = LARGE_CONST4;
    int t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16;
    
    /* Use computed goto for complex control flow */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Switch inside loop creates complex live ranges */
        switch (mode) {
            case 0:
                t5 = t1 * t2 + global_array1[i & 1023];
                t6 = t3 * t4 + global_array2[i & 1023];
                t7 = t5 * t6;
                goto *labels[i % 5];
                
            label0:
                t8 = t7 + t1;
                t9 = t8 * t2;
                break;
                
            case 1:
                t10 = t2 * t3 + global_array3[i & 1023];
                t11 = t4 * t1 + global_double_array[i & 1023];
                t12 = t10 * t11;
                goto *labels[(i + 1) % 5];
                
            label1:
                t13 = t12 + t3;
                t14 = t13 * t4;
                break;
                
            case 2:
                t15 = t3 * t4 + (int)(0x7FFFFFFFFFFFFFFF >> (i & 31));
                t16 = t1 * t2 + (int)(0xFFFFFFFFFFFFFFFF >> (i & 31));
                goto *labels[(i + 2) % 5];
                
            label2:
                t5 = t15 * t16;
                t6 = t5 + t1;
                break;
                
            default:
                goto *labels[(i + 3) % 5];
        }
        
        label3:
        t7 = t9 ? t9 : t14;
        t8 = t7 * (i + 1);
        
        label4:
        sum += t8 + t6 + t12 + t16;
        
        /* Rotate values to keep them all live */
        int temp = t1;
        t1 = t2; t2 = t3; t3 = t4; t4 = temp;
        if (t5) { temp = t5; t5 = t6; t6 = t7; t7 = temp; }
        if (t9) { temp = t9; t9 = t10; t10 = t11; t11 = temp; }
        if (t13) { temp = t13; t13 = t14; t14 = t15; t15 = temp; }
        
        /* Change mode periodically */
        if (i % 100 == 0) mode = (mode + 1) % 3;
    }
    
    return sum;
}

/* Function D: Nested loops with address calculations */
__attribute__((noinline, noclone))
int function_d(int size) {
    /* Complex addressing with large offsets */
    int* ptr1 = global_array1 + 256;
    int* ptr2 = global_array2 + 512;
    int* ptr3 = global_array3 + 768;
    
    int sum = 0;
    
    /* Nested loops create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Loop invariants used in inner loop */
        const long base_addr = (long)ptr1;
        const int large_offset = LARGE_CONST1;
        
        for (int j = 0; j < 16; j++) {
            /* Expensive address calculations */
            int* addr1 = (int*)(base_addr + (i * j * 4));
            int* addr2 = ptr2 + (i * 4) + (j * 16);
            int* addr3 = ptr3 + ((i + j) * 8);
            
            /* Use all addresses to keep them live */
            int val1 = *addr1 + large_offset;
            int val2 = *addr2 + (large_offset >> 16);
            int val3 = *addr3 + (large_offset & 0xFFFF);
            
            /* More calculations creating register pressure */
            int tmp1 = val1 * val2 + (int)(base_addr >> 32);
            int tmp2 = val2 * val3 + (int)(base_addr & 0xFFFFFFFF);
            int tmp3 = val3 * val1 + large_offset;
            
            /* Cross-dependent operations */
            for (int k = 0; k < 4; k++) {
                tmp1 = tmp1 * tmp2 + k;
                tmp2 = tmp2 * tmp3 + (k * large_offset);
                tmp3 = tmp3 * tmp1 + (k * (large_offset >> 8));
            }
            
            sum += tmp1 + tmp2 + tmp3;
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array1[i] = i;
        global_array2[i] = i * 2;
        global_array3[i] = i * 3;
        global_double_array[i] = i * 1.5;
    }
    
    int result = 0;
    int iterations = (argc > 1) ? 1000 : 100;
    
    /* Call all test functions to trigger different patterns */
    result += function_a(iterations, global_array1);
    result += function_b(LARGE_CONST1, LARGE_CONST2);
    result += function_c(argc, iterations);
    result += function_d(50);
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
