/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Compile with: gcc -O2 -fno-omit-frame-pointer caller_save_coverage.c -o caller_save_test
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
    return x + 1;
}

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Mix arithmetic to create register pressure */
        a = a + i * 2;
        b = b - i / 3;
        c = c ^ (i << 2);
        d = d | (i & 0xF);
        
        /* Function call clobbers registers - forces saves */
        external_func();
        
        /* More operations after call requiring original values */
        sum += a + b - c + d;
        
        /* Conditional that might affect BB structure */
        if (i & 1) {
            a += sum;
            external_func();
            b -= sum;
        }
    }
    
    /* Final computation to use all values */
    return a + b + c + d + sum;
}

/* Scenario 2: Volatile variables preventing optimization */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Complex expression with volatile across call */
    for (int i = 0; i < 15; i++) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call in middle of expression */
        external_func();
        
        v3 = v3 - v2 + i;
        v4 = v4 | (v3 & 0xFF);
        
        /* Another call */
        another_external(v4);
        
        v5 = v5 ^ v4;
        result += v1 + v2 + v3 + v4 + v5;
        
        /* Conditional at end of BB with call */
        if (result > 1000) {
            external_func();
            break;
        }
    }
    
    return result;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    int a = x, b = x * 2, c = x * 3, d = x * 4;
    
    /* Chain of calls where arguments depend on previous results */
    for (int i = 0; i < 10; i++) {
        /* Setup in registers */
        a = a + i;
        b = b - i;
        
        /* First call - arguments in registers */
        int r1 = returning_external(a);
        
        /* Computation between calls */
        c = c ^ r1;
        
        /* Second call with result from first */
        int r2 = returning_external(b + c);
        
        /* Third call with complex expression */
        d = returning_external(r1 + r2 + d);
        
        /* Conditional with call at BB end */
        if (d > 100) {
            external_func();
            a += d;
        } else {
            another_external(d);
            b -= d;
        }
    }
    
    /* Switch statement to create complex BB structure */
    switch (x & 3) {
        case 0:
            a += returning_external(b);
            break;
        case 1:
            b += returning_external(c);
            external_func();  /* Call before break */
            break;
        case 2:
            c += returning_external(d);
            /* No break - fall through */
        default:
            d += returning_external(a);
            external_func();  /* Call in default before break */
            break;
    }
    
    return a + b + c + d;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int jmp_val = 0;

int __attribute__((noinline)) scenario4(int x) {
    volatile int a = x, b = x * 2, c = x * 3;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* First execution path */
        for (int i = 0; i < 5; i++) {
            a += i;
            b -= i;
            
            /* Call that might be before longjmp */
            external_func();
            
            c = a ^ b;
            result += c;
            
            /* Conditional call at BB end */
            if (result > 50) {
                another_external(result);
                jmp_val = result;
                longjmp(env, 1);
            }
        }
    } else {
        /* After longjmp */
        a = a * 2;
        b = b / 2;
        external_func();
        result = a + b + c + jmp_val;
    }
    
    return result;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int a = x, b = x + 1, c = x + 2;
    int *ptr = &a;
    
    /* Force pointer in register */
    asm volatile("" : "+r"(ptr) : : "memory");
    
    /* Computed goto */
    goto *labels[x & 3];
    
label0:
    a += 10;
    external_func();  /* Call in middle of block */
    b += a;
    goto end;
    
label1:
    b += 20;
    another_external(b);
    c += b;
    external_func();  /* Another call */
    goto end;
    
label2:
    c += 30;
    /* Multiple calls in sequence */
    external_func();
    another_external(c);
    external_func();
    a += c;
    goto end;
    
label3:
    a += 40;
    b += 50;
    external_func();
    c += 60;
    /* Call at what might be BB_END */
    another_external(a + b + c);
    
end:
    /* Complex tail with conditional call */
    if (a > b) {
        external_func();
        return a + c;
    } else {
        another_external(b);
        return b + c;
    }
}

/* Main function to execute all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios */
    result += scenario1();
    printf("Scenario1 complete\n");
    
    result += scenario2();
    printf("Scenario2 complete\n");
    
    result += scenario3(42);
    printf("Scenario3 complete\n");
    
    result += scenario4(10);
    printf("Scenario4 complete\n");
    
    result += scenario5(7);
    printf("Scenario5 complete\n");
    
    printf("Final result: %d\n", result);
    printf("Compile with: gcc -O2 -fno-omit-frame-pointer caller_save_coverage.c\n");
    printf("For more aggressive optimization: gcc -O3 -fschedule-insns caller_save_coverage.c\n");
    
    return 0;
}
