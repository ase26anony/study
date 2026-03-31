/* caller-save-test.c */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External function declarations to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

/* Global to prevent optimizations */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = (a * i) & 0xFF;
        b = (b + i) | 0x1;
        c = (c ^ i) + 1;
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c;
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            a += global_counter;
        } else {
            b -= global_counter;
        }
        
        /* Another call with different register pressure */
        another_external(c);
    }
    
    /* Final computation using register variables */
    return sum + a + b + c;
}

/* Scenario 2: Many volatile variables forcing memory spills */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 15; i++) {
        v1 = v1 * v2 + i;
        v2 = v2 - v3 * i;
        
        /* Call with many live volatiles */
        external_func();
        
        v3 = v3 + v4 / (i + 1);
        v4 = v4 ^ v5;
        
        /* Another call */
        another_external(v1);
        
        v5 = v5 | v1;
        result += v1 + v2 + v3 + v4 + v5;
        
        /* Conditional jump at end of basic block */
        if (v1 > 1000) {
            goto early_exit;
        }
    }
    
early_exit:
    return result;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    register int r3 __asm__ ("edx") = x * 3;
    
    /* Outer call setup in registers */
    r1 = r1 + 1;
    r2 = r2 - 2;
    
    /* Nested call pattern */
    if (x > 0) {
        /* Arguments depend on register values */
        another_external(r1);
        
        /* Inner call with different register usage */
        r3 = r3 * 2;
        external_func();
        
        /* Use results */
        r1 = r1 + r3;
        
        /* Switch statement to create complex BB end */
        switch (x & 3) {
            case 0:
                r2 += 10;
                break;
            case 1:
                external_func();
                r2 += 20;
                break;
            case 2:
                r2 += 30;
                another_external(r3);
                break;
            default:
                /* This case has call then break - may trigger BB_END update */
                another_external(r1);
                break;
        }
    }
    
    return r1 + r2 + r3;
}

/* Scenario 4: setjmp/longjmp with calls */
int __attribute__((noinline)) scenario4(void) {
    int a = 1, b = 2, c = 3;
    volatile int* ptr = &global_counter;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass - do computations with calls */
        for (int i = 0; i < 10; i++) {
            a = a * i + b;
            b = b - i * c;
            
            /* Call that might be before BB_END */
            external_func();
            
            c = c ^ (a + b);
            
            /* Conditional with goto at end */
            if (i == 5) {
                another_external(a);
                goto skip_part;
            }
        }
        
        skip_part:
        a += 100;
    } else {
        /* longjmp target - different path */
        another_external(b);
        a = a * 2;
    }
    
    /* Function with call at end of basic block */
    if (a > b) {
        external_func();
        return a;
    } else {
        another_external(c);
        return b;
    }
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int x) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    register int r1 __asm__ ("eax") = x;
    
    /* Complex switch-like structure with computed goto */
    void* target = labels[x & 3];
    goto *target;
    
label0:
    r1 += 10;
    external_func();  /* Call in middle of block */
    result = r1 * 2;
    goto done;
    
label1:
    r1 -= 5;
    another_external(r1);
    result = r1 + 100;
    /* Fall through to call at block end */
    external_func();
    goto done;
    
label2:
    r1 *= 3;
    /* Multiple calls in sequence */
    external_func();
    another_external(r1);
    result = r1 - 50;
    goto done;
    
label3:
    r1 /= 2;
    /* Call then immediate return-like jump */
    external_func();
    result = r1;
    /* No goto - falls through to done */
    
done:
    return result;
}

/* Helper to create register pressure */
void __attribute__((noinline)) create_pressure(int iterations) {
    register int a __asm__ ("eax") = 0x1234;
    register int b __asm__ ("ecx") = 0x5678;
    register int c __asm__ ("edx") = 0x9ABC;
    
    for (int i = 0; i < iterations; i++) {
        /* Interleave computations with calls */
        a = (a << 1) | (b & 1);
        b = (b >> 1) ^ c;
        
        /* Call that clobbers registers */
        external_func();
        
        c = c + a - b;
        
        /* Another call with different context */
        if (i & 1) {
            another_external(a);
        } else {
            another_external(b);
        }
        
        /* Complex condition creating BB structure */
        if (c > 10000) {
            a = 0;
        } else if (c < 0) {
            b = 0;
            external_func();  /* Call near BB end */
        }
    }
}

int main(void) {
    int total = 0;
    
    /* Call all scenarios to ensure compilation */
    total += scenario1();
    total += scenario2();
    total += scenario3(5);
    total += scenario4();
    total += scenario5(2);
    
    /* Additional pressure */
    create_pressure(5);
    
    /* Simulate longjmp for scenario4 */
    if (global_counter++ < 3) {
        longjmp(jump_buffer, 1);
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
