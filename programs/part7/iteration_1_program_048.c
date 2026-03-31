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

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Volatile function to prevent optimization */
int __attribute__((noinline)) volatile_func(volatile int x) {
    return x + 1;
}

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with register pressure across call */
    for (int i = 0; i < 10; ++i) {
        /* Multiple operations on register variables */
        a = a + i * 2;
        b = b - i * 3;
        c = c ^ i;
        
        /* Mix with memory operations */
        volatile int temp = a + b;
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call - registers need restoring */
        a = a + temp;
        b = b ^ c;
        result += a + b + c;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            c = c * 2;
            external_func();
        }
    }
    
    return result;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int sum = 0;
    
    /* Multiple expressions using volatiles across calls */
    for (int i = 0; i < 5; ++i) {
        v1 = v1 + v2;
        v3 = v3 * v4;
        v5 = v5 - v6;
        
        /* Call forces spills of all live volatiles */
        external_func();
        
        v7 = v7 ^ v8;
        v9 = v9 / (v10 + 1);
        sum += v1 + v3 + v5 + v7 + v9;
        
        /* Another call with different register pressure */
        if (i % 2 == 0) {
            v2 = v2 + external_func_with_return();
        }
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    int result = x;
    
    /* Chain of operations where inner call args depend on outer call setup */
    for (int i = 0; i < 8; ++i) {
        /* Setup in registers */
        int a = i * 3;
        int b = i * 5;
        int c = i * 7;
        
        /* Outer call setup */
        a = volatile_func(a);
        
        /* Inner call with args depending on outer result */
        b = volatile_func(a + b);
        
        /* Another call */
        external_func();
        
        c = volatile_func(b + c);
        
        /* Complex conditional at block end */
        if (c > 20) {
            result += c;
            goto update;  /* Creates jump at block end */
        }
        
        result += a + b;
        
    update:
        /* Label after goto creates block structure */
        if (i == 7) {
            external_func();
        }
    }
    
    return result;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
static jmp_buf env;
int __attribute__((noinline)) scenario4(void) {
    volatile int x = 0;
    volatile int y = 0;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* First path: register pressure before longjmp */
        x = 10;
        y = 20;
        
        /* Function call that might be saved across longjmp */
        external_func();
        
        /* Operations that need registers preserved */
        x = x * 2;
        y = y + 5;
        
        /* Simulate longjmp - never returns here */
        result = x + y;
    } else {
        /* Second path after longjmp */
        x = 30;
        y = 40;
        
        /* Another call */
        external_func();
        
        result = x - y;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with function call ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    volatile int a = 1, b = 2;
    
    /* Force values into registers */
    a = a * selector;
    b = b + selector;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    a = a + 10;
    external_func();  /* Call near block end before jump */
    result = a;
    goto end;
    
label1:
    b = b * 3;
    external_func();
    result = b;
    goto end;
    
label2:
    a = a - b;
    external_func();
    result = a * b;
    goto end;
    
label3:
    external_func();  /* Call at block end */
    result = a + b;
    /* Fall through to end */
    
end:
    return result;
}

/* ===== SCENARIO 6: Switch statement with calls at block ends ===== */
int __attribute__((noinline)) scenario6(int code) {
    int result = 0;
    register int r1 __asm__ ("eax") = code * 2;
    register int r2 __asm__ ("ecx") = code * 3;
    
    switch (code % 4) {
        case 0:
            r1 = r1 + 10;
            r2 = r2 - 5;
            external_func();  /* Call before break at block end */
            result = r1;
            break;
            
        case 1:
            r1 = r1 * 2;
            external_func();
            r2 = r2 ^ r1;
            result = r2;
            break;
            
        case 2:
            external_func();
            r1 = r1 + r2;
            /* No break - fall through */
            
        case 3:
            r2 = r2 * 3;
            external_func();  /* Another call */
            result = r1 + r2;
            if (result > 100) {
                goto special;  /* Conditional goto at block end */
            }
            break;
    }
    
    return result;
    
special:
    return result * 2;
}

/* ===== Main function to execute all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run each scenario */
    total += scenario1();
    total += scenario2();
    total += scenario3(5);
    
    /* Setup longjmp for scenario4 */
    if (1) {
        total += scenario4();
    }
    
    total += scenario5(2);
    total += scenario6(3);
    
    printf("Total checksum: %d\n", total);
    
    /* Verify with a simple calculation */
    int verify = 0;
    for (int i = 0; i < 100; i++) {
        verify += i;
        if (i % 10 == 0) {
            external_func();
        }
    }
    
    printf("Verification sum: %d\n", verify + total);
    
    return 0;
}
