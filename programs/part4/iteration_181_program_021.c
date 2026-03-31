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
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray1[ARRAY_SIZE];
    double darray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
        darray1[i] = i * 1.5;
        darray2[i] = i * 2.5;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer * 3;
        int v3 = outer * 5;
        int v4 = outer * 7;
        int v5 = outer * 11;
        int v6 = outer * 13;
        int v7 = outer * 17;
        int v8 = outer * 19;
        
        double d1 = outer * 1.1;
        double d2 = outer * 1.3;
        double d3 = outer * 1.7;
        double d4 = outer * 1.9;
        double d5 = outer * 2.1;
        double d6 = outer * 2.3;
        
        /* COMPUTATION THAT SHOULD BE REMATERIALIZED */
        /* This is cheap to recompute and used in multiple places */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        int val1 = array1[key_index];      /* First use */
        use_int(val1);
        
        double dval1 = darray1[key_index]; /* Second use, different mode */
        use_double(dval1);
        
        /* Force materialization */
        volatile int sink1 = key_index;
        
        /* Complex control flow splitting uses of key_index */
        if (outer % 3 == 0) {
            /* Use key_index again after branch */
            int val2 = array2[key_index];  /* Third use */
            accumulator ^= val2;
            
            /* More register pressure in this branch */
            int t1 = v1 + v2;
            int t2 = v3 + v4;
            int t3 = v5 + v6;
            use_int(t1 + t2 + t3);
            
            /* Use double variables too */
            double dt1 = d1 + d2;
            double dt2 = d3 + d4;
            d_accumulator += dt1 * dt2;
        } else if (outer % 3 == 1) {
            /* Different use of key_index */
            double dval2 = darray2[key_index]; /* Fourth use */
            d_accumulator += dval2;
            
            /* More computations to increase pressure */
            for (int inner = 0; inner < 3; inner++) {
                /* Small inner loop creates cyclic data flow */
                v1 = v2 + inner;
                v2 = v3 + inner;
                v3 = v1 + inner;  /* Creates dependency cycle */
                use_int(v1 + v2 + v3);
            }
        } else {
            /* Third path with pointer arithmetic using key_index */
            void *ptr1 = &array1[key_index];  /* Fifth use */
            void *ptr2 = &array2[key_index];  /* Sixth use */
            use_ptr(ptr1);
            use_ptr(ptr2);
            
            /* Mixed mode computations */
            int ival = key_index * 2;      /* SI mode */
            double dval = key_index * 2.5; /* DF mode */
            use_int(ival);
            use_double(dval);
        }
        
        /* Use key_index one more time before loop ends */
        if (key_index % 2 == 0) {          /* Seventh use */
            accumulator += array1[key_index];
        }
        
        /* More volatile operations to prevent optimization */
        volatile int sink2 = v7 + v8;
        volatile double sink3 = d5 * d6;
        
        /* Use all variables to keep them live */
        use_int(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8);
        use_double(d1 + d2 + d3 + d4 + d5 + d6);
    }
    
    printf("Result: %d (accumulator), %f (d_accumulator)\n", 
           accumulator, d_accumulator);
    return 0;
}
