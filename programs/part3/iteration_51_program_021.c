#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
void vla_helper(int size, int seed) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = (i * seed) % 256;
    }
    /* Use the VLA to prevent optimization */
    asm volatile ("" : : "r"(vla) : "memory");
}

/* Another noinline helper with complex operations */
__attribute__((noinline))
double fp_chain(double a, double b, double c, double d) {
    double t1 = a + b;
    double t2 = t1 * c;
    double t3 = t2 / d;
    double t4 = sqrt(fabs(t3));
    double t5 = sin(t4);
    return t5 * cos(t4);
}

int main() {
    int i, j, k;
    double result = 0.0;
    int checksum = 0;
    
    /* Initialize arrays with random data */
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (i = 0; i < OUTER_LOOPS; i++) {
        /* PATTERN 1: Large dependency chain basic block */
        double chain_acc = 0.0;
        int idx = i % ARRAY_SIZE;
        
        /* Start of large basic block with mixed operations */
        int a = int_array[idx];
        int b = int_array[(idx + 1) % ARRAY_SIZE];
        int c = int_array[(idx + 2) % ARRAY_SIZE];
        
        /* Integer dependency chain */
        int d = a + b;
        int e = d * c;
        int f = e % (b + 1);
        int g = f - c;
        int h = g / (a != 0 ? a : 1);
        int m = h * d;
        
        /* Floating-point dependency chain */
        double x = fp_array[idx];
        double y = fp_array[(idx + 1) % ARRAY_SIZE];
        double z = x + y;
        double w = z * y;
        double v = w / (x + 1.0);
        double u = sqrt(fabs(v));
        double t = sin(u) * cos(v);
        
        /* Mixed type operations */
        chain_acc = t * m + u * h;
        
        /* Memory store to force scheduling considerations */
        fp_array[idx] = chain_acc;
        
        /* Inline assembly barrier - creates scheduling region boundary */
        asm volatile ("" ::: "memory");
        
        /* Continue dependency chain after barrier */
        double post_acc = chain_acc * 2.0;
        for (k = 0; k < 5; k++) {
            post_acc = post_acc + fp_chain(post_acc, y, z, x);
        }
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        int inner_bound = (rand() % INNER_BASE) + 10; /* Data-dependent */
        int middle_bound = (i % 10) + 5;
        
        for (j = 0; j < middle_bound; j++) {
            int local_sum = 0;
            for (k = 0; k < inner_bound; k++) {
                /* Complex addressing modes */
                int addr = (j * 17 + k * 13) % ARRAY_SIZE;
                int load1 = int_array[addr];
                int load2 = int_array[(addr + 7) % ARRAY_SIZE];
                
                /* Mixed operations in loop */
                local_sum += load1 * load2;
                local_sum -= load1 % (load2 + 1);
                local_sum = local_sum ^ (load1 & load2);
                
                /* Floating-point in the same loop */
                double fp1 = fp_array[addr];
                double fp2 = fp_array[(addr + 3) % ARRAY_SIZE];
                fp_array[addr] = fp1 * 0.99 + fp2 * 0.01;
            }
            checksum += local_sum;
            
            /* Another assembly barrier inside nested loop */
            asm volatile ("" ::: "memory");
        }
        
        /* PATTERN 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* Rare condition */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_acc = 0.0;
            for (k = 0; k < 100; k++) {
                cold_acc += sqrt(fp_array[k % ARRAY_SIZE]);
                cold_acc *= 1.0001;
                
                /* Use alloca in cold path */
                int* dynamic = (int*)alloca(sizeof(int) * 10);
                for (int n = 0; n < 10; n++) {
                    dynamic[n] = k * n;
                    cold_acc += dynamic[n];
                }
            }
            result += cold_acc;
            
            /* Force scheduler context for cold path */
            vla_helper(50, checksum);
        } else {
            /* Hot path - simpler operations */
            result += chain_acc * 0.01;
        }
        
        /* PATTERN 4: VLA usage between patterns */
        int vla_size = (i % 20) + 10;
        vla_helper(vla_size, checksum);
        
        /* More mixed operations */
        double* ptr = &fp_array[i % ARRAY_SIZE];
        *ptr = *ptr + sin(result) * 0.1;
        
        /* Complex integer math with multiple dependencies */
        int tmp1 = checksum & 0xFF;
        int tmp2 = tmp1 * 1103515245 + 12345;
        int tmp3 = (tmp2 >> 16) & 0x7FFF;
        int tmp4 = tmp3 % 100;
        int tmp5 = tmp4 * tmp1;
        checksum ^= tmp5;
        
        /* Final assembly barrier in iteration */
        asm volatile ("" ::: "memory");
    }
    
    /* Ensure all results are used */
    printf("Result: %f, Checksum: %d\n", result, checksum);
    
    /* Additional complex cleanup pattern */
    {
        /* One more scheduling-intensive block */
        double final_acc = 0.0;
        for (i = 0; i < 100; i++) {
            int idx1 = (i * 17) % ARRAY_SIZE;
            int idx2 = (i * 13) % ARRAY_SIZE;
            
            final_acc += fp_array[idx1] * int_array[idx2];
            final_acc = fmod(final_acc, 1000.0);
            
            /* Memory barrier every 10 iterations */
            if (i % 10 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        printf("Final accumulation: %f\n", final_acc);
    }
    
    return 0;
}
