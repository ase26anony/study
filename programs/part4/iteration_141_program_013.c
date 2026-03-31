/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Mix of call-clobbered and call-saved register usage */
long __attribute__((noinline)) compute_with_saves(int *data, int n, func_ptr_t fp) {
    /* Use explicit register variables for call-clobbered registers */
    register long r10 asm("r10") = 0;
    register long r11 asm("r11") = 0;
    register long r8 asm("r8") = 0;
    register long r9 asm("r9") = 0;
    
    /* Use call-saved registers too */
    long r12_val = 0, r13_val = 0, r14_val = 0, r15_val = 0;
    
    /* Volatile to prevent optimization */
    volatile int *volatile_data = data;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < n; i++) {
        /* Load into call-clobbered registers */
        r10 = volatile_data[i];
        r11 = volatile_data[i + 1];
        
        /* Arithmetic in call-clobbered registers */
        r8 = r10 * r11;
        r9 = r10 + r11;
        
        /* Keep values in call-saved registers live across calls */
        r12_val += r8;
        r13_val += r9;
        
        /* Inline assembly to create artificial dependencies */
        asm volatile("" : "+r"(r8), "+r"(r9));
        
        /* Call external function - forces caller-save for r8, r9, r10, r11 */
        if (i % 2 == 0) {
            long temp = r8 + r9;
            external_func1((int*)&temp);
            r14_val += temp;
        } else {
            long temp = r8 - r9;
            external_func2(&temp);
            r15_val += temp;
        }
        
        /* Use the values after call - they need to be restored */
        volatile_data[i] = (int)(r8 + r10);
        volatile_data[i + 1] = (int)(r9 + r11);
        
        /* Function pointer call with different targets */
        if (fp) {
            long tmp = r12_val + r13_val;
            fp(&tmp);
        }
        
        /* More arithmetic to keep registers busy */
        r10 = r10 * 3 + r8;
        r11 = r11 * 7 - r9;
        
        /* Another volatile asm to prevent reordering */
        asm volatile("" : : "r"(r10), "r"(r11));
    }
    
    /* Final computation using all registers */
    return r12_val + r13_val + r14_val + r15_val + r8 + r9 + r10 + r11;
}

/* Another function with different register pressure pattern */
long __attribute__((noinline)) compute_with_indirect(int *data, int n) {
    long sum = 0;
    
    /* Array of function pointers */
    func_ptr_t funcs[2] = { (func_ptr_t)external_func1, (func_ptr_t)external_func2 };
    
    /* Use multiple call-clobbered registers explicitly */
    register long rax_val asm("rax") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    for (int i = 0; i < n; i += 2) {
        /* Load and compute in call-clobbered registers */
        rax_val = data[i];
        rcx_val = data[i + 1];
        rdx_val = rax_val * rcx_val;
        
        /* Volatile to force memory access */
        volatile int v = data[i % n];
        
        /* Indirect call - compiler doesn't know which registers are clobbered */
        funcs[i % 2]((void*)&v);
        
        /* Use values after call - they need to be saved/restored */
        sum += rax_val + rcx_val + rdx_val + v;
        
        /* Modify data array */
        data[i] = (int)(rax_val + sum);
        data[i + 1] = (int)(rcx_val - sum);
        
        /* Rotate register values to create different live ranges */
        long temp = rax_val;
        rax_val = rcx_val;
        rcx_val = rdx_val;
        rdx_val = temp;
        
        /* Another call with different pattern */
        if (i % 3 == 0) {
            external_func1(&data[i]);
        } else {
            external_func2((long*)&data[i]);
        }
    }
    
    return sum + rax_val + rcx_val + rdx_val;
}

int main() {
    /* Initialize data array */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    long total = 0;
    
    /* First computation with direct function pointer */
    total += compute_with_saves(data, 100, (func_ptr_t)external_func1);
    
    /* Second computation with NULL function pointer */
    total += compute_with_saves(data + 100, 50, NULL);
    
    /* Third computation with indirect calls */
    total += compute_with_indirect(data + 150, 50);
    
    /* Additional loop with mixed operations */
    for (int iter = 0; iter < 10; iter++) {
        /* Use explicit register variables in main too */
        register long r10 asm("r10") = total;
        register long r11 asm("r11") = iter;
        
        for (int i = 0; i < 20; i++) {
            /* Computation in registers */
            r10 = r10 * 1103515245 + 12345;
            r11 = r11 * 1664525 + 1013904223;
            
            /* Call that clobbers registers */
            if (i % 3 == 0) {
                external_func1(&data[i]);
            }
            
            /* Use register values after call */
            data[i] ^= (int)(r10 ^ r11);
            
            /* Another call with different pattern */
            if (i % 4 == 0) {
                long temp = r10 - r11;
                external_func2(&temp);
                data[i] += (int)temp;
            }
            
            /* Inline asm to prevent optimization */
            asm volatile("" : "+r"(r10), "+r"(r11));
        }
        
        total = r10 + r11;
    }
    
    printf("Result: %ld\n", total);
    
    /* Verify data was modified */
    int check = 0;
    for (int i = 0; i < 256; i++) {
        check ^= data[i];
    }
    printf("Checksum: %d\n", check);
    
    return 0;
}
