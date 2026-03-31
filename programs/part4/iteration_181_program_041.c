/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -c early-remat-trigger.c
 */

/* Dummy functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink + 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 1.1;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    /* Large arrays to create memory references */
    int array1[1024];
    double array2[1024];
    volatile int accumulator = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
    }
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < 10000; outer++) {
        /* Create many live scalar variables - high register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = v1 - v2;
        int v4 = v2 * 3;
        int v5 = v3 + v4;
        double d1 = outer * 0.1;
        double d2 = d1 * 2.0;
        double d3 = d2 - 0.5;
        double d4 = d3 * d1;
        int v6 = v5 % 7;
        int v7 = v6 + 11;
        double d5 = d4 + 1.0;
        int v8 = v7 * 2;
        double d6 = d5 * 0.8;
        int v9 = v8 - 3;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % 1024;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array with offset */
        double val2 = array2[(key_index + 5) % 1024];
        
        /* Third use: conditional check */
        if (key_index > 512) {
            /* Use key_index again inside branch */
            int val3 = array1[key_index / 2];
            v1 += val3;
            
            /* More computations to increase pressure */
            d1 += val2;
            v9 += key_index;  /* Use key_index again */
        } else {
            /* Alternative path also using key_index */
            double val4 = array2[key_index * 2 % 1024];
            d2 += val4;
            v8 += key_index % 17;  /* Another use */
        }
        
        /* Fourth use: passed to dummy function */
        use_int(key_index);
        
        /* Complex control flow with goto to create cycles */
        if (v1 % 3 == 0) {
            /* Small inner loop to create data flow cycles */
            int inner_sum = 0;
            for (int j = 0; j < 3; j++) {
                /* Use key_index inside inner loop */
                inner_sum += key_index + j;
                /* Mix data types */
                d3 += key_index * 0.01;
            }
            v2 += inner_sum;
            
            /* goto to create non-trivial CFG */
            if (v2 > 100) {
                goto special_case;
            }
        }
        
        /* Fifth use: in pointer calculation */
        void *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Continue normal execution */
        v3 = v1 + v2;
        goto continue_main;
        
    special_case:
        /* Sixth use: in special case block */
        v4 = key_index * 2;
        accumulator += v4;
        
    continue_main:
        /* Seventh use: final computation */
        int final_use = key_index % 19;
        accumulator ^= final_use;
        
        /* Use all variables to keep them live */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        volatile double dsink = d1 + d2 + d3 + d4 + d5 + d6;
        (void)dsink;
        
        /* Switch statement to split uses across multiple blocks */
        switch (key_index % 5) {
            case 0:
                accumulator += array1[key_index % 100];
                break;
            case 1:
                accumulator -= key_index;
                break;
            case 2:
                accumulator ^= (key_index << 3);
                break;
            case 3:
                /* Eighth use: in another array access */
                array1[key_index] = accumulator;
                break;
            case 4:
                /* Ninth use: complex expression */
                accumulator += (key_index * key_index) % 31;
                break;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int result = accumulator;
    return result % 256;
}
