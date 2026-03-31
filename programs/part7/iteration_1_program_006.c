/* caller-save-coverage.c
 * Designed to trigger specific instruction list manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller-save-coverage.c
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

/* Dummy volatile variable to prevent optimizations */
volatile int global_volatile = 42;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    int sum = 0;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 10; ++i) {
        /* Use registers before call */
        a += i * 2;
        b -= i + 1;
        c ^= (a + b);
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use registers after call - must be restored */
        sum += a + b + c;
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            a += global_volatile;
        } else {
            b -= global_volatile;
        }
        
        /* More arithmetic to increase register pressure */
        c = (c << 1) | (c >> 31);
    }
    
    /* Final computation using all registers */
    return sum + a + b + c;
}

/* Scenario 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario2(int x) {
    volatile int v1 = x;
    volatile int v2 = x * 2;
    volatile int v3 = x * 3;
    volatile int v4 = x * 4;
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 5; ++i) {
        v1 += i;
        v2 -= i;
        
        /* Call that might be inserted near BB_END */
        external_func();
        
        v3 *= (i + 1);
        v4 /= (i + 2);
        
        /* Conditional jump at end of basic block */
        if (v1 > v2) {
            result += v3;
            /* goto-like structure */
            if (v4 < 100) break;
        } else {
            result -= v4;
        }
        
        /* Another call in different path */
        another_external(v1);
    }
    
    return result + v1 + v2 + v3 + v4;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int seed) {
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    
    /* Outer call setup uses registers */
    a = (a << 3) | (a >> 29);
    
    /* Inner call depends on outer call's register values */
    for (int i = 0; i < 3; ++i) {
        int temp = a + b;
        
        /* Call with arguments in registers */
        another_external(temp);
        
        /* Computation that uses call-clobbered registers */
        c = c * 2 + temp;
        
        /* Switch statement to create complex BB structure */
        switch (i) {
            case 0:
                a += c;
                external_func();  /* Call in switch case */
                break;
            case 1:
                b -= c;
                /* Call followed by break at BB end */
                another_external(b);
                break;
            default:
                c ^= a ^ b;
                external_func();
                /* Multiple instructions after call before BB end */
                if (c > 0) {
                    a = b;
                }
                break;
        }
    }
    
    return a + b + c;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int __attribute__((noinline)) scenario4(int x) {
    int a = x;
    int b = x * 2;
    
    if (setjmp(env) == 0) {
        /* First path: use registers, make call */
        a += 100;
        b -= 50;
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More register usage */
        a = a * b;
        
        /* Simulate longjmp - never returns here */
        if (a > 1000) {
            longjmp(env, 1);
        }
    } else {
        /* Second path after longjmp */
        a += 200;
        b += 300;
        
        /* Another call */
        another_external(a);
        
        /* Complex end of basic block */
        if (b > a) {
            a = b;
            external_func();
            /* Call at BB end with following jump */
            goto done;
        }
    }
    
    a += 500;
    
done:
    return a + b;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int x) {
    static void *labels[] = { &&label1, &&label2, &&label3 };
    int a = x;
    int b = x + 1;
    int c = x + 2;
    
    /* Use computed goto to create unusual CFG */
    goto *labels[x % 3];
    
label1:
    a += 10;
    external_func();  /* Call in goto block */
    /* Multiple instructions after call */
    b = a * 2;
    goto end;
    
label2:
    b += 20;
    /* Call near end of basic block */
    another_external(b);
    c = b / 2;
    goto end;
    
label3:
    c += 30;
    external_func();
    a = c + 5;
    /* Fall through to end */
    
end:
    /* Final computation with all values live */
    external_func();
    return a + b + c;
}

/* Helper to prevent optimization */
int __attribute__((noinline)) use_result(int x) {
    volatile int sink = x;
    return sink;
}

/* Main function to drive all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    result ^= use_result(scenario1());
    result ^= use_result(scenario2(5));
    result ^= use_result(scenario3(7));
    result ^= use_result(scenario4(3));
    result ^= use_result(scenario5(2));
    
    /* Add some global volatile usage */
    result += global_volatile;
    
    printf("Final result: %d\n", result);
    printf("(This value is meaningless - coverage is during compilation)\n");
    
    return 0;
}
