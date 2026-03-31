/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 2000
#define M 100

/* Structure for complex data environment */
struct Data {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(struct Data *d) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < N; i++) {
        #pragma omp scan inclusive(prefix_sum)
        d->values[i] += prefix_sum;
        
        #pragma omp scan exclusive(exclusive_sum)
        d->indices[i] = (int)exclusive_sum;
        
        prefix_sum += d->values[i];
        exclusive_sum += 1.0;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct Data *d) {
    #pragma omp target enter data map(to: d->values[0:N/2]) \
        map(alloc: d->indices[0:N/2])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N/2; i++) {
        d->values[i] = d->values[i] * 2.0;
        d->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: d->values[0:N/2]) \
        map(release: d->indices[0:N/2])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(struct Data *d) {
    double temp = 0.0;
    int last_val = 0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:temp) lastprivate(last_val) \
        linear(i:1) collapse(2) if(N > 1000)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            if (idx < N) {
                temp += d->values[idx];
                last_val = idx;
            }
        }
    }
    
    d->result = temp;
    
    /* Nested parallel regions with reduction */
    #pragma omp parallel reduction(+:temp)
    {
        #pragma omp for nowait
        for (int i = 0; i < N; i++) {
            temp += d->indices[i];
        }
        
        #pragma omp single
        {
            d->result += temp;
        }
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(struct Data *d) {
    #pragma omp simd reduction(+:d->result) linear(i:1)
    for (int i = 0; i < N; i++) {
        if (d->values[i] > 0.5) {
            d->result += d->values[i] * 2.0;
        } else {
            d->result += d->values[i];
        }
    }
    
    /* Additional scan with exclusive */
    double scan_temp = 0.0;
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        #pragma omp scan exclusive(scan_temp)
        d->values[i] = scan_temp;
        scan_temp += 1.0 / (i + 1);
    }
}

/* Function 5: Target regions with complex mappings */
void test_target_complex(struct Data *d) {
    #pragma omp target map(tofrom: d->values[0:N]) \
        map(to: d->indices[0:N]) if(N > 500)
    {
        #pragma omp teams distribute parallel for simd \
            reduction(+:d->result) lastprivate(d->indices[N-1])
        for (int i = 0; i < N; i++) {
            d->values[i] = d->values[i] + d->indices[i];
            d->result += d->values[i];
        }
    }
}

/* Main function orchestrating all tests */
int main() {
    struct Data *data = (struct Data*)malloc(sizeof(struct Data));
    if (!data) return 1;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data->values[i] = (double)i / N;
        data->indices[i] = i % 100;
    }
    data->result = 0.0;
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(data);
    
    #pragma omp parallel if(N > 1000)
    {
        test_enter_data(data);
    }
    
    test_internal_temporaries(data);
    test_simd_conditionals(data);
    test_target_complex(data);
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        final_result += data->values[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Data result: %f\n", data->result);
    
    free(data);
    return 0;
}
