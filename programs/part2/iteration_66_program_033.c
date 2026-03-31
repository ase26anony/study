/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation and dead code elimination */
volatile int global_seed = 42;

/* Function 1: High register pressure with multiple calls */
int __attribute__((noinline)) 
test_high_pressure(int x) {
    /* Many local variables, all live across calls */
    int a = x + 1;
    int b = x * 2;
    int c = x - 3;
    int d = x / 4;
    int e = x % 5;
    int f = x << 1;
    int g = x >> 2;
    int h = x | 0xFF;
    int i = x & 0x0F;
    int j = x ^ 0xAA;
    
    /* Force these to be in registers by taking addresses */
    int *ptr_a = &a;
    int *ptr_b = &b;
    
    /* First call that clobbers caller-saved regs */
    clobber_many_regs1();
    
    /* Use all variables after call */
    int sum1 = a + b + c + d;
    
    /* More variables to increase pressure */
    int k = sum1 * 2;
    int l = sum1 + 10;
    int m = sum1 - 5;
    int n = sum1 / 2;
    
    /* Second call */
    clobber_many_regs2();
    
    /* Complex use of variables */
    int sum2 = e + f + g + h + i + j + k + l + m + n;
    
    /* Third call in conditional */
    if (sum2 > 100) {
        clobber_many_regs3();
        sum2 += *ptr_a + *ptr_b;
    } else {
        clobber_many_regs1();
        sum2 -= *ptr_a - *ptr_b;
    }
    
    /* Final computation using all variables */
    return sum1 + sum2 + a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

/* Function 2: Nested loops with calls */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables in loop */
        int v1 = i * 2;
        int v2 = i + 1;
        int v3 = i - 1;
        int v4 = i << 1;
        int v5 = i >> 1;
        int v6 = i | 0x0F;
        int v7 = i & 0xF0;
        int v8 = i ^ 0x55;
        
        /* Call inside loop - variables must be preserved */
        clobber_many_regs1();
        
        /* Use variables after call */
        total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        
        /* Another call in inner conditional */
        if (i % 3 == 0) {
            clobber_many_regs2();
            total += v1 * v2;
        } else if (i % 3 == 1) {
            clobber_many_regs3();
            total += v3 - v4;
        }
        
        /* More computation */
        total += (v5 * v6) / (v7 + 1);
    }
    
    return total;
}

/* Function 3: Switch statement with calls in cases */
int __attribute__((noinline))
test_switch_pressure(int x) {
    int result = 0;
    
    switch (x % 4) {
        case 0: {
            int a = x + 10, b = x - 10, c = x * 2, d = x / 2;
            clobber_many_regs1();
            result = a + b + c + d;
            int e = result * 3, f = result + 5, g = result - 3;
            clobber_many_regs2();
            result += e + f + g;
            break;
        }
        case 1: {
            int h = x << 2, i = x >> 2, j = x | 0xAA, k = x & 0x55;
            clobber_many_regs3();
            result = h + i + j + k;
            int l = result * 4, m = result / 4, n = result % 4;
            clobber_many_regs1();
            result += l - m + n;
            break;
        }
        case 2: {
            int o = x ^ 0xFF, p = x + 100, q = x - 100, r = x * 3;
            clobber_many_regs2();
            result = o + p + q + r;
            int s = result << 1, t = result >> 1, u = result & 0x0F;
            clobber_many_regs3();
            result += s * t / (u + 1);
            break;
        }
        default: {
            int v = x + 50, w = x - 50, y = x * 4, z = x % 4;
            clobber_many_regs1();
            result = v + w + y + z;
            int aa = result + 100, bb = result - 100, cc = result * 2;
            clobber_many_regs2();
            clobber_many_regs3();
            result = aa * bb / (cc + 1);
            break;
        }
    }
    
    return result;
}

/* Function 4: Mixed caller/callee saved usage with address taking */
int __attribute__((noinline))
test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("r12") = x + 1;
    register long r2 asm("r13") = x * 2;
    register long r3 asm("r14") = x - 3;
    
    /* Many caller-saved candidates */
    int a = x + 10, b = x + 20, c = x + 30, d = x + 40;
    int e = x + 50, f = x + 60, g = x + 70, h = x + 80;
    
    /* Take addresses to force memory locations */
    int *ptrs[] = {&a, &b, &c, &d, &e, &f, &g, &h};
    
    /* Sequence of calls with computation in between */
    clobber_many_regs1();
    int sum1 = a + b + c + d + r1;
    
    clobber_many_regs2();
    int sum2 = e + f + g + h + r2;
    
    clobber_many_regs3();
    int sum3 = *ptrs[0] + *ptrs[1] + *ptrs[2] + *ptrs[3] + r3;
    
    clobber_many_regs1();
    int sum4 = *ptrs[4] + *ptrs[5] + *ptrs[6] + *ptrs[7];
    
    return sum1 + sum2 + sum3 + sum4;
}

/* Inline assembly to simulate register-clobbering functions */
void clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber caller-saved regs\n"
        :
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void clobber_many_regs2(void) {
    asm volatile (
        "# clobber different caller-saved regs\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void clobber_many_regs3(void) {
    asm volatile (
        "# clobber yet more registers\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary execution paths */
    int base = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Call all test functions to maximize coverage */
    result += test_high_pressure(base);
    result += test_loop_pressure(base % 10 + 5);
    result += test_switch_pressure(base);
    result += test_mixed_save(base);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
