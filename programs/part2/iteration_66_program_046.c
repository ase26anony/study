/* caller_save_test.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_x86(void) {
    /* Use asm to clobber many caller-saved registers */
    asm volatile (
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x11111111, %%rdx\n\t"
        "mov $0x22222222, %%rsi\n\t"
        "mov $0x33333333, %%rdi\n\t"
        "mov $0x44444444, %%r8\n\t"
        "mov $0x55555555, %%r9\n\t"
        "mov $0x66666666, %%r10\n\t"
        "mov $0x77777777, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
}

void __attribute__((noinline)) clobber_many_regs_arm(void) {
    /* ARM version - will be ignored on x86 but provides alternative */
    asm volatile (
        "mov r0, #0x12345678\n\t"
        "mov r1, #0x87654321\n\t"
        "mov r2, #0x11111111\n\t"
        "mov r3, #0x22222222\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "memory"
    );
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - a + 4;
    int e = d * b + 5;
    int f = e - c + 6;
    int g = f * a + 7;
    int h = g - d + 8;
    int i = h * e + 9;
    int j = i - f + 10;
    int k = j * g + 11;
    int l = k - h + 12;
    int m = l * i + 13;
    int n = m - j + 14;
    int o = n * k + 15;
    int p = o - l + 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_x86();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* More variables to increase pressure */
    int q = sum1 * m + 17;
    int r = q - n + 18;
    int s = r * o + 19;
    int t = s - p + 20;
    
    /* Second call with different register pressure */
    clobber_many_regs_x86();
    
    /* Use variables across second call */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call in conditional path */
    if (x > y) {
        clobber_many_regs_x86();
        sum2 += q + r + s + t;
    } else {
        /* Alternative path with different usage */
        sum2 -= q - r + s - t;
    }
    
    return sum1 + sum2 + a - b + c - d + e - f + g - h + i - j;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int seed) {
    volatile int v1 = seed * 3;
    volatile int v2 = seed * 5;
    volatile int v3 = seed * 7;
    volatile int v4 = seed * 11;
    volatile int v5 = seed * 13;
    volatile int v6 = seed * 17;
    volatile int v7 = seed * 19;
    volatile int v8 = seed * 23;
    
    int result = 0;
    
    /* Loop with calls inside */
    for (int i = 0; i < 3; i++) {
        v1 += i;
        v2 += i * 2;
        
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_x86();
        
        v3 += v1;
        v4 += v2;
        
        /* Conditional call */
        if (i % 2 == 0) {
            clobber_many_regs_x86();
            v5 += v3;
        } else {
            v6 += v4;
        }
        
        /* Use all variables to keep them live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    }
    
    /* Switch statement with calls */
    switch (seed % 4) {
        case 0:
            clobber_many_regs_x86();
            result += v1 * 2;
            break;
        case 1:
            result += v2 * 3;
            clobber_many_regs_x86();
            break;
        case 2:
            clobber_many_regs_x86();
            result += v3 * 4;
            clobber_many_regs_x86();
            break;
        default:
            result += v4 * 5;
            break;
    }
    
    return result;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_register_usage(int* ptr) {
    /* Force variables to different registers by taking addresses */
    int var1 = 100;
    int var2 = 200;
    int var3 = 300;
    int var4 = 400;
    int var5 = 500;
    int var6 = 600;
    int var7 = 700;
    int var8 = 800;
    
    /* Use addresses to inhibit optimizations */
    int* p1 = &var1;
    int* p2 = &var2;
    int* p3 = &var3;
    int* p4 = &var4;
    
    /* Call that clobbers registers */
    clobber_many_regs_x86();
    
    /* Complex computation using all variables */
    int sum = (*p1 + var2) * (var3 - *p2);
    sum += (var4 + *p3) / (var5 - *p4);
    
    /* Another call */
    clobber_many_regs_x86();
    
    /* More computations keeping variables live */
    sum += var6 * var7 - var8;
    sum += (*p1)++ + (*p2)--;
    
    /* Store through pointer parameter (forces register usage) */
    *ptr = sum;
    
    return sum + var1 + var2 + var3 + var4;
}

/* Function with nested calls and register pressure */
int __attribute__((noinline)) test_nested_pressure(int a, int b, int c, int d,
                                                   int e, int f, int g, int h) {
    /* All parameters are in registers initially */
    int t1 = a * b + c * d;
    int t2 = e * f + g * h;
    int t3 = a + b + c + d;
    int t4 = e + f + g + h;
    
    /* Call sequence with live variables */
    clobber_many_regs_x86();
    int u1 = t1 + t2;
    
    clobber_many_regs_x86();
    int u2 = t3 + t4;
    
    clobber_many_regs_x86();
    int u3 = u1 * u2;
    
    /* Use all intermediate values */
    return u3 + t1 - t2 + t3 - t4 + u1 - u2 + a - b + c - d + e - f + g - h;
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use argc to vary paths and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing caller-save optimization patterns...\n");
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed * 2);
    
    /* Test 2: Control flow variations */
    result += test_control_flow(seed);
    
    /* Test 3: Mixed register usage */
    int ptr_val = 0;
    result += test_mixed_register_usage(&ptr_val);
    result += ptr_val;
    
    /* Test 4: Many parameters (register pressure) */
    result += test_nested_pressure(seed, seed+1, seed+2, seed+3,
                                   seed+4, seed+5, seed+6, seed+7);
    
    /* Additional test with varying call patterns based on input */
    for (int i = 0; i < (argc > 2 ? atoi(argv[2]) : 2); i++) {
        clobber_many_regs_x86();
        result += i * seed;
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1; /* Return 0 on success */
}
