/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller_save_coverage.c
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
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Create register pressure before call */
        a = a + i * 2;
        b = b - i / 2;
        c = c ^ (i + 1);
        d = d * (i % 3 + 1);
        
        /* Mix arithmetic to create dense sequence */
        a = (a << 1) | (b & 0xFF);
        b = (b >> 2) + c;
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* More arithmetic to extend basic block */
        a = a ^ sum;
        b = b + (sum >> 3);
        c = c * 2 - sum;
        d = d & 0xFFFF;
    }
    
    /* Conditional at end of block */
    if (sum > 1000) {
        external_func();
        return sum;
    }
    
    return sum + a + b;
}

/* ===== SCENARIO 2: Volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    
    int result = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call with side effects */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v1;
        
        /* Another call */
        another_external(v3);
        
        v1 = v1 + v4;
        v2 = v2 - v3;
        
        result += v1 + v2 + v3 + v4;
        
        /* Conditional jump at end of block */
        if (i & 1) {
            external_func();
            /* continue creates fallthrough */
        } else {
            v1 += returning_external();
        }
    }
    
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    register int r3 __asm__ ("edx") = x * 3;
    
    /* Outer call setup in registers */
    r1 = r1 + 100;
    r2 = r2 * 2;
    
    /* Nested call pattern */
    int temp = returning_external();  /* First call */
    
    /* Use result immediately in argument to another call */
    another_external(temp + r1);      /* Second call depends on r1 */
    
    /* More register manipulation */
    r3 = r3 ^ temp;
    r1 = r1 << 2;
    
    /* Switch statement to create complex block ends */
    switch (x % 4) {
        case 0:
            external_func();
            r2 += 10;
            break;  /* Creates jump at block end */
        case 1:
            r1 = returning_external();
            external_func();
            r3 -= 5;
            break;
        case 2:
            r2 = r2 * 3;
            /* Fall through */
        default:
            external_func();
            r1 = r1 + r2 + r3;
            /* No break - falls through to return */
    }
    
    return r1 + r2 * 3;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    register int a __asm__ ("eax") = 100;
    register int b __asm__ ("ecx") = 200;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        a = a * 2;
        b = b / 2;
        
        /* Call that might longjmp */
        external_func();
        
        a = a + 50;
        b = b - 25;
        
        /* Another call */
        another_external(a + b);
        
        return a + b;
    } else {
        /* After longjmp */
        a = a ^ 0xFF;
        b = b & 0x7F;
        
        external_func();
        
        return a - b;
    }
}

/* ===== SCENARIO 5: Computed goto with function calls ===== */
int __attribute__((noinline)) scenario5(int mode) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r1 __asm__ ("eax") = mode;
    register int r2 __asm__ ("ecx") = mode * 10;
    
    /* Complex pre-call computation */
    r1 = (r1 << 3) | (r2 & 7);
    r2 = r2 ^ r1;
    
    /* Call before computed goto */
    external_func();
    
    if (mode < 0 || mode > 3) mode = 0;
    
    /* Computed goto */
    goto *labels[mode];
    
label0:
    r1 += returning_external();
    external_func();
    r2 = r2 * 2;
    goto end;
    
label1:
    another_external(r1);
    r2 = r2 + 100;
    external_func();
    goto end;
    
label2:
    r1 = r1 ^ r2;
    external_func();
    r2 = returning_external();
    goto end;
    
label3:
    external_func();
    r1 = r1 - r2;
    another_external(r1);
    /* fall through */
    
end:
    return r1 + r2;
}

/* Helper to trigger longjmp */
void __attribute__((noinline)) trigger_longjmp(void) {
    longjmp(jump_buffer, 1);
}

/* ===== MAIN FUNCTION ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(5);
    total += scenario4();
    total += scenario5(2);
    
    /* Trigger longjmp path */
    trigger_longjmp();
    
    /* Run scenario4 again to get longjmp result */
    total += scenario4();
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return total & 0xFF;
}
