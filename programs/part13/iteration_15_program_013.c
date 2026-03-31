/* test_ddg.c - Complex loop to trigger DDG edge creation */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i) {
    static int offsets[8] = {0, 1, -1, 2, -2, 3, -3, 4};
    return offsets[i & 7];
}

void process_data(double* restrict result, const double* data, 
                  double* shared_buffer, int size, int seed) {
    double acc = seed * 0.01;
    double temp_reg = 0.0;
    
    /* Force loop to be non-trivial for scheduler */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = data[i] * 1.5;          /* Read data[i] */
        val = val / (data[i-1] + 0.001);     /* Floating-point division - high latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with recurrence */
        acc = acc + val * 0.25;              /* distance = 1 */
        
        /* 3. ANTI-DEPENDENCY (WAR) via shared buffer */
        double old_val = shared_buffer[i-1]; /* Read before write */
        shared_buffer[i] = acc + old_val;    /* Write to same location */
        
        /* 4. OUTPUT DEPENDENCY (WAW) on temp_reg */
        temp_reg = external_func(acc);       /* First write to temp_reg */
        
        /* 5. CONTROL DEPENDENCY with conditional */
        if (i % 16 == 0) {
            /* Overwrite temp_reg - creates output dependency */
            temp_reg = 1.0 / (double)(i+1);  /* Another division */
            
            /* Anti-dependency within conditional */
            double tmp = shared_buffer[i];   /* Read */
            shared_buffer[i] = tmp * 0.5;    /* Write to same location */
        }
        
        /* 6. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + get_offset(i);
        if (idx >= 0 && idx < size) {
            /* May alias with shared_buffer due to complex addressing */
            result[idx] = temp_reg + shared_buffer[i];
        }
        
        /* 7. ANOTHER OUTPUT DEPENDENCY chain */
        temp_reg = acc * 0.3;                /* Overwrite temp_reg again */
        
        /* 8. MEMORY DEPENDENCY with pointer arithmetic */
        double* ptr = shared_buffer + i;
        *ptr = *ptr + temp_reg;              /* Read-modify-write */
    }
    
    /* Volatile sink to prevent elimination */
    volatile double sink = acc + result[size/2];
}

/* Secondary loop with different dependency patterns */
void process_data2(float* a, float* b, float* c, int n) {
    float sum = 0.0f;
    
    for (int i = 2; i < n; ++i) {
        /* Loop-carried output dependency */
        sum = a[i] * b[i];                   /* WAW on sum */
        
        /* True dependency chain with integer division */
        int divisor = (int)b[i-1] + 1;
        if (divisor != 0) {
            sum = sum / divisor;             /* Integer division - variable latency */
        }
        
        /* Anti-dependency with array */
        float temp = c[i-2];                 /* Read */
        c[i] = sum + temp;                   /* Write */
        
        /* Control flow affecting dependencies */
        if (sum > 100.0f) {
            c[i-1] = sum * 0.5f;            /* May create output dep with previous iteration */
        }
    }
    
    volatile float sink2 = sum + c[n/2];
}

int main() {
    const int SIZE = 1024;
    
    /* Source data arrays */
    double data[SIZE];
    double result[SIZE];
    double buffer[SIZE];
    
    float fa[SIZE];
    float fb[SIZE];
    float fc[SIZE];
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (i % 37) * 0.1;
        buffer[i] = (i % 19) * 0.05;
        fa[i] = (i % 23) * 0.07f;
        fb[i] = (i % 29) * 0.03f;
        fc[i] = (i % 31) * 0.11f;
    }
    
    /* Process with complex dependencies */
    process_data(result, data, buffer, SIZE, 42);
    
    /* Process second loop with different patterns */
    process_data2(fa, fb, fc, SIZE);
    
    /* Use results to prevent dead code elimination */
    volatile double final_check = result[SIZE-1] + buffer[SIZE/2] + fc[SIZE-1];
    
    return (final_check > 0.0) ? 0 : 1;
}
