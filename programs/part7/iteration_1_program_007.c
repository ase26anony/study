/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but not inlined */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    /* Clobber multiple registers */
    asm volatile("" : : "r"(x) : "eax", "ecx", "edx", "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    int sum = 0;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 10; ++i) {
        /* Multiple operations on register variables */
        a = a * 2 + i;
        b = b - i * 3;
        c = c ^ (a + b);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call - registers must be restored */
        sum += a + b - c;
        a = (a ^ b) + 1;
        b = (b << 2) | 1;
        c = c + sum;
        
        /* Another call with different clobber pattern */
        another_external(sum);
        
        /* Final computation before loop end */
        sum = (sum * 3) / 2;
    }
    
    /* Force all registers to be used in return computation */
    return sum + a - b * c;
}

/* ===== SCENARIO 2: Volatile variables and complex basic block end ===== */
int __attribute__((noinline)) scenario2(int x) {
    volatile int v1 = x * 2;
    volatile int v2 = x + 7;
    volatile int v3 = x - 3;
    int result = 0;
    
    /* Create a basic block that ends with call + conditional */
    if (x > 0) {
        /* Multiple volatile operations */
        v1 = v1 * v2;
        v2 = v2 + v3;
        v3 = v3 ^ v1;
        
        /* External call - will need spills for volatiles */
        external_func();
        
        /* This creates a jump at end of basic block */
        if (v1 > v2) {
            result = v1 - v2;
            goto compute_end;  /* Creates jump at BB end */
        } else {
            result = v2 - v1;
        }
        
        /* More operations after the label */
        v3 = result * 2;
    }
    
compute_end:
    /* Mix volatiles with another call */
    another_external(v3);
    
    /* Complex return to prevent tail optimization */
    return result + v1 * 2 - v2 / 3 + v3;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int seed) {
    /* Use explicit registers again */
    register int r1 __asm__ ("eax") = seed;
    register int r2 __asm__ ("ecx") = seed * 2;
    register int r3 __asm__ ("edx") = seed + 100;
    
    /* Outer call setup uses registers */
    int temp = r1 + r2;
    
    /* Nested call pattern */
    external_func();  /* First call clobbers registers */
    
    /* Arguments for next call depend on register values */
    r1 = temp * 3;
    r2 = r3 - temp;
    
    /* Inner call with register-based arguments */
    another_external(r1 + r2);
    
    /* Switch statement to create complex BB structure */
    switch (seed % 4) {
        case 0:
            r3 = r1 * r2;
            external_func();  /* Call in switch case */
            break;  /* Creates jump at BB end */
        case 1:
            r3 = r1 + r2;
            /* Fall through */
        case 2:
            r3 = r3 ^ 0xFF;
            another_external(r3);
            break;
        default:
            r3 = 0;
            external_func();  /* Call at end of default case */
            break;  /* Jump at BB end */
    }
    
    return r1 + r2 * 2 - r3;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(int x) {
    register int a __asm__ ("eax") = x;
    register int b __asm__ ("ecx") = x * 3;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        a = a + b;
        b = b * 2;
        
        /* Call that might longjmp */
        external_func();
        
        /* These must be preserved across potential longjmp */
        result = a - b;
        a = result * 3;
        
        /* Simulate error condition */
        if (x > 1000) {
            longjmp(jump_buffer, 1);
        }
        
        /* Another call after condition */
        another_external(result);
    } else {
        /* longjmp return path */
        result = a + b + 100;
    }
    
    /* Force register usage before return */
    global_counter += a;
    return result + b;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5(int selector) {
    register int r1 __asm__ ("eax") = selector;
    register int r2 __asm__ ("ecx") = selector + 10;
    int result = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&case0, &&case1, &&case2, &&case3 };
    
    if (selector >= 0 && selector < 4) {
        goto *labels[selector];
    }
    
case0:
    r1 = r1 * 2;
    external_func();  /* Call in goto block */
    result = r1 + r2;
    goto end;
    
case1:
    r2 = r2 - 5;
    /* Call followed by computed goto */
    another_external(r1);
    result = r1 - r2;
    goto end;
    
case2:
    r1 = r1 ^ r2;
    external_func();
    result = r1 * r2;
    /* Fall through to case3 */
    
case3:
    r2 = r2 + 20;
    another_external(r2);
    result = result + r1 + r2;
    /* No goto - falls through to end */
    
end:
    /* Multiple operations to create register pressure */
    for (int i = 0; i < 3; i++) {
        r1 = (r1 << 1) | 1;
        external_func();
        r2 = r2 + r1;
    }
    
    return result + r1 - r2;
}

/* ===== MAIN FUNCTION ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2(42);
    total += scenario3(17);
    total += scenario4(500);
    total += scenario5(2);
    
    /* Add some variation */
    total += scenario2(100);
    total += scenario3(99);
    
    printf("Final result: %d\n", total);
    
    /* Verify with a simple check */
    if (total != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
