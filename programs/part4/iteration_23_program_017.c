/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to force calls */
void __attribute__((noinline, noclone)) opaque_func1(int, int, int, int, int, int);
void __attribute__((noinline, noclone)) opaque_func2(double, double, double);
void __attribute__((noinline, noclone)) opaque_func3(void*, void*, void*);
int __attribute__((noinline, noclone)) opaque_func4(int, int, int, int, int, int, int, int);

/* Volatile globals to prevent optimization */
volatile int gv1 = 1, gv2 = 2, gv3 = 3, gv4 = 4, gv5 = 5, gv6 = 6;
volatile double gd1 = 1.0, gd2 = 2.0, gd3 = 3.0;
volatile void* gp1 = (void*)0x1000, gp2 = (void*)0x2000, gp3 = (void*)0x3000;

/* Function pointer with volatile to prevent devirtualization */
void (* volatile fp1)(int, int, int, int, int, int) = opaque_func1;
void (* volatile fp2)(double, double, double) = opaque_func2;

/* Test 1: Many integer arguments with register pressure around call */
void __attribute__((noinline, noclone)) test1(int mode) {
    /* Use explicit register variables to create conflicts */
    register int r10_val asm("r10") = gv1 + 1;
    register int r11_val asm("r11") = gv2 + 2;
    register int r12_val asm("r12") = gv3 + 3;
    register int r13_val asm("r13") = gv4 + 4;
    register int r14_val asm("r14") = gv5 + 5;
    register int r15_val asm("r15") = gv6 + 6;
    
    /* Volatile array to force spills */
    volatile int stack_save[20];
    for (int i = 0; i < 20; i++) {
        stack_save[i] = i + mode;
    }
    
    /* Inline asm to clobber call-clobbered registers */
    asm volatile("" : : : "eax", "ecx", "edx", "r8", "r9", "r10", "r11");
    
    /* Complex control flow with goto to split basic blocks */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Function call with many arguments - forces caller-save */
    opaque_func1(r10_val, r11_val, r12_val, r13_val, r14_val, r15_val);
    
    /* Use saved values immediately after call */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += stack_save[i];
    }
    
    /* Another asm barrier */
    asm volatile("" : : : "memory");
    
    /* Use register variables again */
    gv1 = r10_val + sum;
    goto end;
    
label2:
    /* Alternative path with different call pattern */
    opaque_func1(r15_val, r14_val, r13_val, r12_val, r11_val, r10_val);
    
    /* Force register reload */
    asm volatile("" : : : "rax", "rcx", "rdx");
    
    gv2 = r11_val * 2;
    
end:
    /* Complex expression using all register variables */
    gv3 = (r10_val + r11_val) * (r12_val - r13_val) / (r14_val | r15_val);
}

/* Test 2: Floating point with mixed usage */
void __attribute__((noinline, noclone)) test2(int mode) {
    volatile double local_doubles[10];
    for (int i = 0; i < 10; i++) {
        local_doubles[i] = gd1 + i * gd2;
    }
    
    /* Switch with default that calls function */
    switch (mode % 4) {
        case 0:
            opaque_func2(gd1, gd2, gd3);
            break;
        case 1:
            opaque_func2(gd3, gd1, gd2);
            break;
        case 2:
            opaque_func2(gd2, gd3, gd1);
            break;
        default: {
            /* Nested call scenario */
            void (* volatile local_fp)(double, double, double) = fp2;
            local_fp(local_doubles[0], local_doubles[1], local_doubles[2]);
            
            /* Inline asm that looks like a call */
            asm volatile(
                "movq %0, %%rax\n\t"
                "movq %1, %%rbx\n\t"
                "movq %2, %%rcx\n\t"
                : 
                : "r"(gp1), "r"(gp2), "r"(gp3)
                : "rax", "rbx", "rcx", "memory"
            );
            break;
        }
    }
    
    /* Use saved values after call */
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        sum += local_doubles[i];
    }
    gd1 = sum;
}

/* Test 3: Pointer arguments with irreducible control flow */
void __attribute__((noinline, noclone)) test3(int mode) {
    /* Create loop with break inside conditional with call */
    int counter = 0;
    volatile int results[5];
    
    while (counter < 5) {
        if (mode & (1 << counter)) {
            /* Function call inside loop with pointer args */
            opaque_func3(gp1 + counter, gp2 + counter, gp3 + counter);
            
            /* Force spill/restore around call */
            asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            results[counter] = counter * 2;
            if (results[counter] > 6) {
                break;  /* Causes basic block boundary */
            }
        } else {
            /* Alternative path */
            results[counter] = counter * 3;
        }
        
        /* Nested conditional with goto */
        if (counter == 3) {
            goto early_exit;
        }
        
        counter++;
        continue;
        
    early_exit:
        /* Call with register pressure */
        int r = opaque_func4(counter, counter+1, counter+2, counter+3,
                            counter+4, counter+5, counter+6, counter+7);
        results[4] = r;
        break;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    gv4 = sum;
}

/* Test 4: Mixed types and __builtin_apply */
void __attribute__((noinline, noclone)) test4(int mode) {
    /* Use __builtin_apply to create unusual call sequence */
    typedef int (*func_t)(int, int, int, int, int, int, int, int);
    func_t f = opaque_func4;
    
    /* Build argument array */
    __builtin_apply_args();
    
    /* Register variables with explicit registers */
    register long rax_save asm("rax");
    register long rdx_save asm("rdx");
    
    /* Save before potential call */
    asm volatile("mov %%rax, %0" : "=r"(rax_save));
    asm volatile("mov %%rdx, %0" : "=r"(rdx_save));
    
    /* Complex expression that needs temps */
    int complex_val = (gv1 * gv2) + (gv3 / gv4) - (gv5 ^ gv6);
    
    /* Call via function pointer */
    if (mode > 0) {
        int result = f(complex_val, gv1, gv2, gv3, gv4, gv5, gv6, mode);
        
        /* Restore and use saved registers */
        asm volatile("mov %0, %%rax" : : "r"(rax_save));
        asm volatile("mov %0, %%rdx" : : "r"(rdx_save));
        
        /* Use in computation */
        gv5 = result + (int)rax_save + (int)rdx_save;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* Helper with nested call */
void __attribute__((noinline, noclone)) nested_helper(int depth, int* result) {
    volatile int local = depth * 2;
    
    if (depth > 0) {
        /* Recursive call */
        nested_helper(depth - 1, result);
        
        /* Use value after nested call */
        *result += local;
        
        /* Inline asm between calls */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
        
        /* Another call */
        opaque_func1(local, local+1, local+2, local+3, local+4, local+5);
    } else {
        /* Base case with call */
        opaque_func1(1, 2, 3, 4, 5, 6);
        *result = 1;
    }
}

/* Main test driver */
int main(int argc, char** argv) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Initialize function pointers with some pattern */
    fp1 = opaque_func1;
    fp2 = opaque_func2;
    
    /* Run all tests in sequence with mode variations */
    test1(mode);
    test2(mode + 1);
    test3(mode + 2);
    test4(mode + 3);
    
    /* Test with nested calls */
    int nested_result = 0;
    for (int i = 0; i < 3; i++) {
        nested_helper(2 + (mode % 3), &nested_result);
        gv6 += nested_result;
    }
    
    /* Compute final checksum to prevent elimination */
    int checksum = gv1 + gv2 + gv3 + gv4 + gv5 + gv6;
    checksum += (int)gd1 + (int)gd2 + (int)gd3;
    checksum += (long)gp1 + (long)gp2 + (long)gp3;
    
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_func1(int a, int b, int c, int d, int e, int f) {
    /* Empty but compiler doesn't know */
}

void opaque_func2(double a, double b, double c) {
    /* Empty */
}

void opaque_func3(void* a, void* b, void* c) {
    /* Empty */
}

int opaque_func4(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b + c + d + e + f + g + h;
}
