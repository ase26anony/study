/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Global volatile variables to extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
struct DataPacket {
    int id;
    double values[8];
    float coords[4];
    long timestamp;
    char metadata[32];
    short flags[16];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataPacket *data, int size) {
    int i, j, k, l;
    double temp1, temp2, temp3, temp4;
    float f1, f2, f3, f4;
    long accumulator = 0;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i].values[0];
        temp2 = data[i].values[1];
        
        for (j = 0; j < 8; j++) {
            f1 = data[i].coords[0];
            f2 = data[i].coords[1];
            
            for (k = 0; k < 4; k++) {
                temp3 = temp1 * temp2 + f1 - f2;
                
                for (l = 0; l < 2; l++) {
                    temp4 = sin(temp3) * cos(f1) + tan(f2);
                    accumulator += (long)(temp4 * 1000);
                    
                    /* Force register pressure with many temporaries */
                    double t1 = temp1 * 1.1;
                    double t2 = temp2 * 1.2;
                    double t3 = temp3 * 1.3;
                    double t4 = temp4 * 1.4;
                    float ft1 = f1 * 1.5f;
                    float ft2 = f2 * 1.6f;
                    
                    /* Complex expression with many intermediates */
                    data[i].values[j % 8] = t1 + t2 - t3 * t4 / (ft1 + ft2);
                }
            }
            
            /* Early continue to create complex CFG */
            if (j % 3 == 0) continue;
            
            /* Another level of computation */
            for (k = 0; k < data[i].flags[j % 16]; k++) {
                accumulator += k * (j + i);
            }
        }
        
        /* Conditional break */
        if (accumulator > 1000000) break;
    }
    
    global_counter += accumulator;
}

/* Function 2: Complex switch statement with fall-through */
int test_complex_switch(int value, struct DataPacket *data) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[value].id * 2;
            /* Fall through */
        case 1:
            result += data[value].values[0];
            break;
        case 2:
            result = (int)(data[value].coords[0] * 100);
            /* Fall through */
        case 3:
        case 4:
            result += data[value].flags[0] * 3;
            break;
        case 5:
            result = data[value].timestamp % 1000;
            /* Fall through */
        case 6:
            result += strlen(data[value].metadata);
            break;
        case 7:
        case 8:
        case 9:
            result = (int)(sin(data[value].values[1]) * 1000);
            break;
        case 10:
            result = data[value].id + data[value].flags[1];
            /* Fall through */
        case 11:
            result *= 2;
            break;
        case 12:
            result = (int)(cos(data[value].coords[1]) * 1000);
            /* Fall through */
        case 13:
            result += 42;
            break;
        case 14:
            result = -1;
            break;
        default:
            result = 0;
    }
    
    /* Multiple returns create complex CFG */
    if (result < 0) return -1;
    if (result > 1000) return 1000;
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(struct DataPacket *data, int index) {
    int a, b, c, d;
    double x, y, z;
    float f;
    long l;
    
    /* Force use of specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (a)
        : "r" (data[index].id), "r" (global_counter)
        : "%eax", "memory"
    );
    
    asm volatile (
        "movq %1, %%rax\n\t"
        "imulq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (l)
        : "r" (data[index].timestamp), "r" (1000L)
        : "%rax", "memory"
    );
    
    /* Multiple asm statements competing for registers */
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=x" (x)
        : "x" (data[index].values[0]), "x" (data[index].values[1])
        : "%xmm0", "%xmm1", "memory"
    );
    
    asm volatile (
        "movss %1, %%xmm2\n\t"
        "movss %2, %%xmm3\n\t"
        "mulss %%xmm3, %%xmm2\n\t"
        "movss %%xmm2, %0\n\t"
        : "=x" (f)
        : "x" (data[index].coords[0]), "x" (data[index].coords[1])
        : "%xmm2", "%xmm3", "memory"
    );
    
    /* Complex expression using all the results */
    y = x * a + l / 1000.0;
    z = sin(y) * cos(f);
    
    data[index].values[2] = z;
    data[index].coords[2] = f;
}

/* Function 4: Irreducible control flow with computed goto */
void test_irreducible_cfg(struct DataPacket *data, int size) {
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4,
                      &&label5, &&label6, &&label7, &&label8, &&label9 };
    
    int i = 0;
    double sum = 0.0;
    
    /* Create irreducible control flow */
    if (size > 0) goto *labels[data[0].id % 10];
    
label0:
    sum += data[i].values[0];
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 1) % 10];
    
label1:
    sum -= data[i].values[1];
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 2) % 10];
    
label2:
    sum *= 1.01;
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 3) % 10];
    
label3:
    sum /= 1.02;
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 4) % 10];
    
label4:
    sum = fabs(sum);
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 5) % 10];
    
label5:
    sum += sin(data[i].values[2]);
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 6) % 10];
    
label6:
    sum += cos(data[i].values[3]);
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 7) % 10];
    
label7:
    sum += tan(data[i].values[4]);
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 8) % 10];
    
label8:
    sum = sqrt(fabs(sum));
    i++;
    if (i >= size) goto end;
    goto *labels[(i + 9) % 10];
    
label9:
    sum = log(fabs(sum) + 1.0);
    i++;
    if (i >= size) goto end;
    goto *labels[i % 10];
    
end:
    global_accumulator += sum;
}

/* Function 5: Many function calls within loops */
void test_many_calls(struct DataPacket *data, int size) {
    int i, j;
    
    for (i = 0; i < size; i += 10) {
        /* Multiple function calls with many arguments */
        int r1 = test_complex_switch(data[i].id, data);
        int r2 = test_complex_switch(data[i+1].id, &data[i+1]);
        int r3 = test_complex_switch(data[i+2].id, &data[i+2]);
        int r4 = test_complex_switch(data[i+3].id, &data[i+3]);
        
        /* Use results in complex expression */
        double avg = (r1 + r2 + r3 + r4) / 4.0;
        
        for (j = 0; j < 8; j++) {
            /* More calls inside inner loop */
            test_inline_asm(data, i + j);
            
            /* Complex computation */
            data[i].values[j] = avg * sin(data[i].values[j]) 
                              + cos(global_accumulator)
                              * tan(data[i].coords[j % 4]);
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Function 6: Mixed data types and address calculations */
void test_mixed_types(struct DataPacket *data, int size) {
    char *ptr;
    short *sptr;
    int *iptr;
    float *fptr;
    double *dptr;
    long *lptr;
    
    /* Different pointer types accessing same data */
    for (int i = 0; i < size; i++) {
        ptr = (char *)&data[i];
        sptr = (short *)&data[i].flags;
        iptr = &data[i].id;
        fptr = data[i].coords;
        dptr = data[i].values;
        lptr = &data[i].timestamp;
        
        /* Complex address calculations */
        for (int j = 0; j < 32; j++) {
            ptr[j] = (ptr[j] + i + j) % 256;
        }
        
        for (int j = 0; j < 16; j++) {
            sptr[j] = (sptr[j] * 3 + j) % 32767;
        }
        
        /* Mixed type computations */
        *iptr += (int)(*fptr * *dptr);
        *lptr += (long)(*dptr * 1000000);
        
        /* Vector-like operations */
        fptr[0] = fptr[0] * 0.9f + fptr[1] * 0.1f;
        fptr[1] = fptr[1] * 0.8f + fptr[2] * 0.2f;
        fptr[2] = fptr[2] * 0.7f + fptr[3] * 0.3f;
        
        dptr[0] = dptr[0] * 0.6 + dptr[1] * 0.4;
        dptr[1] = dptr[1] * 0.5 + dptr[2] * 0.5;
        dptr[2] = dptr[2] * 0.4 + dptr[3] * 0.6;
    }
}

/* Function 7: Function with many arguments */
double test_many_args(int a, int b, int c, int d, int e,
                      float f, float g, float h, float i,
                      double j, double k, double l, double m,
                      long n, long o, char p, short q) {
    /* Use all arguments in complex expression */
    double result = (a + b + c + d + e) * 1.0
                  + (f + g + h + i) * 2.0
                  + (j + k + l + m) * 0.5
                  + (n + o) * 0.01
                  + p * 0.001
                  + q * 0.0001;
    
    /* Complex computation to increase register pressure */
    result = sin(result) * cos(result) + tan(result * 0.1);
    result = exp(log(fabs(result) + 1.0)) * 0.5;
    
    return result;
}

/* Main function that orchestrates all tests */
int main() {
    struct DataPacket *data;
    int i, j;
    long checksum = 0;
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Allocate and initialize data */
    data = (struct DataPacket *)malloc(ARRAY_SIZE * sizeof(struct DataPacket));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i].id = i;
        data[i].timestamp = rand();
        
        for (j = 0; j < 8; j++) {
            data[i].values[j] = (double)rand() / RAND_MAX * 100.0;
        }
        
        for (j = 0; j < 4; j++) {
            data[i].coords[j] = (float)rand() / RAND_MAX * 10.0;
        }
        
        for (j = 0; j < 16; j++) {
            data[i].flags[j] = rand() % 100;
        }
        
        snprintf(data[i].metadata, 32, "Packet_%d_%ld", i, data[i].timestamp);
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (i = 0; i < ITERATIONS / 10; i++) {
        test_nested_loops(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test sequence */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch */
        for (j = 0; j < ARRAY_SIZE; j += 100) {
            checksum += test_complex_switch(data[j].id, data);
        }
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        for (j = 0; j < ARRAY_SIZE; j += 50) {
            test_inline_asm(data, j);
        }
        asm volatile("" ::: "memory");
        
        /* Test 4: Irreducible CFG */
        test_irreducible_cfg(data, ARRAY_SIZE / 2);
        asm volatile("" ::: "memory");
        
        /* Test 5: Many function calls */
        test_many_calls(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 6: Mixed types */
        test_mixed_types(data, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
        
        /* Test 7: Many arguments */
        for (j = 0; j < ARRAY_SIZE; j += 200) {
            double res = test_many_args(
                data[j].id, data[j].flags[0], data[j].flags[1],
                data[j].flags[2], data[j].flags[3],
                data[j].coords[0], data[j].coords[1],
                data[j].coords[2], data[j].coords[3],
                data[j].values[0], data[j].values[1],
                data[j].values[2], data[j].values[3],
                data[j].timestamp, data[j].timestamp / 2,
                (char)data[j].id, (short)data[j].flags[4]
            );
            global_accumulator += res;
        }
        asm volatile("" ::: "memory");
        
        /* Progress indicator */
        if (i % 10 == 0) {
            printf("Iteration %d/%d, Checksum: %ld\n", 
                   i, ITERATIONS, checksum);
        }
    }
    
    /* Final computation and output */
    checksum += (long)global_counter + (long)global_accumulator;
    
    printf("\nFinal checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Verify with a simple computation */
    long verify = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        verify += data[i].id + (long)data[i].values[0];
    }
    printf("Verification sum: %ld\n", verify);
    
    free(data);
    return 0;
}
