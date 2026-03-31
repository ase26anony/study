#include <stdio.h>
#include <stdlib.h>

/* Global arrays with potential aliasing */
extern int global_arr1[1024];
extern int global_arr2[1024];
int global_arr1[1024];
int global_arr2[1024];

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) void memory_barrier() {
    asm volatile("" ::: "memory");
}

/* Pure function for scheduling interest */
__attribute__((const)) int pure_func(int a, int b) {
    return (a * b) >> 1;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* data, int size, int seed) {
    register int acc = seed;  /* Hint at register allocation */
    int temp;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < size; ++i) {
        /* Conditional inner loop execution */
        if (__builtin_expect(i > size/2, 0)) {
            /* Inner loop with different data type */
            for (unsigned short j = 0; j < 8; ++j) {
                /* Mix of operations */
                temp = data[i] + (int)j;
                acc = pure_func(temp, acc);
                
                /* Memory barrier creates scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* Conditional branch */
                if (j % 3 == 0) {
                    acc += non_inline_func(i, j);
                }
            }
        } else {
            /* Different computation path */
            for (unsigned k = 0; k < 4; ++k) {
                /* Memory access with potential aliasing */
                acc = global_arr1[i] + acc;
                acc ^= global_arr2[k];
                
                /* Function call with loop-variant arguments */
                acc += pure_func(i, k);
            }
        }
        
        /* Another conditional inner loop */
        if (i % 5 == 0) {
            int local_var = acc;
            for (short m = 0; m < 3; ++m) {
                local_var = (local_var << 1) | (local_var >> 31);
                acc += local_var;
            }
            memory_barrier();
        }
    }
    
    return acc;
}

/* Warm-up function */
__attribute__((noinline)) void warm_up_computation() {
    volatile int warm_up_data[16];
    int warm_up_acc = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 16; ++i) {
        warm_up_data[i] = i;
        warm_up_acc += warm_up_data[i];
        if (i % 4 == 0) {
            warm_up_acc = pure_func(warm_up_acc, i);
        }
    }
    
    /* Prevent optimization */
    asm volatile("" ::: "memory");
}

/* Main computation with nested loops */
__attribute__((noinline)) int main_computation(int N, int M) {
    int result = 0;
    int local_arr[256];
    
    /* Initialize local array */
    for (int i = 0; i < 256; ++i) {
        local_arr[i] = i * 3 + 1;
    }
    
    /* Complex nested loop structure */
    for (int outer = 0; outer < N; ++outer) {
        int mid_acc = outer;
        
        /* Middle loop with different type */
        for (unsigned mid = 0; mid < (unsigned)M; ++mid) {
            int inner_acc = 0;
            
            /* Innermost loop - main target for scheduling */
            for (short inner = 0; inner < 32; ++inner) {
                /* Multiple operations with dependencies */
                int val1 = local_arr[(outer + inner) & 0xFF];
                int val2 = global_arr1[(mid + inner) & 0x3FF];
                
                /* Loop-carried dependency */
                inner_acc = val1 * val2 + inner_acc;
                
                /* Conditional with function call */
                if (__builtin_expect((inner + outer) % 7 == 0, 0)) {
                    inner_acc = non_inline_func(inner_acc, val1);
                }
                
                /* Memory operation */
                global_arr2[inner & 0x3FF] ^= inner_acc;
                
                /* Another scheduling boundary */
                asm volatile("" ::: "memory");
            }
            
            mid_acc += inner_acc;
            
            /* Variable scope stress */
            {
                int scope_var = mid_acc;
                scope_var = pure_func(scope_var, mid);
                mid_acc = scope_var;
            }
        }
        
        result += mid_acc;
        
        /* Conditional loop execution */
        if (outer % 3 == 0) {
            for (int k = 0; k < 8; ++k) {
                result = (result << 1) | (result >> 31);
                result += k;
            }
        }
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Use arguments for loop bounds to prevent optimization */
    int N = (argc > 1) ? atoi(argv[1]) : 10;
    int M = (argc > 2) ? atoi(argv[2]) : 15;
    
    /* Initialize global arrays with pseudo-random values */
    unsigned int lcg_seed = 12345;
    for (int i = 0; i < 1024; ++i) {
        lcg_seed = lcg_seed * 1103515245 + 12345;
        global_arr1[i] = (lcg_seed >> 16) & 0x7FFF;
        global_arr2[i] = (lcg_seed >> 8) & 0xFF;
    }
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computation */
    int checksum = main_computation(N, M);
    
    /* Additional computation with different patterns */
    checksum += compute_checksum(global_arr1, 256, checksum);
    
    /* Print verifiable result */
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
