/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Force register clobbering with inline assembly */
    /* x86_64 specific - clobber common caller-saved registers */
#if defined(__x86_64__) || defined(__i386__)
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    /* ARM64 specific - clobber caller-saved registers */
    asm volatile (
        "# Clobber caller-saved registers on ARM64\n\t"
        "mov x0, #0\n\t"
        "mov x1, #0\n\t"
        "mov x2, #0\n\t"
        "mov x3, #0\n\t"
        "mov x4, #0\n\t"
        "mov x5, #0\n\t"
        "mov x6, #0\n\t"
        "mov x7, #0\n\t"
        "mov x8, #0\n\t"
        "mov x9, #0\n\t"
        "mov x10, #0\n\t"
        "mov x11, #0\n\t"
        "mov x12, #0\n\t"
        "mov x13, #0\n\t"
        "mov x14, #0\n\t"
        "mov x15, #0\n\t"
        "mov x16, #0\n\t"
        "mov x17, #0\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-inline")))
int caller_function(int x, int y) {
    /* Declare many local variables to create register pressure */
    register int a = x + 1;
    register int b = y * 2;
    register int c = x * y + 3;
    register int d = y - x + 4;
    register int e = (x << 2) + 5;
    register int f = (y >> 1) + 6;
    register int g = x * x + y * y + 7;
    register int h = (x + y) * 3 + 8;
    register int i = x ^ y + 9;
    register int j = ~x + y + 10;
    
    /* Pre-call computation using all variables */
    a = b + c * 2;
    b = d - e / 3;
    c = f * g + h;
    d = i ^ j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    /* This creates a basic block boundary opportunity */
    if (global_flag) {
        /* Additional computation before call in the same block */
        e = a + b;
        f = c - d;
        
        /* The call that needs caller-save handling */
        callee_function();
        
        /* Post-call computation - variables still live */
        g = e * f + 1;
        h = g - a;
    } else {
        /* Alternative path without call */
        g = a * b;
        h = c + d;
    }
    
    /* More computation ensuring all variables are live */
    i = g + h * 2;
    j = i - (a + b + c + d);
    
    /* Complex return value using all variables */
    /* This ensures they can't be optimized away */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + 
           f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Another caller to create more optimization context */
__attribute__((noipa, noinline))
int secondary_caller(int val) {
    int result = 0;
    for (int i = 0; i < 3; i++) {
        /* Mix of conditions to create control flow complexity */
        if (val & (1 << i)) {
            result += caller_function(val, i);
        } else {
            result -= caller_function(i, val);
        }
    }
    return result;
}

int main(void) {
    int total = 0;
    
    /* Vary the flag to create different execution paths */
    for (int i = 0; i < 10; i++) {
        global_flag = i & 1;  /* Alternate between 0 and 1 */
        
        /* Call with different arguments to prevent constant propagation */
        int result = caller_function(i * 3 + 1, i * 2 + 5);
        
        /* Also call secondary caller */
        result += secondary_caller(i + 2);
        
        total += result;
        
        /* Use result to prevent dead code elimination */
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    /* Additional test with different register usage patterns */
    {
        /* Force spill/fill behavior */
        volatile int seed = 42;
        srand(seed);
        
        for (int k = 0; k < 5; k++) {
            int x = rand() % 100;
            int y = rand() % 100;
            
            /* Temporarily change flag */
            global_flag = (x > y);
            
            int res = caller_function(x, y);
            printf("Random test %d: x=%d, y=%d, flag=%d, result=%d\n", 
                   k, x, y, global_flag, res);
            
            total += res;
        }
    }
    
    printf("Final total: %d\n", total);
    return total > 0 ? 0 : 1;
}
