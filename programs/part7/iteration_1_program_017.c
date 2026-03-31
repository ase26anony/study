/* test_caller_save.c - Program to trigger GCC caller-save pass list manipulation */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int) __attribute__((noinline));
int returns_value(void) __attribute__((noinline));

/* Define them to avoid linkage errors */
void external_func(void) {
    /* Empty but non-inlineable */
    asm volatile("" : : : "memory");
}

void another_external(int x) {
    /* Clobber registers */
    asm volatile("" : : "r"(x) : "eax", "ecx", "edx", "memory");
}

int returns_value(void) {
    return 42;
}

/* Scenario 1: Explicit register variables with loop */
int scenario1(void) __attribute__((noinline, optimize("O2")));
int scenario1(void) {
    /* Force use of call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c;
        
        /* More arithmetic to create dense block */
        if (i & 1) {
            a += returns_value();
        } else {
            b -= returns_value();
        }
        
        /* Another call with different register pressure */
        another_external(sum);
    }
    
    /* Force use at end of basic block */
    if (sum > 100) {
        external_func();
        return sum + a + b;  /* All registers live here */
    }
    return sum + c;
}

/* Scenario 2: Volatile variables forcing memory spills */
int scenario2(void) __attribute__((noinline, optimize("O3")));
int scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int result = 0;
    
    /* Multiple volatile operations around calls */
    for (int i = 0; i < 8; i++) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call with volatile values live */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v3;
        
        /* Conditional with call at end of block */
        if (v4 > 100) {
            another_external(v1);
            /* This creates a basic block ending with call then jump */
            goto compute;
        }
        
        /* More operations */
        v1++;
        v2--;
    }
    
compute:
    result = v1 + v2 + v3 + v4;
    
    /* Switch with calls in cases */
    switch (result % 4) {
        case 0:
            external_func();
            result += 10;
            break;  /* Creates jump at end of basic block */
        case 1:
            v1 = returns_value();
            another_external(v1);
            result += 20;
            break;
        case 2:
            external_func();
            result += 30;
            /* Fall through */
        default:
            another_external(result);
            result += 40;
            break;
    }
    
    return result;
}

/* Scenario 3: Nested calls with register pressure */
int scenario3(void) __attribute__((noinline, optimize("O2")));
int scenario3(void) {
    int x = 1, y = 2, z = 3;
    
    /* Arguments in registers for outer call */
    x = returns_value() + x;
    
    /* Nested call pattern */
    for (int i = 0; i < 5; i++) {
        /* Setup for inner call clobbers registers */
        int a = x + i;
        int b = y * i;
        
        /* Inner call with register arguments */
        another_external(a + b);
        
        /* Outer call with same registers potentially live */
        x = returns_value() + a;
        y = returns_value() + b;
        
        /* Complex conditional with call at end */
        if (x > y) {
            external_func();
            z = x - y;  /* Live after call */
        } else {
            another_external(z);
            z = y - x;
        }
        
        /* Label for computed goto */
        if (i == 3) {
            z += 100;
        }
    }
    
    return x + y + z;
}

/* Scenario 4: setjmp/longjmp with calls */
jmp_buf env;
int scenario4(void) __attribute__((noinline, optimize("O3")));
int scenario4(void) {
    volatile int counter = 0;
    register int r1 __asm__ ("eax") = 10;
    register int r2 __asm__ ("ecx") = 20;
    
    if (setjmp(env) == 0) {
        /* First time through */
        for (int i = 0; i < 3; i++) {
            r1 += i;
            r2 -= i;
            
            /* Call with registers live */
            external_func();
            
            counter++;
            
            /* Use registers after call */
            if (r1 > r2) {
                another_external(r1);
            }
        }
    } else {
        /* After longjmp - registers may need restoration */
        r1 *= 2;
        r2 /= 2;
        external_func();
    }
    
    /* Simulate longjmp call */
    if (counter == 3) {
        longjmp(env, 1);
    }
    
    return r1 + r2 + counter;
}

/* Scenario 5: Computed goto with calls */
int scenario5(void) __attribute__((noinline, optimize("O2")));
int scenario5(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int val = 0;
    register int a __asm__ ("eax") = 5;
    register int b __asm__ ("ecx") = 10;
    
    for (int i = 0; i < 10; i++) {
        a += i;
        b -= i;
        
        /* Call with registers live */
        if (i % 2 == 0) {
            external_func();
        } else {
            another_external(a);
        }
        
        /* Computed goto based on value */
        int idx = (a + b) % 4;
        goto *labels[idx];
        
    label0:
        val += 1;
        continue;
    label1:
        val += 2;
        external_func();  /* Call at end of basic block before continue */
        continue;
    label2:
        val += 3;
        another_external(b);
        continue;
    label3:
        val += 4;
        /* Fall through to next iteration */
    }
    
    return val + a + b;
}

/* Main function to run all scenarios */
int main(void) {
    int result = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    result += scenario1();
    printf("Scenario1 completed: %d\n", result);
    
    result += scenario2();
    printf("Scenario2 completed: %d\n", result);
    
    result += scenario3();
    printf("Scenario3 completed: %d\n", result);
    
    result += scenario4();
    printf("Scenario4 completed: %d\n", result);
    
    result += scenario5();
    printf("Scenario5 completed: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return 0;
}
