/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline,noipa)) 
long long test_remat(int iterations, int offset, double seed) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = seed + i * 0.5;
        arr_ll[i] = (long long)(seed * 1000) + i * offset;
        arr_int[i] = i * 3 + offset;
    }
    
    /* Complex expression results */
    double dbl_result = 0.0;
    long long ll_result = 0;
    v2df vec_result = {0.0, 0.0};
    
    /* Outer loop with multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile_value() % 10;
        
        /* Inner if with complex control flow */
        if (invariant > 3 && outer % 2 == 0) {
            /* REGISTER PRESSURE BLOCK - many intermediate values */
            
            /* Vector operations creating virtual registers */
            v2df vec1 = {arr_dbl[outer % 32], arr_dbl[(outer + 1) % 32]};
            v2df vec2 = {arr_dbl[(outer + 2) % 32], arr_dbl[(outer + 3) % 32]};
            
            /* __builtin_shuffle creates virtual registers */
            v2df shuffled = __builtin_shuffle(vec1, vec2, 
                (v4si){0, 1, 2, 3});
            
            /* Mix integer and floating-point operations */
            double temp1 = shuffled[0] * shuffled[1];
            double temp2 = arr_dbl[invariant % 32] * 2.5;
            
            /* More volatile barriers in the expression */
            int volatile_idx = get_volatile_value() % 32;
            double temp3 = arr_dbl[volatile_idx] / (temp1 + 1.0);
            
            /* Integer operations with different modes */
            long long ll_temp1 = arr_ll[outer % 32];
            long long ll_temp2 = (long long)(temp1 * 1000.0);
            long long ll_temp3 = ll_temp1 * ll_temp2;
            
            /* More vector operations */
            v2df vec3 = {temp2, temp3};
            v2df vec4 = __builtin_shuffle(vec3, shuffled, 
                (v4si){2, 3, 0, 1});
            
            /* Complex expression with many intermediates */
            double complex_expr = 
                (vec4[0] * vec4[1]) +
                (temp1 * temp2) +
                (temp3 * arr_dbl[(outer + 4) % 32]) -
                (double)(ll_temp3 % 1000) / 100.0;
            
            /* Use all the intermediate results */
            dbl_result += complex_expr;
            ll_result ^= ll_temp3;
            
            /* More register pressure */
            for (int inner = 0; inner < 4; inner++) {
                double inner_temp = arr_dbl[(outer + inner) % 32];
                long long inner_ll = arr_ll[(outer + inner) % 32];
                
                /* Mixed mode operations */
                dbl_result += inner_temp * (inner % 2 ? 1.5 : 2.5);
                ll_result += (long long)(inner_temp * 100.0) ^ inner_ll;
                
                /* Another volatile barrier */
                if (get_volatile_value() % 5 == 0) {
                    dbl_result -= arr_dbl[volatile_idx];
                }
            }
            
            /* Final complex expression with many live values */
            vec_result += vec4 * shuffled + 
                (v2df){complex_expr, dbl_result / (outer + 1)};
        }
        
        /* Alternate path to create more control flow complexity */
        else if (invariant <= 3 && outer % 3 == 0) {
            /* Different computation path still using many registers */
            double alt_temp1 = arr_dbl[offset % 32];
            double alt_temp2 = arr_dbl[(offset + 1) % 32];
            
            for (int i = 0; i < 8; i++) {
                alt_temp1 = alt_temp1 * alt_temp2 + 
                           arr_dbl[(offset + i) % 32];
                alt_temp2 = alt_temp2 / (i + 1.0) + 
                           arr_int[(offset + i) % 32];
            }
            
            dbl_result += alt_temp1 - alt_temp2;
        }
        
        /* Simple path to vary register usage patterns */
        else {
            ll_result += arr_ll[outer % 32] * outer;
            dbl_result += arr_dbl[outer % 32] / (outer + 1.0);
        }
        
        /* Cross-iteration dependency */
        if (outer > 0) {
            arr_dbl[outer % 32] = dbl_result / (outer + 100.0);
            arr_ll[outer % 32] = ll_result ^ outer;
        }
    }
    
    /* Final mixed-mode computation */
    return ll_result + (long long)(dbl_result * 1000.0) + 
           (long long)(vec_result[0] + vec_result[1]);
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 10), i % 8, 1.5 + i * 0.1);
        
        /* Prevent loop unrolling from simplifying register pressure */
        if (i % 7 == 0) {
            barrier_counter += i;
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
