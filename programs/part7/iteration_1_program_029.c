/* test_caller_save.c - Program to trigger GCC caller-save pass instruction list manipulation */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but not inlineable */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    /* Clobber multiple registers */
    asm volatile("" : : "r"(x) : "eax", "ecx", "edx", "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Function call clobbers our registers */
        external_func();
        
        /* More operations after call - values must be restored */
        result += a * b;
        result -= c;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            a += result;
            external_func();  /* Another call in different basic block */
        } else if (i % 5 == 0) {
            b -= result;
            another_external(c);
        }
        
        /* Mix with volatile to prevent optimization */
        global_counter++;
    }
    
    return result + a + b + c;
}

/* Scenario 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 10, v2 = 20, v3 = 30, v4 = 40;
    volatile int v5 = 50, v6 = 60, v7 = 70, v8 = 80;
    int sum = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 15; ++i) {
        v1 = v1 + v2;
        v3 = v4 - v1;
        
        /* Call that forces spills */
        external_func();
        
        v5 = v6 * v7;
        v8 = v8 / (v1 + 1);
        
        sum += v1 + v3 + v5 + v8;
        
        /* Conditional with call at end of basic block */
        if (sum > 1000) {
            another_external(v2);
            /* Basic block ends with this call potentially */
            goto update;
        }
        
        v2 += i;
        update:
        v7 -= 2;
    }
    
    /* Switch statement creating complex basic block structure */
    switch (sum % 4) {
        case 0:
            v1 += external_func(), sum;  /* Comma operator with call */
            break;
        case 1:
            sum += v2;
            another_external(v3);
            break;  /* Jump at end of basic block after call */
        case 2:
            external_func();
            /* Fall through */
        default:
            sum = sum * 2;
            another_external(sum);
            /* BB_END might be the call instruction */
            break;
    }
    
    return sum + v1 + v8;
}

/* Scenario 3: Nested calls with register pressure */
int __attribute__((noinline)) scenario3(int x) {
    /* Use explicit asm to bind to registers */
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    register int r3 __asm__ ("edx") = x * 3;
    
    /* Outer call setup uses call-clobbered registers */
    r1 = r1 * r2 + 12345;
    r2 = r2 ^ r3;
    
    /* Nested call scenario */
    external_func();  /* First call clobbers registers */
    
    /* Arguments for next call in clobbered registers */
    r3 = r1 + r2;
    
    /* This call's arguments depend on values that need saving */
    another_external(r3);
    
    /* More computation */
    for (int i = 0; i < 8; ++i) {
        r1 += i;
        if (i % 2 == 0) {
            external_func();
            r2 -= i;
            /* Basic block with call in middle, jump at end */
            goto loop_end;
        }
        r3 *= 2;
        loop_end:
        ;
    }
    
    return r1 + r2 * 3 - r3;
}

/* Scenario 4: setjmp/longjmp with calls */
int __attribute__((noinline)) scenario4(void) {
    volatile int saved = 0;
    int ret = setjmp(jump_buffer);
    
    if (ret == 0) {
        /* First time through */
        register int j1 __asm__ ("eax") = 100;
        register int j2 __asm__ ("ecx") = 200;
        
        /* Do work with register variables */
        for (int i = 0; i < 5; ++i) {
            j1 += i * 10;
            j2 -= i * 5;
            
            /* Call that might longjmp */
            if (i == 3) {
                external_func();
                saved = j1 + j2;  /* Must be saved across potential longjmp */
            }
        }
        
        /* Another call */
        another_external(j1);
        
        return saved + j2;
    } else {
        /* After longjmp */
        return ret * 2;
    }
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    
    register int g1 __asm__ ("eax") = selector * 10;
    register int g2 __asm__ ("ecx") = selector * 20;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
    label0:
        g1 += 5;
        external_func();  /* Call in basic block with goto edge */
        g2 -= 3;
        result = g1 + g2;
        goto end;
    
    label1:
        g1 *= 2;
        another_external(g1);
        result = g1 - g2;
        /* Fall through to label2 */
    
    label2:
        external_func();
        result += 100;
        if (g2 > 50) {
            goto end;
        }
        g2 += 20;
        goto label3;
    
    label3:
        g1 = g1 ^ g2;
        another_external(result);
        result = g1 * 2;
        /* End of basic block */
    
    end:
    return result;
}

/* Helper to force register spilling */
void __attribute__((noinline)) force_spill(int* arr, int n) {
    register int s1 __asm__ ("eax") = arr[0];
    register int s2 __asm__ ("ecx") = arr[1];
    register int s3 __asm__ ("edx") = arr[2];
    
    for (int i = 0; i < n; ++i) {
        s1 = s1 * s2 + i;
        s2 = s2 - s3;
        
        /* Multiple calls in loop */
        if (i % 2 == 0) {
            external_func();
        } else {
            another_external(s1);
        }
        
        s3 = s3 ^ s2;
        arr[i % 3] = s1 + s2 + s3;
    }
}

/* Main function that exercises all scenarios */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save scenarios...\n");
    
    /* Run each scenario multiple times */
    for (int i = 0; i < 3; ++i) {
        total += scenario1();
        total += scenario2();
        total += scenario3(i);
        total += scenario4();
        total += scenario5(i);
    }
    
    /* Additional test with array */
    int arr[3] = {1, 2, 3};
    force_spill(arr, 10);
    total += arr[0] + arr[1] + arr[2];
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
