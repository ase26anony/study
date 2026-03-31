/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will be called */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : "memory");
}

/* Function with complex register usage and calls */
int __attribute__((noinline)) compute_with_saves(int *arr, int n, void (*fp)(int*)) {
    /* Use explicit register variables to target call-clobbered registers */
    register long r10 asm("r10") = arr[0];
    register long r11 asm("r11") = arr[1];
    register long r12 asm("r12") = arr[2];  /* Call-saved register */
    register long r13 asm("r13") = arr[3];  /* Call-saved register */
    register long rax_val asm("rax") = arr[4];
    register long rcx_val asm("rcx") = arr[5];
    
    volatile int barrier = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Complex arithmetic keeping values live in registers */
        r10 = (r10 * 1103515245 + 12345) & 0x7fffffff;
        r11 = (r11 * 1103515245 + 12345) & 0x7fffffff;
        rax_val = (rax_val * 1103515245 + 12345) & 0x7fffffff;
        rcx_val = (rcx_val * 1103515245 + 12345) & 0x7fffffff;
        
        /* Mix with call-saved registers */
        r12 = r12 ^ r10;
        r13 = r13 ^ r11;
        
        /* Force spill/reload around call with volatile */
        barrier = r10 + r11;
        
        /* Call external function - forces caller-save for call-clobbered regs */
        fp(&barrier);
        
        /* Use values after call - they need to be restored */
        r10 = r10 ^ barrier;
        r11 = r11 ^ barrier;
        rax_val = rax_val ^ barrier;
        rcx_val = rcx_val ^ barrier;
        
        /* More mixing */
        r12 = r12 + rax_val;
        r13 = r13 + rcx_val;
        
        /* Store intermediate results back to array */
        arr[i % 8] = r10 + r11 + rax_val + rcx_val + r12 + r13;
    }
    
    /* Final computation using all registers */
    return (r10 + r11 + rax_val + rcx_val + r12 + r13) & 0xffff;
}

/* Another function with different pattern */
int __attribute__((noinline)) nested_loop_computation(int *arr, int size) {
    int sum = 0;
    void (*funcs[3])(int*) = {external_func1, external_func2, external_func3};
    
    for (int outer = 0; outer < 3; ++outer) {
        register long r14 asm("r14") = arr[outer * 2];
        register long r15 asm("r15") = arr[outer * 2 + 1];
        register long rbx_val asm("rbx") = arr[outer * 2 + 2];
        
        for (int inner = 0; inner < size; ++inner) {
            /* Create live values across calls */
            r14 = r14 * 6364136223846793005ULL + 1;
            r15 = r15 * 6364136223846793005ULL + 1;
            rbx_val = rbx_val * 6364136223846793005ULL + 1;
            
            /* Inline assembly to prevent optimization */
            asm volatile("" : "+r"(r14), "+r"(r15) : : "memory");
            
            /* Call with function pointer - inhibits optimizations */
            int temp = r14 + r15 + rbx_val;
            funcs[inner % 3](&temp);
            
            /* Values must survive across call */
            r14 = r14 ^ temp;
            r15 = r15 ^ temp;
            rbx_val = rbx_val ^ temp;
            
            sum += temp;
            
            /* Force register pressure with many temporaries */
            {
                register long t1 asm("r8") = r14 * 3;
                register long t2 asm("r9") = r15 * 5;
                register long t3 asm("r10") = rbx_val * 7;
                asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3));
                sum += t1 + t2 + t3;
            }
        }
        
        arr[outer * 2] = r14;
        arr[outer * 2 + 1] = r15;
    }
    
    return sum;
}

int main() {
    /* Initialize data array */
    int data[256];
    for (int i = 0; i < 256; ++i) {
        data[i] = i * 1103515245 + 12345;
    }
    
    /* Array of function pointers for indirect calls */
    void (*func_array[3])(int*) = {external_func1, external_func2, external_func3};
    
    int result1 = 0, result2 = 0;
    
    /* First computation pattern */
    for (int iter = 0; iter < 10; ++iter) {
        result1 ^= compute_with_saves(data, 50, func_array[iter % 3]);
        
        /* Shuffle data to create different patterns */
        for (int i = 0; i < 255; ++i) {
            data[i] = data[i] ^ data[i + 1];
        }
    }
    
    /* Second computation pattern with nested loops */
    int data2[64];
    for (int i = 0; i < 64; ++i) {
        data2[i] = i * 6364136223846793005ULL;
    }
    
    result2 = nested_loop_computation(data2, 20);
    
    /* Final result */
    int final_result = result1 ^ result2;
    
    /* Use the result to prevent dead code elimination */
    volatile int output __attribute__((unused)) = final_result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
