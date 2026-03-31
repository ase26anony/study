#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variable to prevent optimization of conditionals */
volatile int volatile_flag = 1;

/* Target function with high register pressure */
void reload_heavy_computation(int N, int *checksum) {
    /* Declare many scalar variables to exceed available registers */
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6, a7 = 7, a8 = 8;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    long l1 = 1000L, l2 = 2000L, l3 = 3000L, l4 = 4000L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed packed_data;
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 31 + j * 17) % 256;
        }
    }
    
    /* Initialize packed struct */
    packed_data.d = 3.14159;
    packed_data.i = 42;
    packed_data.c = 'X';
    packed_data.l = 999999L;
    packed_data.f = 2.71828f;
    
    /* Main computation loop with high register pressure */
    for (int iter = 0; iter < N; iter++) {
        /* Complex array access pattern forcing address reloads */
        int idx1 = (iter * 13) % 128;
        int idx2 = (iter * 17) % 128;
        
        /* Chain computations to keep variables live */
        a1 = a2 + a3;
        a2 = a4 * a5;
        a3 = a6 - a7;
        a4 = a8 + a1;
        
        f1 = f2 * f3;
        f2 = f4 / f5;
        f3 = f1 + f2;
        
        d1 = d2 + d3;
        d2 = d4 * d1;
        d3 = d2 - d1;
        
        l1 = l2 + l3;
        l2 = l4 * l1;
        l3 = l2 - l1;
        
        /* Inline assembly with conflicting constraints to force reloads */
        /* Force input/output reloads with tied operands */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a5)
            : "r"(a6), "0"(a7)
            : "cc"
        );
        
        /* Another asm with memory constraint */
        asm volatile (
            "mov %0, %1\n\t"
            : "=r"(a6)
            : "m"(a8)
        );
        
        /* Float/double asm with specific constraints */
        asm volatile (
            "addsd %0, %1, %2\n\t"
            : "=x"(d4)
            : "x"(d1), "x"(d2)
        );
        
        /* Array access with computed indices - forces address reloads */
        int temp = arr[idx1][idx2];
        arr[idx2][idx1] = temp + a1;
        
        /* Conditional block for optional reloads */
        if (volatile_flag) {
            /* Use different subset of variables inside conditional */
            int cond_sum = a1 + a3 + a5 + a7;
            float cond_float = f1 + f3 + f5;
            
            /* More asm in conditional path */
            asm volatile (
                "imul %0, %1, %2\n\t"
                : "=r"(cond_sum)
                : "r"(cond_sum), "r"(iter)
                : "cc"
            );
            
            a8 = cond_sum;
            f4 = cond_float;
        } else {
            /* Alternative path with different variables */
            double cond_double = d1 + d3;
            long cond_long = l1 + l3;
            
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(cond_long)
                : "r"(cond_long), "r"(iter)
                : "cc"
            );
            
            l4 = cond_long;
            d4 = cond_double;
        }
        
        /* Access packed struct through volatile pointer - may need secondary reloads */
        int packed_int = volatile_packed->i;
        double packed_double = volatile_packed->d;
        
        /* Use packed data in computation */
        a1 += packed_int;
        d1 += packed_double;
        
        /* More complex chaining to increase live range */
        a2 = a3 + a4 + a5 + a6;
        f1 = f2 + f3 + f4 + f5;
        d2 = d3 + d4 + packed_double;
        l2 = l3 + l4 + packed_int;
        
        /* Another asm with multiple constraints */
        asm volatile (
            "lea (%1,%2,1), %0\n\t"
            : "=r"(a7)
            : "r"(a8), "r"(iter)
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    sum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    sum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    sum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    sum += (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    /* Add array checksum */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            sum += arr[i][j];
        }
    }
    
    *checksum = sum;
}

/* Another function with different pattern to increase variety */
void secondary_reload_test(int N, int *result) {
    /* Variables with different types */
    short s1 = 1, s2 = 2, s3 = 3, s4 = 4;
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    unsigned int u1 = 100, u2 = 200, u3 = 300, u4 = 400;
    
    /* Array with non-power-of-two size for odd addressing */
    int odd_array[123][45];
    
    /* Initialize */
    for (int i = 0; i < 123; i++) {
        for (int j = 0; j < 45; j++) {
            odd_array[i][j] = i * 45 + j;
        }
    }
    
    for (int i = 0; i < N; i++) {
        /* Mixed type computations */
        u1 = u2 + u3;
        u2 = u4 * u1;
        u3 = u1 - u2;
        
        s1 = s2 + s3;
        s2 = s4 * s1;
        s3 = s1 - s2;
        
        /* Asm with byte/word register constraints (x86 specific) */
        #ifdef __x86_64__
        asm volatile (
            "addb %1, %0\n\t"
            : "+r"(c1)
            : "r"(c2)
            : "cc"
        );
        
        asm volatile (
            "addw %1, %0\n\t"
            : "+r"(s1)
            : "r"(s2)
            : "cc"
        );
        #endif
        
        /* Complex array indexing */
        int idx = (i * 19) % 123;
        int jdx = (i * 23) % 45;
        
        odd_array[idx][jdx] = odd_array[jdx % 123][idx % 45] + u1 + s1;
        
        /* Chain dependencies */
        c4 = c1 + c2 + c3;
        u4 = u1 + u2 + u3 + u4;
        s4 = s1 + s2 + s3 + s4;
    }
    
    /* Compute result */
    int res = 0;
    res += s1 + s2 + s3 + s4;
    res += c1 + c2 + c3 + c4;
    res += u1 + u2 + u3 + u4;
    
    for (int i = 0; i < 123; i++) {
        for (int j = 0; j < 45; j++) {
            res += odd_array[i][j];
        }
    }
    
    *result = res;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    
    srand(time(NULL));
    
    int checksum1 = 0;
    int checksum2 = 0;
    
    /* Call both functions to increase variety of reload patterns */
    reload_heavy_computation(N, &checksum1);
    secondary_reload_test(N / 2, &checksum2);
    
    int final_result = checksum1 + checksum2;
    
    printf("Result: %d\n", final_result);
    
    /* Use result to prevent optimization */
    volatile int dummy = final_result;
    
    return 0;
}
