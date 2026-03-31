/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func1(volatile int* p) {
    *p += 1;
    /* Use inline assembly to clobber specific registers */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

int main(void) {
    /* Large array to work with */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + global_seed;
    }
    
    /* Function pointers to inhibit optimization */
    func_ptr_t funcs[2] = {external_func1, external_func2};
    
    /* Result accumulator */
    int result = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to create pressure on call-clobbered regs */
        register long r10_val asm("r10") = data[outer] * 3;
        register long r11_val asm("r11") = data[outer + 1] * 7;
        register long r9_val asm("r9") = data[outer + 2] * 11;
        
        /* Inner loop with function calls and register-intensive calculations */
        for (int inner = 0; inner < 20; inner++) {
            /* Create more register pressure with additional values */
            register long rax_val asm("rax") = data[inner] + r10_val;
            register long rcx_val asm("rcx") = data[inner + 1] + r11_val;
            register long rdx_val asm("rdx") = data[inner + 2] + r9_val;
            
            /* Volatile variables to prevent reordering/elimination */
            volatile int temp1 = rax_val;
            volatile int temp2 = rcx_val;
            volatile int temp3 = rdx_val;
            
            /* Perform arithmetic keeping values live across calls */
            rax_val = rax_val * 3 + rcx_val;
            rcx_val = rcx_val * 5 + rdx_val;
            rdx_val = rdx_val * 7 + rax_val;
            
            /* Use inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
            
            /* Call external function via pointer - inhibits optimizations */
            funcs[inner % 2](&temp1);
            
            /* More calculations after call, using values that must survive */
            r10_val = rax_val + r10_val - temp1;
            r11_val = rcx_val + r11_val - temp2;
            r9_val = rdx_val + r9_val - temp3;
            
            /* Another call with different pattern */
            if (inner % 3 == 0) {
                external_func1(&temp2);
            } else {
                external_func2(&temp3);
            }
            
            /* Final computation using all live values */
            result += r10_val + r11_val + r9_val + rax_val + rcx_val + rdx_val;
            
            /* Use inline assembly to prevent dead code elimination */
            asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r9_val));
        }
        
        /* Store results back to array, creating memory dependencies */
        data[outer % 256] = result;
        data[(outer + 1) % 256] = r10_val;
        data[(outer + 2) % 256] = r11_val;
        data[(outer + 3) % 256] = r9_val;
        
        /* Mix in some pointer arithmetic */
        volatile int* ptr = &data[outer % 256];
        *ptr += global_seed;
    }
    
    /* Additional complex loop with nested function calls */
    for (int i = 0; i < 100; i++) {
        /* Multiple values in call-clobbered registers */
        register long x asm("rax") = data[i] * i;
        register long y asm("rcx") = data[i + 1] * (i + 1);
        register long z asm("rdx") = data[i + 2] * (i + 2);
        
        /* Chain of calculations with calls in between */
        for (int j = 0; j < 5; j++) {
            x = x + y + z;
            external_func1((volatile int*)&x);
            y = y + x + z;
            external_func2((volatile int*)&y);
            z = z + x + y;
            
            /* Use function pointer */
            funcs[j % 2]((volatile int*)&z);
            
            /* Force spill/reload around calls */
            asm volatile("" : "+r"(x), "+r"(y), "+r"(z));
        }
        
        result += x + y + z;
        data[i % 256] = x;
    }
    
    printf("Result: %d\n", result);
    
    /* Verify computation wasn't optimized away */
    volatile int check = result;
    asm volatile("" : : "r"(check));
    
    return result != 0 ? 0 : 1;
}
