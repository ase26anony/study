/* test_caller_save.c - Program to trigger specific caller-save optimization logic */

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

/* Dummy volatile variable to prevent optimizations */
volatile int global_counter = 0;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i + 1;
        c = (c * 3) ^ i;
        d = d + (i << 2);
        
        /* Mix with conditional to create basic block structure */
        if (i & 1) {
            a += b;
            b ^= c;
        } else {
            c -= d;
            d |= a;
        }
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call - registers need to be restored */
        sum += a + b + c + d;
        
        /* Conditional at end of basic block */
        if (sum > 1000) {
            sum = sum / 2;
        }
    }
    
    /* Final computation using all register variables */
    return sum + a - b + c * d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 8; i++) {
        v1 = v1 * 2 + i;
        v2 = v2 - i * 3;
        
        if (v1 > v2) {
            v3 = v3 ^ v4;
            external_func();
            v4 = v4 | v5;
        } else {
            v5 = v5 & v3;
            external_func();
            v3 = v3 + v1;
        }
        
        /* Switch statement to create complex basic block ends */
        switch (i % 3) {
            case 0:
                v1 += external_func_with_return();
                break;  /* Creates jump at end of basic block */
            case 1:
                v2 -= external_func_with_return();
                break;
            default:
                v3 *= external_func_with_return();
                /* No break here - falls through to label */
                goto switch_end;
        }
        
        v4 = v4 / 2;
        
    switch_end:
        result += v1 + v2 + v3 + v4 + v5;
    }
    
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3_inner(int x, int y) {
    external_func();
    return x * y;
}

int __attribute__((noinline)) scenario3(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int total = 0;
    
    /* Outer call setup uses values that must survive inner call */
    for (int i = 0; i < 6; i++) {
        /* Complex expression with multiple subexpressions */
        int temp1 = (a + b) * (c - d);
        int temp2 = (b << 2) | (c & 0xFF);
        
        /* Nested calls with arguments depending on live values */
        a = scenario3_inner(temp1, i);
        b = scenario3_inner(temp2, a);
        
        /* Conditional goto creating specific basic block structure */
        if (a > b) {
            c = scenario3_inner(a, b);
            goto compute;
        } else {
            d = scenario3_inner(b, a);
            /* Fall through */
        }
        
        /* More operations */
        a = a ^ d;
        b = b | c;
        
    compute:
        total += a + b + c + d;
        
        /* Loop with conditional at end */
        if (total > 100) {
            total -= 50;
        }
    }
    
    return total;
}

/* ===== SCENARIO 4: setjmp/longjmp with function calls ===== */
static jmp_buf jump_buffer;

void __attribute__((noinline)) scenario4_helper(int x) {
    if (x > 100) {
        longjmp(jump_buffer, 1);
    }
    external_func();
}

int __attribute__((noinline)) scenario4(void) {
    register int r1 __asm__ ("eax") = 10;
    register int r2 __asm__ ("ecx") = 20;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* This path contains function calls with live registers */
        for (int i = 0; i < 5; i++) {
            r1 += i * 3;
            r2 -= i * 2;
            
            /* Function call that might longjmp */
            scenario4_helper(r1 + r2);
            
            /* These need to be saved/restored due to setjmp */
            result += r1 * r2;
            
            if (result > 500) {
                /* Early exit via goto */
                goto done;
            }
        }
    } else {
        /* longjmp target */
        result = -1;
    }
    
done:
    return result + r1 - r2;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    register int x __asm__ ("eax") = 0;
    register int y __asm__ ("ecx") = 0;
    int z = 0;
    
    for (int i = 0; i < 10; i++) {
        x += i;
        y -= i * 2;
        
        /* Computed goto based on condition */
        goto *labels[i % 4];
        
    label0:
        z = x + y;
        external_func();
        if (z > 0) goto end_loop;
        continue;
        
    label1:
        z = x - y;
        external_func();
        if (z < 0) goto end_loop;
        continue;
        
    label2:
        z = x * y;
        external_func();
        /* Fall through to next case */
        
    label3:
        z = x ^ y;
        external_func();
        /* Continue loop */
        
    end_loop:
        break;
    }
    
    return x + y + z;
}

/* ===== Main function to execute all scenarios ===== */
int main(void) {
    int result = 0;
    
    printf("Running caller-save coverage tests...\n");
    
    /* Run all scenarios and accumulate results */
    result += scenario1();
    printf("Scenario1 complete, result so far: %d\n", result);
    
    result += scenario2();
    printf("Scenario2 complete, result so far: %d\n", result);
    
    result += scenario3();
    printf("Scenario3 complete, result so far: %d\n", result);
    
    result += scenario4();
    printf("Scenario4 complete, result so far: %d\n", result);
    
    result += scenario5();
    printf("Scenario5 complete, final result: %d\n", result);
    
    /* Verify result is non-zero */
    if (result != 0) {
        printf("All scenarios executed successfully.\n");
    }
    
    return 0;
}
