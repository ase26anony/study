/* caller_save_test.c - Test program for GCC caller-save optimization */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call */
    asm volatile("" : : : "memory");
}

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Volatile counter to prevent optimizations */
static volatile int global_counter = 0;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (a + b);
        d = d * (i + 1);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        result += a + b;
        result ^= c;
        result *= (d & 0xFF);
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            a += result;
            external_func();
            b -= result;
        }
        
        /* Another call with different register pressure */
        if (i % 5 == 0) {
            c = external_func_with_return();
            d += c;
        }
    }
    
    return result + a + b + c + d;
}

/* Scenario 2: Many volatile variables forcing memory spills */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int result = 0;
    
    /* Complex expression with calls in the middle */
    for (int i = 0; i < 15; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call with many live volatile values */
        external_func();
        
        v4 = v4 + v3;
        v5 = v5 - v4;
        
        /* Another call */
        if (i % 4 == 0) {
            external_func_with_return();
            v6 = v6 * v5;
        }
        
        v7 = v7 + v6;
        v8 = v8 ^ v7;
        v9 = v9 * v8;
        v10 = v10 - v9;
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Create basic block ending with call and jump */
        if (result > 1000) {
            external_func();
            goto skip;
        }
        
        v1 += 10;
    skip:
        v2 -= 5;
    }
    
    return result;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    int a = x, b = x * 2, c = x * 3, d = x * 4;
    
    /* Chain of operations with calls in between */
    for (int i = 0; i < 10; ++i) {
        /* Setup values in registers */
        a = a + i;
        b = b - i * 2;
        
        /* Outer call - arguments depend on register values */
        int r1 = external_func_with_return() + a;
        
        c = c ^ r1;
        
        /* Inner call with different arguments */
        if (c > 0) {
            int r2 = external_func_with_return() + b;
            d = d * r2;
            
            /* Another call in the same block before jump */
            external_func();
            if (d > 100) break;
        }
        
        /* Switch to create complex basic block structure */
        switch (i % 3) {
            case 0:
                a += external_func_with_return();
                break;
            case 1:
                external_func();
                b *= 2;
                break;
            default:
                /* This creates BB ending with call and break */
                external_func();
                c = external_func_with_return();
                break;
        }
    }
    
    return a + b + c + d;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf jump_buffer;
int __attribute__((noinline)) scenario4(void) {
    volatile int x = 1, y = 2, z = 3;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass */
        for (int i = 0; i < 8; ++i) {
            x = x * 2 + i;
            y = y - i;
            z = z ^ (x + y);
            
            /* Call with live values that might be longjmp'd over */
            external_func();
            
            result += x + y + z;
            
            /* Conditional with call at end of basic block */
            if (i == 5) {
                external_func_with_return();
                /* Potential longjmp target */
            }
        }
    } else {
        /* longjmp return path */
        result += 1000;
    }
    
    /* Another call after setjmp context */
    external_func();
    
    return result;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    int i = 0;
    
label0:
    a += i;
    external_func();  /* Call in middle of basic block */
    b -= i;
    goto *labels[(i + 1) % 4];
    
label1:
    c *= 2;
    if (i++ > 10) goto end;
    external_func_with_return();
    d /= 2;
    goto *labels[(i + 2) % 4];
    
label2:
    a ^= b;
    external_func();
    c += d;
    goto *labels[(i + 3) % 4];
    
label3:
    d = external_func_with_return();
    result += a + b + c + d;
    goto *labels[i % 4];
    
end:
    return result;
}

/* Main function to drive all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    result += scenario1();
    printf("Scenario 1 completed\n");
    
    result += scenario2();
    printf("Scenario 2 completed\n");
    
    result += scenario3(5);
    printf("Scenario 3 completed\n");
    
    result += scenario4();
    printf("Scenario 4 completed\n");
    
    result += scenario5();
    printf("Scenario 5 completed\n");
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 0) {
        return 0;
    } else {
        return 1;
    }
}
