/* Target: haifa-sched.cc - free_sched_context coverage test */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE unsigned int hash_mix(unsigned int a, unsigned int b, unsigned int c) {
    a = a - b;  a = a - c;  a = a ^ (c >> 13);
    b = b - c;  b = b - a;  b = b ^ (a << 8);
    c = c - a;  c = c - b;  c = c ^ (b >> 13);
    a = a - b;  a = a - c;  a = a ^ (c >> 12);
    b = b - c;  b = b - a;  b = b ^ (a << 16);
    c = c - a;  c = c - b;  c = c ^ (b >> 5);
    a = a - b;  a = a - c;  a = a ^ (c >> 3);
    b = b - c;  b = b - a;  b = b ^ (a << 10);
    c = c - a;  c = c - b;  c = c ^ (b >> 15);
    return c;
}

ALWAYS_INLINE int compute_index(int x, int y, int z, int size) {
    int idx = (x * 73856093) ^ (y * 19349663) ^ (z * 83492791);
    return idx & (size - 1);
}

/* Complex control flow with irreducible CFG using computed goto */
HOT NOINLINE unsigned long complex_flow_irreducible(int *data, int size, int iterations) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    unsigned long sum = 0;
    int i, j, k;
    
    for (i = 0; i < iterations; i++) {
        int state = i % 8;
        goto *labels[state];
        
        L0: {
            /* Nested loops with memory dependencies */
            for (j = 0; j < size; j++) {
                int val = data[j];
                /* Artificial scheduling barrier */
                asm volatile("" ::: "memory");
                for (k = 0; k < 8; k++) {
                    val = (val << 3) | (val >> 29);
                    val ^= 0x9e3779b9;
                    val += j * k;
                }
                data[j] = val;
                sum += val;
            }
            state = (state + 1) % 8;
            if (i % 3 == 0) goto *labels[state];
            else goto L7;
        }
        
        L1: {
            /* Switch with multiple cases */
            switch (i % 5) {
                case 0: {
                    #pragma omp simd
                    for (j = 0; j < size; j++) {
                        data[j] = data[j] * 3 + 1;
                        sum += data[j];
                    }
                    break;
                }
                case 1: {
                    for (j = size - 1; j >= 0; j--) {
                        data[j] = hash_mix(data[j], j, i);
                        sum ^= data[j];
                    }
                    break;
                }
                case 2: {
                    int temp = 0;
                    while (temp < size) {
                        data[temp] = (data[temp] << 1) | (data[temp] >> 31);
                        sum += data[temp];
                        temp += 2;
                    }
                    break;
                }
                case 3: {
                    do {
                        data[i % size] = compute_index(data[i % size], i, sum, size);
                        sum = sum * 1103515245 + 12345;
                    } while (--j > 0);
                    break;
                }
                default: {
                    /* Mixed operations with asm barriers */
                    for (j = 0; j < size; j += 4) {
                        int t0 = data[j];
                        int t1 = data[j+1];
                        int t2 = data[j+2];
                        int t3 = data[j+3];
                        
                        asm volatile("" : "+r"(t0), "+r"(t1) : : "memory");
                        t0 = t0 * t1 + t2;
                        t1 = t1 * t3 - t0;
                        t2 = t2 ^ t0 ^ t1;
                        t3 = t3 * 0xcc9e2d51;
                        
                        asm volatile("" : "+r"(t2), "+r"(t3) : : "memory");
                        data[j] = t0;
                        data[j+1] = t1;
                        data[j+2] = t2;
                        data[j+3] = t3;
                        
                        sum += t0 + t1 + t2 + t3;
                    }
                    break;
                }
            }
            state = (state * 7 + 3) % 8;
            goto *labels[state];
        }
        
        L2: {
            /* Tight inner loop with carried dependency */
            int acc = 0;
            for (j = 0; j < size; j++) {
                acc = acc * 31 + data[j];
                data[j] = acc;
                /* Create anti-dependency */
                asm volatile("" : : "r"(acc) : "memory");
            }
            sum += acc;
            goto L4;
        }
        
        L3: {
            /* Pointer arithmetic and memory ops */
            int *ptr = data;
            int *end = data + size;
            while (ptr < end) {
                *ptr = (*ptr ^ 0x5a827999) + (int)(ptr - data);
                sum += *ptr;
                ptr += (i % 4) + 1;
            }
            goto L5;
        }
        
        L4: {
            /* Another nested loop structure */
            for (j = 0; j < 16; j++) {
                int idx = compute_index(i, j, sum, size);
                for (k = 0; k < 4; k++) {
                    data[idx] = hash_mix(data[idx], k, j);
                    idx = (idx + 1) & (size - 1);
                }
                sum += data[idx];
            }
            goto L6;
        }
        
        L5: {
            /* Conditional chain */
            if (i % 2 == 0) {
                for (j = 0; j < size; j += 2) {
                    data[j] = data[j] * 2;
                    sum += data[j];
                }
            } else if (i % 3 == 0) {
                for (j = 1; j < size; j += 2) {
                    data[j] = data[j] / 3;
                    sum += data[j];
                }
            } else if (i % 5 == 0) {
                for (j = 0; j < size; j++) {
                    data[j] = data[j] ^ sum;
                    sum = sum ^ data[j];
                }
            } else {
                for (j = 0; j < size; j++) {
                    data[j] = ~data[j];
                    sum -= data[j];
                }
            }
            goto L0;
        }
        
        L6: {
            /* OpenMP parallel region */
            #pragma omp parallel for reduction(+:sum)
            for (j = 0; j < size; j++) {
                int val = data[j];
                val = (val << 1) | (val >> 31);
                val = val * 0x9e3779b9;
                val = val ^ (val >> 16);
                data[j] = val;
                sum += val;
            }
            goto L1;
        }
        
        L7: {
            /* Mixed operations with function calls */
            for (j = 0; j < size; j++) {
                int idx = compute_index(data[j], j, i, size);
                data[j] = hash_mix(data[j], idx, sum);
                sum = sum * 1664525 + 1013904223;
                
                /* Inline asm with clobber */
                asm volatile (
                    "rorl $7, %0\n\t"
                    "addl $1, %0\n\t"
                    : "+r" (data[j])
                    :
                    : "cc"
                );
            }
            /* Don't jump - fall through to next iteration */
        }
    }
    
    return sum;
}

/* Another complex function with different patterns */
HOT NOINLINE unsigned long nested_switch_loops(int *data, int size) {
    unsigned long result = 0;
    int i, j, k;
    
    for (i = 0; i < size; i++) {
        switch (data[i] % 7) {
            case 0:
                for (j = i; j < size; j++) {
                    int val = data[j];
                    for (k = 0; k < 3; k++) {
                        val = (val * 1103515245 + 12345) & 0x7fffffff;
                        asm volatile("" : "+r"(val) : : "memory");
                    }
                    data[j] = val;
                    result += val;
                }
                break;
                
            case 1:
                j = 0;
                while (j < size) {
                    data[j] = data[j] ^ result;
                    result = result ^ data[j];
                    j += (i % 3) + 1;
                }
                break;
                
            case 2:
                do {
                    data[i] = (data[i] << 4) | (data[i] >> 28);
                    data[i] = data[i] * 0xcc9e2d51;
                    result += data[i];
                } while (--i > 0);
                break;
                
            case 3:
                #pragma omp simd
                for (j = 0; j < size; j += 4) {
                    data[j] = data[j] + data[j+1];
                    data[j+1] = data[j+1] - data[j+2];
                    data[j+2] = data[j+2] ^ data[j+3];
                    data[j+3] = data[j+3] * data[j];
                    result += data[j] + data[j+1] + data[j+2] + data[j+3];
                }
                break;
                
            case 4:
                for (j = 0; j < 8; j++) {
                    int idx = (i * j) & (size - 1);
                    data[idx] = hash_mix(data[idx], i, j);
                    result = result * 31 + data[idx];
                }
                break;
                
            case 5:
                /* Deep if-else chain */
                if (data[i] < 0) {
                    data[i] = -data[i];
                    result -= data[i];
                } else if (data[i] < 100) {
                    data[i] = data[i] * 2;
                    result += data[i];
                } else if (data[i] < 1000) {
                    data[i] = data[i] / 2;
                    result ^= data[i];
                } else if (data[i] < 10000) {
                    data[i] = data[i] & 0x5555;
                    result |= data[i];
                } else {
                    data[i] = ~data[i];
                    result &= data[i];
                }
                break;
                
            default:
                /* Mixed operations */
                int temp = data[i];
                temp = temp * 0x5bd1e995;
                temp = temp ^ (temp >> 15);
                temp = temp + 0x7fffffff;
                asm volatile (
                    "bsfl %1, %0\n\t"
                    : "=r" (temp)
                    : "r" (temp)
                    : "cc"
                );
                data[i] = temp;
                result += temp;
                break;
        }
    }
    
    return result;
}

/* Function with loop-carried dependencies */
HOT NOINLINE unsigned long loop_carried_dep(int *data, int size, int iters) {
    unsigned long sum = 0;
    int i, j;
    
    for (i = 0; i < iters; i++) {
        int acc = data[0];
        for (j = 1; j < size; j++) {
            /* Create true dependency chain */
            acc = acc * 3 + data[j];
            data[j] = acc;
            
            /* Memory barrier to force scheduling constraints */
            asm volatile("" : : "r"(acc) : "memory");
            
            /* Independent operations that can be scheduled in parallel */
            int idx = compute_index(i, j, acc, size);
            int temp = data[idx];
            temp = (temp << 5) | (temp >> 27);
            temp ^= 0x9e3779b9;
            data[idx] = temp;
            
            sum += acc + temp;
        }
        data[0] = acc;
    }
    
    return sum;
}

/* Main driver with warm-up and verification */
int main() {
    const int sizes[] = {256, 512, 1024, 2048, 4096};
    const int iters[] = {100, 200, 300, 400, 500};
    int num_tests = sizeof(sizes) / sizeof(sizes[0]);
    
    unsigned long total_checksum = 0;
    clock_t total_time = 0;
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int test = 0; test < 2; test++) {
        int size = sizes[test % num_tests];
        int *data = (int*)malloc(size * sizeof(int));
        
        /* Initialize with pseudo-random data */
        srand(42 + test);
        for (int i = 0; i < size; i++) {
            data[i] = rand();
        }
        
        unsigned long result = complex_flow_irreducible(data, size, 10);
        total_checksum ^= result;
        
        free(data);
    }
    
    /* Main test phase with different patterns */
    printf("Main test phase...\n");
    for (int test = 0; test < num_tests; test++) {
        int size = sizes[test];
        int iteration = iters[test % (sizeof(iters)/sizeof(iters[0]))];
        
        int *data1 = (int*)malloc(size * sizeof(int));
        int *data2 = (int*)malloc(size * sizeof(int));
        int *data3 = (int*)malloc(size * sizeof(int));
        
        /* Initialize arrays */
        srand(12345 + test);
        for (int i = 0; i < size; i++) {
            int val = rand();
            data1[i] = val;
            data2[i] = val ^ 0xaaaaaaaa;
            data3[i] = val * 0x9e3779b9;
        }
        
        clock_t start = clock();
        
        /* Execute different scheduling patterns */
        unsigned long r1 = complex_flow_irreducible(data1, size, iteration);
        unsigned long r2 = nested_switch_loops(data2, size);
        unsigned long r3 = loop_carried_dep(data3, size, iteration / 10);
        
        clock_t end = clock();
        total_time += (end - start);
        
        /* Combine results */
        unsigned long test_checksum = r1 ^ r2 ^ r3;
        total_checksum = total_checksum * 31 + test_checksum;
        
        printf("Test %d (size=%d): checksum = 0x%016lx\n", 
               test, size, test_checksum);
        
        free(data1);
        free(data2);
        free(data3);
    }
    
    printf("\nFinal checksum: 0x%016lx\n", total_checksum);
    printf("Total CPU time: %.3f seconds\n", (double)total_time / CLOCKS_PER_SEC);
    
    return 0;
}
