/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
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

int __attribute__((noinline)) returning_external(void) {
    return 42;
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 10; ++i) {
        /* Multiple operations on register variables */
        a = a + i * 2;
        b = b - i * 3;
        c = c ^ (i * 5);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call - registers must be restored */
        a = a + 1;
        b = b * 2;
        c = c | 0xFF;
        
        /* Conditional that creates basic block structure */
        if (i & 1) {
            a += returning_external();  /* Another call */
        }
        
        result += a + b + c;
    }
    
    return result;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Complex expression with volatiles across call */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * v2 + v3;
        v2 = v2 - v4 / (v5 + 1);
        
        /* Call in the middle of computation */
        another_external(v1);
        
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        v5 = v5 & v3;
        
        /* Create basic block ending with call and jump */
        if (v1 > 100) {
            external_func();
            goto compute_sum;  /* Creates jump at end of basic block */
        }
        
        v1 += i;
    }
    
compute_sum:
    sum = v1 + v2 + v3 + v4 + v5;
    
    /* Switch with default case containing call */
    switch (sum % 4) {
        case 0: sum += 10; break;
        case 1: sum += 20; break;
        default:
            external_func();  /* Call before break at block end */
            sum += returning_external();
            break;  /* Jump instruction at block end */
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Multiple calls where arguments depend on previous results */
    int a = returning_external();  /* First call */
    
    /* Use result in computation before next call */
    register int r1 __asm__ ("eax") = a * 2;
    register int r2 __asm__ ("ecx") = a + 5;
    
    /* Nested call pattern */
    for (int i = 0; i < 5; ++i) {
        /* Arguments depend on register values */
        another_external(r1 + i);
        
        /* Intermediate computation */
        r1 = r1 * 3 - r2;
        external_func();
        r2 = r2 / 2 + r1;
        
        /* Conditional with call at end */
        if (r1 > r2) {
            int temp = returning_external();
            r1 += temp;
            goto update;  /* Creates control flow */
        }
        
        r1 += i;
    }
    
update:
    /* Final call with complex expression */
    another_external(r1 * r2 + x);
    
    return r1 + r2;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int j1 __asm__ ("eax") = 100;
        register int j2 __asm__ ("ecx") = 200;
        
        /* Computation across call */
        j1 = j1 * 2;
        j2 = j2 - 50;
        
        /* Call that might longjmp */
        external_func();
        
        /* More computation - registers must be preserved */
        j1 += 10;
        j2 *= 2;
        
        result = j1 + j2;
        
        /* Simulate error condition */
        if (global_counter++ > 5) {
            longjmp(jump_buffer, 1);
        }
    } else {
        /* After longjmp */
        result = -1;
    }
    
    /* Another call in cleanup path */
    another_external(result);
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int value = 0;
    
    register int g1 __asm__ ("eax") = selector * 10;
    register int g2 __asm__ ("ecx") = selector * 20;
    
    /* Call before computed goto */
    external_func();
    
    /* Update registers */
    g1 += 5;
    g2 -= 3;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    value = g1 + g2;
    another_external(value);
    break;
    
label1:
    g1 *= 2;
    external_func();  /* Call in middle of block */
    g2 /= 2;
    value = g1 - g2;
    break;
    
label2:
    value = returning_external();  /* Call at block start */
    g1 += value;
    g2 -= value;
    value = g1 * g2;
    break;
    
label3:
    /* Multiple calls in sequence */
    another_external(g1);
    external_func();
    another_external(g2);
    value = g1 + g2;
    break;
    
    return value;
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
    total += scenario5(2);
    
    printf("Final checksum: %d\n", total);
    
    /* Verify with a simple computation */
    if (total != 0) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
