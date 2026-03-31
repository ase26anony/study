/* Program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1000;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray1[ARRAY_SIZE];
    double darray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        darray1[i] = i * 0.5;
        darray2[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer - 1;
        int v5 = outer * 5;
        int v6 = outer + 2;
        int v7 = outer * 7;
        int v8 = outer - 2;
        int v9 = outer * 11;
        int v10 = outer + 3;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        double d6 = outer * 0.6;
        double d7 = outer * 0.7;
        double d8 = outer * 0.8;
        
        /* KEY COMPUTATION: Cheap to recompute expression */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple places - creates multiple DF_REFs */
        int val1 = array1[key_index];      /* First use */
        use_int(val1);
        
        double dval1 = darray1[key_index]; /* Second use, different mode */
        use_double(dval1);
        
        /* Force materialization with volatile */
        volatile int vol_idx = key_index;
        
        /* Complex control flow splitting uses of key_index */
        if (outer % 3 == 0) {
            /* Use key_index again in a different basic block */
            int val2 = array2[key_index];  /* Third use */
            accumulator ^= val2;
            
            /* Inner conditional creating more data flow */
            if (key_index % 2 == 0) {      /* Fourth use */
                darray2[key_index] += 1.0; /* Fifth use */
            } else {
                /* Small inner loop creating cyclic flow */
                for (int inner = 0; inner < 3; inner++) {
                    /* Use key_index inside inner loop */
                    array1[(key_index + inner) % ARRAY_SIZE] += inner; /* Sixth use */
                }
            }
            
            /* Use key_index after inner block */
            use_int(key_index * 2);        /* Seventh use */
        } else if (outer % 3 == 1) {
            /* Different path using key_index */
            double temp = darray2[key_index] * 2.0; /* Eighth use */
            use_double(temp);
            
            /* Switch statement to create more control flow */
            switch (key_index % 4) {       /* Ninth use */
                case 0:
                    array1[key_index] += v1; /* Tenth use */
                    break;
                case 1:
                    array2[key_index] += v2; /* Eleventh use */
                    break;
                default:
                    darray1[key_index] += d1; /* Twelfth use, different mode */
                    break;
            }
        } else {
            /* Third path with goto to create label */
            if (key_index < ARRAY_SIZE / 2) { /* Thirteenth use */
                goto process_lower_half;
            }
            
            /* Use key_index before goto */
            use_ptr(&array1[key_index]);   /* Fourteenth use */
            
            process_lower_half:
            /* Use key_index after label */
            int sum = key_index + v3 + v4; /* Fifteenth use */
            accumulator += sum;
        }
        
        /* Use all the pressure variables to keep them live */
        use_int(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
        use_double(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
        
        /* Final use of key_index in loop */
        if (outer % 100 == 0) {
            volatile int last_use = key_index; /* Sixteenth use */
            accumulator += last_use;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
