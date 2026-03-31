/* caller_save_test.c - Test program to trigger specific caller-save optimization logic */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call boundary */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returns_value(void) {
    return 42;
}

/* Global jump buffer for setjmp/longjmp test */
jmp_buf env;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i;
        c = a ^ b;
        d = c + i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* Conditional to create basic block structure */
        if (i % 3 == 0) {
            a += returns_value();
        } else if (i % 3 == 1) {
            b -= returns_value();
        }
        
        /* More arithmetic to increase register pressure */
        c = (a * b) / (d + 1);
        d = (b << 2) | (a & 0xFF);
    }
    
    /* Final computation using all register variables */
    return sum + a - b + c * d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    
    int result = 0;
    
    /* Multiple expressions with volatiles across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        v5 = v5 & v4;
        
        /* Call between volatile uses */
        another_external(v1);
        
        /* More volatile operations */
        v1 = v1 + v5;
        v2 = v2 - v4;
        
        /* Another call */
        external_func();
        
        v3 = v3 * v2;
        v4 = v4 / (v1 + 1);
        
        /* Conditional with call at end of basic block */
        if (v1 > 100) {
            result += returns_value();
            /* Call at end of if-block before break-like structure */
            external_func();
            goto compute;
        } else if (v2 < 50) {
            result -= returns_value();
            external_func();
        }
        
        v5 = v5 + v3 - v4;
    }
    
compute:
    result += v1 + v2 + v3 + v4 + v5;
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Inner function that uses call-clobbered registers */
    int inner_func(int y) {
        register int r1 __asm__ ("eax") = y;
        register int r2 __asm__ ("ecx") = y * 2;
        external_func();
        return r1 + r2;
    }
    
    int sum = 0;
    
    /* Outer loop with nested calls */
    for (int i = 0; i < 5; ++i) {
        /* Setup in call-clobbered registers */
        register int a __asm__ ("eax") = x + i;
        register int b __asm__ ("ecx") = x * i;
        register int c __asm__ ("edx") = x - i;
        
        /* Use values before first call */
        int temp = a * b + c;
        
        /* Call that clobbers registers */
        another_external(temp);
        
        /* Values must be restored for next computation */
        int val1 = a + 10;
        int val2 = b - 5;
        
        /* Nested call sequence */
        val1 = inner_func(val1);
        external_func();
        val2 = inner_func(val2);
        
        /* Switch statement with calls near block ends */
        switch (i % 3) {
            case 0:
                sum += val1;
                external_func();  /* Call before break */
                break;
            case 1:
                sum += val2;
                another_external(val1);
                break;
            default:
                sum += val1 + val2;
                external_func();
                /* No break here - falls through to label */
                goto switch_end;
        }
        
        /* Label for computed goto */
        switch_end:
        sum += 1;
    }
    
    return sum;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int counter = 0;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* First time through */
        register int a __asm__ ("eax") = 1;
        register int b __asm__ ("ecx") = 2;
        
        for (int i = 0; i < 3; ++i) {
            a += i * 10;
            b -= i * 5;
            
            /* Call before potential longjmp */
            external_func();
            
            result += a + b;
            
            /* Condition that might trigger longjmp */
            if (counter++ > 5) {
                longjmp(env, 1);
            }
            
            /* More register use */
            a = a ^ b;
            b = b << 1;
            
            /* Another call */
            another_external(a);
        }
    } else {
        /* After longjmp */
        result += 1000;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int x) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int result = 0;
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    
    /* Jump table based on input */
    goto *labels[x % 4];
    
label0:
    r1 += 10;
    external_func();  /* Call in middle of basic block */
    r2 -= 5;
    result = r1 + r2;
    goto end;
    
label1:
    r1 *= 2;
    another_external(r1);
    r2 /= 2;
    result = r1 - r2;
    /* Fall through to call */
    external_func();
    goto end;
    
label2:
    r1 = r1 ^ 0xFF;
    external_func();
    r2 = r2 | 0xAA;
    result = r1 * r2;
    /* Conditional at end of block with call */
    if (result > 100) {
        another_external(result);
        goto end;
    }
    /* else fall through */
    
label3:
    r1 = ~r1;
    r2 = -r2;
    external_func();
    result = r1 & r2;
    /* Multiple calls in sequence */
    another_external(r1);
    external_func();
    /* end label is here */

end:
    return result;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(7);
    total += scenario4();
    total += scenario5(3);
    
    printf("Final checksum: %d\n", total);
    
    /* Verify with simple computation */
    if (total != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
