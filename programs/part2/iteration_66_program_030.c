/* test_caller_save.c
 * Designed to trigger instruction reordering in GCC's caller-save pass
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must stay live across calls */
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
    int q = p * m + 17;
    int r = q - n + 18;
    int s = r * o + 19;
    int t = s - p + 20;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a - b + c - d + e - f + g - h + i - j +
                         k - l + m - n + o - p + q - r + s - t);
}

/* Function with control flow variations */
int __attribute__((noinline)) test_with_branches(int x, int y, int mode) {
    /* Many local variables */
    int v1 = x * 2;
    int v2 = y + 5;
    int v3 = v1 - v2;
    int v4 = v2 * 3;
    int v5 = v3 + v4;
    int v6 = v4 - v1;
    int v7 = v5 * v6;
    int v8 = v7 / 2;
    int v9 = v8 + v3;
    int v10 = v9 - v5;
    
    if (mode > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables in both branches */
        v1 = v1 + v10;
        v2 = v2 * 2;
        v3 = v3 - v9;
        
        /* Another call */
        clobber_many_regs_2();
        
        v4 = v4 + v8;
        v5 = v5 * 3;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        v6 = v6 + v7;
        v7 = v7 * 4;
        v8 = v8 - v6;
        
        clobber_many_regs_1();
        
        v9 = v9 + v5;
        v10 = v10 * 5;
    }
    
    /* Use all variables after conditional */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Final call that might trigger BB_END update */
    if (result > 100) {
        clobber_many_regs_2();
        result = result / 2;
    } else {
        clobber_many_regs_3();
        result = result * 2;
    }
    
    return result;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int iterations) {
    int acc = 1;
    int a = 2, b = 3, c = 4, d = 5, e = 6;
    int f = 7, g = 8, h = 9, i = 10, j = 11;
    
    for (int n = 0; n < iterations; n++) {
        /* Mix of caller-saved and callee-saved usage */
        int temp = a + b + c + d + e;
        
        /* Call inside loop - forces register spilling */
        clobber_many_regs_1();
        
        /* Use many variables after call */
        acc = acc * (temp + f + g + h + i + j);
        
        /* Modify variables to keep them live */
        a = a + n;
        b = b - n;
        c = c * (n + 1);
        d = d / (n + 2);
        e = e + acc;
        
        /* Another call with different clobbers */
        if (n % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More variable usage */
        f = f + a;
        g = g + b;
        h = h + c;
        i = i + d;
        j = j + e;
    }
    
    return acc + a + b + c + d + e + f + g + h + i + j;
}

/* Function that takes addresses to inhibit optimizations */
int __attribute__((noinline)) test_address_taken(int x, int y) {
    /* Variables whose addresses are taken force different allocation */
    int v1 = x, v2 = y, v3 = x + y, v4 = x * y;
    int v5 = v1 - v2, v6 = v3 + v4, v7 = v5 * v6;
    int v8 = v7 / 2, v9 = v8 + v1, v10 = v9 - v2;
    
    /* Take addresses to force stack/memory usage */
    int *ptr1 = &v1;
    int *ptr2 = &v2;
    int *ptr3 = &v3;
    int *ptr4 = &v4;
    int *ptr5 = &v5;
    
    /* Call between address taking and use */
    clobber_many_regs_1();
    
    /* Use through pointers */
    *ptr1 = *ptr1 + *ptr2;
    *ptr3 = *ptr3 - *ptr4;
    
    clobber_many_regs_2();
    
    *ptr5 = *ptr5 * 2;
    
    /* More variables */
    int v11 = v6 + v7, v12 = v8 + v9, v13 = v10 * v11;
    int v14 = v12 - v13, v15 = v14 / 3;
    
    clobber_many_regs_3();
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary behavior and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed + 1);
    
    /* Test 2: Branches and calls */
    result += test_with_branches(seed, seed * 2, seed % 3);
    
    /* Test 3: Loop with calls */
    result += test_with_loop(seed % 10 + 3);
    
    /* Test 4: Address taken */
    result += test_address_taken(seed, seed / 2);
    
    /* Additional test with mixed types for more register pressure */
    {
        long l1 = seed * 100L;
        long l2 = l1 + 500L;
        long l3 = l2 * 3L;
        double d1 = l1 * 0.5;
        double d2 = l2 * 0.25;
        
        clobber_many_regs_1();
        
        l1 = l1 + (long)d1;
        l2 = l2 + (long)d2;
        
        clobber_many_regs_2();
        
        result += (int)(l1 + l2 + l3);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
