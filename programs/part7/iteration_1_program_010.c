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

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables in loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a * 2 + i;
        b = b - i * 3;
        c = c ^ (a + b);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - registers must be restored */
        result += a + b - c;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += result;
        } else {
            b -= result;
        }
        
        /* Another call with different register pressure */
        another_external(c);
        
        /* Final computation before loop end */
        c = (c * 7) & 0xFF;
    }
    
    return result + a + b + c;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple basic blocks with calls at different points */
    for (int i = 0; i < 15; i++) {
        v1 = v1 * 3 + i;
        v2 = v2 / 2 - i;
        
        external_func();  /* Call in middle of computation */
        
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        
        /* Conditional jump creates block ending with call */
        if (v1 > v2) {
            another_external(v3);
            /* This call is just before block end */
            goto update_sum;  /* Creates jump at block end */
        }
        
        v5 = v5 + v4;
        external_func();
        
    update_sum:
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* Switch to create complex control flow */
        switch (i % 4) {
            case 0:
                v1 += sum;
                break;
            case 1:
                external_func();  /* Call in switch case */
                v2 -= sum;
                break;  /* Jump instruction at block end */
            case 2:
                v3 *= sum;
                another_external(v4);
                break;
            default:
                v4 /= (sum ? sum : 1);
                external_func();
                /* No break - falls through to increment */
        }
        
        global_counter++;
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Force register usage for arguments */
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    register int r3 __asm__ ("edx") = x * 3;
    
    /* Outer call setup uses registers */
    r1 = r1 + 1;
    r2 = r2 - 2;
    
    /* First call - arguments in registers */
    another_external(r1);
    
    /* Computation between calls */
    r3 = r1 * r2;
    
    /* Nested call pattern */
    if (r3 > 0) {
        /* Inner call with different arguments */
        external_func();
        
        /* More register operations */
        r1 = r3 / 2;
        another_external(r2);
        
        /* This creates a block ending with call+jump */
        if (r1 < 100) {
            external_func();
            goto compute_result;  /* Creates jump after call at block end */
        }
        
        r2 = r1 + r3;
    }
    
    external_func();
    
compute_result:
    /* Final computation uses all registers */
    return r1 + r2 * 2 + r3 * 3;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int value = 0;
    int ret = setjmp(jump_buffer);
    
    if (ret == 0) {
        /* First time through */
        register int a __asm__ ("eax") = 42;
        register int b __asm__ ("ecx") = 24;
        
        /* Computation before call */
        a = a * 2 + global_counter;
        b = b - global_counter;
        
        /* Call that might be before longjmp */
        external_func();
        
        value = a + b;
        
        /* Another call just before potential longjmp */
        another_external(value);
        
        /* Simulate error - triggers longjmp */
        if (global_counter > 1000) {
            longjmp(jump_buffer, 1);
        }
        
        /* More register use after calls */
        a = a ^ b;
        value += a;
        
        external_func();
        
        return value;
    } else {
        /* After longjmp */
        return ret + global_counter;
    }
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int mode) {
    static const void* labels[] = { &&case0, &&case1, &&case2, &&case3 };
    register int r1 __asm__ ("eax") = mode;
    register int r2 __asm__ ("ecx") = mode * 10;
    int result = 0;
    
    /* Use computed goto to create unusual control flow */
    goto *labels[mode % 4];
    
case0:
    r1 = r1 + 5;
    external_func();  /* Call in middle of case */
    r2 = r2 - 3;
    result = r1 + r2;
    goto end;
    
case1:
    another_external(r1);
    /* Call at block end before jump */
    if (r2 > 0) {
        external_func();
        goto end;  /* Jump instruction right after call */
    }
    r1 = r1 * 2;
    result = r1;
    goto end;
    
case2:
    r2 = r2 / 2;
    external_func();
    r1 = r1 ^ r2;
    /* Another call then fall through */
    
case3:
    another_external(r2);
    /* Multiple calls in sequence */
    external_func();
    result = r1 * r2;
    /* Fall through to end */
    
end:
    return result;
}

/* ===== Main function to exercise all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run each scenario multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += scenario1();
        total += scenario2();
        total += scenario3(i);
        total += scenario4();
        total += scenario5(i);
        
        global_counter++;
    }
    
    printf("Final checksum: %d\n", total);
    printf("(If this compiles and runs, the caller-save pass was exercised)\n");
    
    return 0;
}
