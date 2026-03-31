/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use asm to clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
    /* Many local variables that must be kept alive across calls */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    int i = seed + 9;
    int j = seed + 10;
    int k = seed + 11;
    int l = seed + 12;
    int m = seed + 13;
    int n = seed + 14;
    int o = seed + 15;
    int p = seed + 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different register pressure pattern */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h + i - j + k - l + m - n + o - p;
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    /* Many variables to create register pressure */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    if (flag > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables to keep them live */
        v1 = v1 + v2;
        v3 = v3 + v4;
        
        /* Another call */
        clobber_many_regs_2();
        
        v5 = v5 + v6;
        v7 = v7 + v8;
    } else {
        /* Different pattern in false branch */
        clobber_many_regs_3();
        
        v2 = v2 + v3;
        v4 = v4 + v5;
        
        clobber_many_regs_1();
        
        v6 = v6 + v7;
        v8 = v8 + v9;
    }
    
    /* Common code with more register pressure */
    clobber_many_regs_2();
    
    /* Force all variables to be used */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with mixed caller/callee saved usage */
int __attribute__((noinline)) test_mixed_save(int seed) {
    /* Variables that might use callee-saved registers */
    register long r12_val asm("r12") = seed + 100;
    register long r13_val asm("r13") = seed + 200;
    register long r14_val asm("r14") = seed + 300;
    register long r15_val asm("r15") = seed + 400;
    
    /* Many temporary variables for caller-saved pressure */
    int t1 = seed + 1;
    int t2 = seed + 2;
    int t3 = seed + 3;
    int t4 = seed + 4;
    int t5 = seed + 5;
    int t6 = seed + 6;
    int t7 = seed + 7;
    int t8 = seed + 8;
    int t9 = seed + 9;
    int t10 = seed + 10;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types of variables */
    int temp_sum = t1 + t2 + t3 + t4 + t5;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    long callee_sum = r12_val + r13_val + r14_val + r15_val;
    
    /* Final call */
    clobber_many_regs_3();
    
    return temp_sum + (int)callee_sum + t6 + t7 + t8 + t9 + t10;
}

/* Function with loop creating multiple basic blocks */
int __attribute__((noinline)) test_loop_pressure(int seed, int iterations) {
    int result = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int a = result + i * 1;
        int b = result + i * 2;
        int c = result + i * 3;
        int d = result + i * 4;
        int e = result + i * 5;
        int f = result + i * 6;
        int g = result + i * 7;
        int h = result + i * 8;
        
        /* Call inside loop - creates high pressure */
        clobber_many_regs_1();
        
        /* Use variables to keep them live */
        a = a + b;
        c = c + d;
        e = e + f;
        g = g + h;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More computations */
        result = a + c + e + g + result;
    }
    
    return result;
}

/* Function that takes addresses to inhibit optimizations */
int __attribute__((noinline)) test_address_taken(int seed) {
    int var1 = seed + 1;
    int var2 = seed + 2;
    int var3 = seed + 3;
    int var4 = seed + 4;
    int var5 = seed + 5;
    int var6 = seed + 6;
    int var7 = seed + 7;
    int var8 = seed + 8;
    
    /* Take addresses to force stack/memory usage */
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    int *ptr3 = &var3;
    int *ptr4 = &var4;
    
    /* Call between address taking and use */
    clobber_many_regs_1();
    
    /* Use through pointers */
    *ptr1 = *ptr1 + *ptr2;
    *ptr3 = *ptr3 + *ptr4;
    
    /* More calls */
    clobber_many_regs_2();
    
    /* Take more addresses */
    int *ptr5 = &var5;
    int *ptr6 = &var6;
    int *ptr7 = &var7;
    int *ptr8 = &var8;
    
    clobber_many_regs_3();
    
    /* Final computation */
    return *ptr1 + *ptr3 + *ptr5 + *ptr6 + *ptr7 + *ptr8 + var1 + var2;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line to vary behavior and prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Run all test functions to create various register pressure scenarios */
    result += test_high_pressure(seed);
    result += test_control_flow(seed, seed % 2);
    result += test_mixed_save(seed);
    result += test_loop_pressure(seed, 3);  /* Small iteration count */
    result += test_address_taken(seed);
    
    /* Also test with different seeds */
    if (argc > 2) {
        int seed2 = atoi(argv[2]);
        result += test_high_pressure(seed2);
        result += test_control_flow(seed2, seed2 % 3);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;  /* Return non-zero to indicate success */
}
