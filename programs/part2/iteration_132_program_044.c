/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define NUM_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex structure to force register pressure */
struct DataPacket {
    int values[8];
    double coords[4];
    char metadata[16];
    short indices[12];
    float weights[6];
    long long timestamps[3];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataPacket *data, int size) {
    int i, j, k, l, m;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    long long counter = 0;
    
    /* Outer loops create many live ranges */
    for (i = 0; i < size / 4; i++) {
        int base = data[i].values[0];
        double scale = data[i].coords[0];
        
        for (j = 0; j < 8; j++) {
            int val1 = data[i].values[j];
            float weight = data[i].weights[j % 6];
            
            for (k = 0; k < 4; k++) {
                double coord = data[i].coords[k];
                short idx = data[i].indices[k * 3];
                
                for (l = 0; l < 3; l++) {
                    long long ts = data[i].timestamps[l];
                    char meta = data[i].metadata[l];
                    
                    /* Complex expression with many intermediates */
                    double temp1 = val1 * scale + coord;
                    float temp2 = weight * acc1 + acc2;
                    long long temp3 = ts + counter;
                    int temp4 = base + idx + meta;
                    
                    /* Multiple nested if-else chains */
                    if (temp1 > 100.0) {
                        sum1 += temp1 * 0.5;
                        if (temp2 < 50.0f) {
                            acc1 = temp2 * 1.1f;
                            if (temp3 > 1000) {
                                counter = temp3 / 2;
                            } else {
                                counter = temp3 * 2;
                            }
                        } else if (temp2 > 100.0f) {
                            acc2 = temp2 * 0.9f;
                            for (m = 0; m < 2; m++) {
                                sum2 += temp4 * (m + 1);
                            }
                        }
                    } else if (temp1 < -50.0) {
                        sum3 -= temp1 * 0.3;
                        acc3 = temp2 * 0.7f;
                    } else {
                        /* Early return in some cases */
                        if (temp4 == 0) return;
                    }
                    
                    /* Inline assembly with register constraints */
                    asm volatile (
                        "mov %[val], %%eax\n\t"
                        "imul %%eax, %%eax\n\t"
                        "mov %%eax, %[res]"
                        : [res] "=r" (temp4)
                        : [val] "r" (val1)
                        : "%eax", "cc"
                    );
                    
                    data[i].values[j] = temp4;
                }
            }
        }
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    global_barrier = sum1 + sum2 + sum3 + acc1 + acc2 + acc3 + counter;
}

/* Function 2: Switch statement with many cases */
int test_large_switch(int value, struct DataPacket *data) {
    int result = 0;
    
    switch (value % NUM_CASES) {
        case 0: {
            double a = data[0].coords[0];
            double b = data[0].coords[1];
            result = (int)(a * b * 100);
            /* Fall through */
        }
        case 1: {
            float x = data[1].weights[0];
            float y = data[1].weights[1];
            result += (int)(x + y * 50);
            break;
        }
        case 2: {
            int v1 = data[2].values[0];
            int v2 = data[2].values[1];
            int v3 = data[2].values[2];
            result = v1 * v2 - v3;
            break;
        }
        case 3: {
            long long t1 = data[3].timestamps[0];
            long long t2 = data[3].timestamps[1];
            result = (int)(t2 - t1);
            /* Fall through */
        }
        case 4: {
            short idx = data[4].indices[0];
            char meta = data[4].metadata[0];
            result += idx * meta;
            break;
        }
        case 5: {
            double sum = 0.0;
            for (int i = 0; i < 4; i++) {
                sum += data[5].coords[i];
            }
            result = (int)(sum * 10);
            break;
        }
        case 6: {
            float prod = 1.0f;
            for (int i = 0; i < 6; i++) {
                prod *= data[6].weights[i];
            }
            result = (int)(prod * 1000);
            break;
        }
        case 7: {
            int total = 0;
            for (int i = 0; i < 8; i++) {
                total += data[7].values[i];
            }
            result = total / 8;
            break;
        }
        case 8: {
            /* Complex expression with many temporaries */
            double d1 = data[8].coords[0];
            double d2 = data[8].coords[1];
            double d3 = data[8].coords[2];
            float f1 = data[8].weights[0];
            float f2 = data[8].weights[1];
            int i1 = data[8].values[0];
            int i2 = data[8].values[1];
            result = (int)((d1 * d2 / d3) + (f1 - f2) * (i1 + i2));
            break;
        }
        case 9: {
            /* Multiple inline asm statements */
            int a, b, c;
            asm volatile (
                "mov $100, %%eax\n\t"
                "mov $200, %%ebx\n\t"
                "add %%eax, %%ebx\n\t"
                "mov %%ebx, %0"
                : "=r" (a)
                :: "%eax", "%ebx", "cc"
            );
            
            asm volatile (
                "mov $300, %%ecx\n\t"
                "sub $50, %%ecx\n\t"
                "mov %%ecx, %0"
                : "=r" (b)
                :: "%ecx", "cc"
            );
            
            asm volatile (
                "mov %1, %%edx\n\t"
                "mov %2, %%esi\n\t"
                "imul %%edx, %%esi\n\t"
                "mov %%esi, %0"
                : "=r" (c)
                : "r" (a), "r" (b)
                : "%edx", "%esi", "cc"
            );
            
            result = c;
            break;
        }
        case 10: {
            /* Vector-like operations */
            int sum[4] = {0};
            for (int i = 0; i < 4; i++) {
                sum[0] += data[10].values[i];
                sum[1] += data[10].values[i + 4];
                sum[2] += (int)data[10].weights[i];
                sum[3] += data[10].indices[i * 3];
            }
            result = sum[0] + sum[1] + sum[2] + sum[3];
            break;
        }
        case 11: {
            /* Mixed precision calculations */
            double dsum = 0.0;
            float fsum = 0.0f;
            long long lsum = 0;
            
            for (int i = 0; i < 3; i++) {
                dsum += data[11].coords[i];
                fsum += data[11].weights[i];
                lsum += data[11].timestamps[i];
            }
            
            result = (int)(dsum + fsum + (lsum % 1000));
            break;
        }
        case 12: {
            /* Pointer aliasing to extend live ranges */
            int *p1 = &data[12].values[0];
            int *p2 = &data[12].values[4];
            int *p3 = &data[12].values[2];
            
            *p1 = *p2 + *p3;
            *p2 = *p1 - *p3;
            *p3 = *p1 * *p2;
            
            result = *p1 + *p2 + *p3;
            break;
        }
        case 13: {
            /* Nested switch */
            switch (data[13].metadata[0] % 3) {
                case 0:
                    result = data[13].values[0] * 2;
                    break;
                case 1:
                    result = data[13].values[1] * 3;
                    break;
                case 2:
                    result = data[13].values[2] * 4;
                    break;
            }
            break;
        }
        case 14: {
            /* Computed goto (label address) */
            void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
            int idx = data[14].values[0] % 4;
            
            goto *labels[idx];
            
        label0:
            result = 100;
            goto end;
        label1:
            result = 200;
            goto end;
        label2:
            result = 300;
            goto end;
        label3:
            result = 400;
            goto end;
        end:
            break;
        }
        default:
            result = -1;
    }
    
    return result;
}

/* Function 3: Many function arguments to stress register/stack passing */
long long test_many_args(int a, int b, int c, int d, int e, int f,
                         int g, int h, int i, int j, int k, int l,
                         double m, double n, double o, double p,
                         float q, float r, float s, float t) {
    /* Complex expression using all arguments */
    long long result = 
        (long long)a * b * c * d +
        (long long)e * f * g * h +
        (long long)i * j * k * l +
        (long long)(m * n * 1000) +
        (long long)(o * p * 1000) +
        (long long)(q * r * 1000) +
        (long long)(s * t * 1000);
    
    /* Inline asm with memory clobber */
    asm volatile (
        "mov %0, %%rax\n\t"
        "add $1000, %%rax\n\t"
        "mov %%rax, %0"
        : "+r" (result)
        :
        : "%rax", "cc", "memory"
    );
    
    return result;
}

/* Function 4: Irreducible control flow with goto */
void test_irreducible_cfg(struct DataPacket *data, int size) {
    int i = 0;
    int state = 0;
    
start:
    if (i >= size) goto finish;
    
    switch (state) {
        case 0:
            data[i].values[0] *= 2;
            state = 1;
            goto middle;
        case 1:
            data[i].values[1] += 3;
            state = 2;
            goto middle;
        case 2:
            data[i].values[2] -= 5;
            state = 3;
            goto middle;
        case 3:
            data[i].values[3] /= 2;
            state = 0;
            i++;
            goto start;
    }
    
middle:
    data[i].coords[0] += 1.0;
    data[i].coords[1] -= 1.0;
    goto start;
    
finish:
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Function 5: SIMD-like operations using multiple registers */
void test_vector_ops(struct DataPacket *data, int size) {
    /* Declare many variables at function scope */
    double d0, d1, d2, d3;
    float f0, f1, f2, f3, f4, f5;
    int i0, i1, i2, i3, i4, i5, i6, i7;
    long long l0, l1, l2;
    short s0, s1, s2, s3, s4, s5;
    char c0, c1, c2, c3;
    
    for (int idx = 0; idx < size; idx++) {
        /* Load multiple values into registers */
        d0 = data[idx].coords[0];
        d1 = data[idx].coords[1];
        d2 = data[idx].coords[2];
        d3 = data[idx].coords[3];
        
        f0 = data[idx].weights[0];
        f1 = data[idx].weights[1];
        f2 = data[idx].weights[2];
        f3 = data[idx].weights[3];
        f4 = data[idx].weights[4];
        f5 = data[idx].weights[5];
        
        i0 = data[idx].values[0];
        i1 = data[idx].values[1];
        i2 = data[idx].values[2];
        i3 = data[idx].values[3];
        i4 = data[idx].values[4];
        i5 = data[idx].values[5];
        i6 = data[idx].values[6];
        i7 = data[idx].values[7];
        
        l0 = data[idx].timestamps[0];
        l1 = data[idx].timestamps[1];
        l2 = data[idx].timestamps[2];
        
        s0 = data[idx].indices[0];
        s1 = data[idx].indices[1];
        s2 = data[idx].indices[2];
        s3 = data[idx].indices[3];
        s4 = data[idx].indices[4];
        s5 = data[idx].indices[5];
        
        c0 = data[idx].metadata[0];
        c1 = data[idx].metadata[1];
        c2 = data[idx].metadata[2];
        c3 = data[idx].metadata[3];
        
        /* Complex parallel computations */
        d0 = d0 * 1.1 + d1 * 0.9 - d2 * 0.5 + d3 * 1.5;
        d1 = d1 * 0.8 + d2 * 1.2 - d3 * 0.7 + d0 * 0.6;
        
        f0 = f0 * 2.0f + f1 * 1.5f - f2 * 0.5f;
        f1 = f1 * 3.0f + f2 * 0.8f - f3 * 1.2f;
        f2 = f2 * 1.1f + f3 * 2.0f - f4 * 0.9f;
        f3 = f3 * 0.7f + f4 * 1.3f - f5 * 0.6f;
        
        i0 = i0 + i1 - i2 * i3 / (i4 + 1);
        i1 = i1 * i2 + i3 - i4 * i5 / (i6 + 1);
        i2 = i2 + i3 * i4 - i5 * i6 / (i7 + 1);
        
        l0 = l0 + l1 - l2;
        l1 = l1 * 2 - l0;
        
        s0 = s0 + s1 - s2 + s3 - s4 + s5;
        c0 = c0 * c1 + c2 - c3;
        
        /* Store back with transformations */
        data[idx].coords[0] = d0;
        data[idx].coords[1] = d1;
        data[idx].weights[0] = f0;
        data[idx].weights[1] = f1;
        data[idx].weights[2] = f2;
        data[idx].weights[3] = f3;
        data[idx].values[0] = i0;
        data[idx].values[1] = i1;
        data[idx].values[2] = i2;
        data[idx].timestamps[0] = l0;
        data[idx].timestamps[1] = l1;
        data[idx].indices[0] = s0;
        data[idx].metadata[0] = c0;
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "mov %[in2], %%ebx\n\t"
            "add %%ebx, %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "mov %[in3], %%ecx\n\t"
            "mov %[in4], %%edx\n\t"
            "imul %%ecx, %%edx\n\t"
            "mov %%edx, %[out2]"
            : [out1] "=r" (i3), [out2] "=r" (i4)
            : [in1] "r" (i5), [in2] "r" (i6), 
              [in3] "r" (i7), [in4] "r" (i0)
            : "%eax", "%ebx", "%ecx", "%edx", "cc"
        );
    }
}

/* Main function that orchestrates everything */
int main() {
    struct DataPacket *data;
    long long checksum = 0;
    int i, j;
    
    /* Initialize random seed */
    srand(global_seed);
    
    /* Allocate and initialize data */
    data = (struct DataPacket*)malloc(ARRAY_SIZE * sizeof(struct DataPacket));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = 0; j < 8; j++) {
            data[i].values[j] = rand() % 1000;
        }
        for (j = 0; j < 4; j++) {
            data[i].coords[j] = (rand() % 10000) / 100.0;
        }
        for (j = 0; j < 16; j++) {
            data[i].metadata[j] = 'A' + (rand() % 26);
        }
        for (j = 0; j < 12; j++) {
            data[i].indices[j] = rand() % 100;
        }
        for (j = 0; j < 6; j++) {
            data[i].weights[j] = (rand() % 1000) / 10.0f;
        }
        for (j = 0; j < 3; j++) {
            data[i].timestamps[j] = rand() % 1000000;
        }
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (i = 0; i < ITERATIONS / 10; i++) {
        test_nested_loops(data, ARRAY_SIZE / 100);
    }
    
    /* Main test iterations */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE / 10);
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Large switch */
        int switch_result = test_large_switch(i, data);
        checksum += switch_result;
        
        /* Test 3: Many arguments */
        long long args_result = test_many_args(
            data[i % ARRAY_SIZE].values[0],
            data[i % ARRAY_SIZE].values[1],
            data[i % ARRAY_SIZE].values[2],
            data[i % ARRAY_SIZE].values[3],
            data[i % ARRAY_SIZE].values[4],
            data[i % ARRAY_SIZE].values[5],
            data[i % ARRAY_SIZE].values[6],
            data[i % ARRAY_SIZE].values[7],
            data[i % ARRAY_SIZE].indices[0],
            data[i % ARRAY_SIZE].indices[1],
            data[i % ARRAY_SIZE].indices[2],
            data[i % ARRAY_SIZE].indices[3],
            data[i % ARRAY_SIZE].coords[0],
            data[i % ARRAY_SIZE].coords[1],
            data[i % ARRAY_SIZE].coords[2],
            data[i % ARRAY_SIZE].coords[3],
            data[i % ARRAY_SIZE].weights[0],
            data[i % ARRAY_SIZE].weights[1],
            data[i % ARRAY_SIZE].weights[2],
            data[i % ARRAY_SIZE].weights[3]
        );
        checksum += args_result;
        
        /* Test 4: Irreducible CFG */
        test_irreducible_cfg(data, ARRAY_SIZE / 100);
        
        /* Test 5: Vector operations */
        test_vector_ops(data, ARRAY_SIZE / 50);
        
        /* Update checksum with some data */
        for (j = 0; j < 8; j++) {
            checksum += data[i % ARRAY_SIZE].values[j];
        }
    }
    
    /* Final verification */
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(data);
    
    return 0;
}
