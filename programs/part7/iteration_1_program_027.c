/* caller-save-coverage.c
 * Designed to trigger specific instruction list manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller-save-coverage.c
 * Or with: gcc -O3 -fschedule-insns -freg-struct-return -c caller-save-coverage.c
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External function declarations to force calls */
void __attribute__((noinline)) external_func(void);
void __attribute__((noinline)) another_external(int);
int __attribute__((noinline) external_with_return(void);

/* Dummy implementations to satisfy linker */
void external_func(void) {
    volatile int x = 0;
    (void)x;
}

void another_external(int x) {
    volatile int y = x;
    (void)y;
}

int external_with_return(void) {
    volatile int x = 42;
    return x;
}

/* Scenario 1: Explicit register variables with loop */
/* Forces multiple caller-save operations in a loop */
int __attribute__((noinline)) scenario1(void) {
    /* Use explicit register constraints for call-clobbered registers (x86-64) */
    register int a __asm__ ("rax") = 1;
    register int b __asm__ ("rcx") = 2;
    register int c __asm__ ("rdx") = 3;
    register int d __asm__ ("rsi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across calls */
    for (int i = 0; i < 10; ++i) {
        /* Multiple operations on register variables */
        a += i * 2;
        b -= i;
        c = a ^ b;
        d = c * i;
        
        /* Function call clobbers registers - forces saves/restores */
        external_func();
        
        /* More operations after call - values must be restored */
        sum += a + b - c + d;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += external_with_return();
        } else {
            b -= external_with_return();
        }
        
        /* Another call with different register pressure */
        another_external(sum);
    }
    
    return sum + a + b + c + d;
}

/* Scenario 2: Volatile variables preventing optimization */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Complex expression with volatiles across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        v3 = v3 ^ v2;
        
        /* Call that forces spills of volatile values */
        external_func();
        
        v4 = v4 - v3 + v2;
        v5 = v5 | v4;
        
        /* Another call with different arguments */
        another_external(v1 + v2);
        
        /* Conditional jump at end of basic block */
        if (v5 > 100) {
            result += v1;
            /* goto creates edge in control flow */
            goto compute;
        } else {
            result += v2;
        }
        
        v1 += external_with_return();
    }
    
compute:
    return result + v3 + v4 + v5;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int x) {
    /* Values computed in call-clobbered registers */
    int a = x * 2;
    int b = x + 7;
    int c = x ^ 0xFF;
    
    /* Outer call setup uses registers that inner call clobbers */
    int r1 = external_with_return();
    
    /* Inner call with arguments that depend on outer call results */
    another_external(a + r1);
    
    /* More computation between calls */
    a += b * c;
    
    /* Switch statement with calls in cases - creates multiple BB ends */
    switch (x & 3) {
        case 0:
            external_func();
            a += 10;
            break;  /* Creates jump at BB end */
        case 1:
            another_external(a);
            b += 20;
            break;
        case 2:
            a = external_with_return();
            c += 30;
            break;
        default:
            external_func();
            another_external(b);
            a = b + c;
            break;
    }
    
    /* Final call with complex expression */
    another_external(a + b + c + external_with_return());
    
    return a * 100 + b * 10 + c;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;

int __attribute__((noinline)) scenario4(int x) {
    volatile int counter = 0;
    int result = 0;
    
    if (setjmp(env) == 0) {
        /* First execution path */
        for (int i = 0; i < 5; i++) {
            /* Register-intensive computation */
            int a = x + i * 3;
            int b = x - i * 2;
            int c = a * b;
            
            /* Function call that might be saved across longjmp */
            external_func();
            
            result += a + b + c;
            counter++;
            
            /* Conditional that could trigger longjmp */
            if (result > 1000) {
                longjmp(env, 1);
            }
        }
    } else {
        /* After longjmp - different register usage */
        result = external_with_return() * 2;
        another_external(result);
    }
    
    return result + counter;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int a = 1, b = 2, c = 3, d = 4;
    int* ptrs[] = { &a, &b, &c, &d };
    
    /* Force values into registers */
    a = selector * 10;
    b = selector * 20;
    c = selector * 30;
    d = selector * 40;
    
    /* Computed goto - creates unusual control flow */
    goto *labels[selector & 3];
    
label0:
    external_func();
    a += external_with_return();
    goto end;
    
label1:
    another_external(a);
    b += external_with_return();
    /* Fall through to call */
    external_func();
    goto end;
    
label2:
    c = external_with_return();
    another_external(b + c);
    goto end;
    
label3:
    external_func();
    d = external_with_return();
    another_external(c + d);
    /* No goto - falls through to end */
    
end:
    return a + b * 2 + c * 3 + d * 4;
}

/* Main function to exercise all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    result += scenario1();
    result += scenario2();
    result += scenario3(42);
    result += scenario4(17);
    result += scenario5(2);
    
    printf("Final result: %d\n", result);
    
    /* Verify the result is non-zero */
    if (result != 0) {
        printf("All scenarios executed successfully.\n");
    }
    
    return 0;
}
