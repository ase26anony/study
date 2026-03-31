/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
struct DataBlock {
    int values[8];
    double fp_values[4];
    char* ptr;
    volatile int* volatile_ptr;
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int* data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double fp1, fp2, fp3, fp4;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i];
        fp1 = sin(temp1 * 0.01);
        
        for (j = 0; j < 8; j++) {
            temp2 = data[i + j];
            fp2 = cos(temp2 * 0.02);
            
            for (k = 0; k < 4; k++) {
                temp3 = data[i + j + k];
                temp4 = temp1 + temp2 + temp3;
                fp3 = fp1 * fp2 + tan(temp4 * 0.001);
                
                for (l = 0; l < 2; l++) {
                    temp5 = temp3 << l;
                    temp6 = temp4 >> (l + 1);
                    temp7 = temp5 ^ temp6;
                    temp8 = temp7 * (l + 1);
                    
                    fp4 = fp3 + exp(temp8 * 0.0001);
                    sum1 += (int)(fp4 * 1000);
                }
                
                sum2 += temp4;
                global_accumulator += fp3;
            }
            
            sum3 += temp2;
            /* Inline asm with register constraints */
            asm volatile (
                "mov %[val], %%eax\n\t"
                "imul $0x1234, %%eax, %%ebx\n\t"
                "add %%ebx, %[sum]\n\t"
                : [sum] "+r" (sum4)
                : [val] "r" (temp2)
                : "eax", "ebx", "cc"
            );
        }
        
        sum4 += temp1;
        global_counter++;
    }
    
    /* Force all sums to be used */
    data[0] = sum1 + sum2 + sum3 + sum4;
}

/* Function 2: Complex switch with fall-through cases */
int test_complex_switch(int value, int* data, int size) {
    int result = 0;
    int i, temp;
    double fp_temp;
    
    for (i = 0; i < size; i++) {
        switch ((value + i) % SWITCH_CASES) {
            case 0:
                temp = data[i] * 2;
                fp_temp = sin(temp);
                /* Fall through */
            case 1:
                temp += data[i] / 3;
                fp_temp += cos(temp);
                result += (int)(fp_temp * 100);
                break;
            case 2:
                temp = data[i] << 2;
                /* Fall through */
            case 3:
                temp ^= 0xABCD;
                /* Fall through */
            case 4:
                temp |= 0x1234;
                result += temp;
                break;
            case 5:
            case 6:
            case 7:
                temp = data[i] * data[i + 1];
                result += temp % 1000;
                break;
            case 8:
                /* Inline asm with fixed registers */
                asm volatile (
                    "movl %[in], %%eax\n\t"
                    "movl %%eax, %%ecx\n\t"
                    "shrl $4, %%ecx\n\t"
                    "addl %%ecx, %%eax\n\t"
                    "movl %%eax, %[out]\n\t"
                    : [out] "=r" (temp)
                    : [in] "r" (data[i])
                    : "eax", "ecx", "cc"
                );
                result += temp;
                break;
            case 9:
                /* Memory clobber forces spills */
                asm volatile (
                    "mov %0, %%eax\n\t"
                    "add $1, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+m" (global_counter)
                    :
                    : "eax", "memory"
                );
                result += global_counter;
                break;
            case 10:
                /* Multiple asm statements competing for registers */
                asm volatile ("movl $1, %%eax" ::: "eax");
                asm volatile ("movl $2, %%ebx" ::: "ebx");
                asm volatile ("movl $3, %%ecx" ::: "ecx");
                asm volatile ("movl $4, %%edx" ::: "edx");
                result += 4;
                break;
            case 11:
                temp = data[i] * 3;
                fp_temp = log(fabs(temp) + 1.0);
                result += (int)(fp_temp * 1000);
                break;
            case 12:
                temp = data[i] % 256;
                result += temp * temp;
                break;
            case 13:
                /* Vector-like operations */
                int v1 = data[i];
                int v2 = data[i + 1];
                int v3 = data[i + 2];
                int v4 = data[i + 3];
                result += v1 * v2 + v3 * v4;
                i += 3;  // Skip ahead
                break;
            case 14:
                /* Complex expression with many temps */
                result += ((data[i] & 0xFF) << 16) |
                         ((data[i + 1] & 0xFF) << 8) |
                         (data[i + 2] & 0xFF);
                i += 2;
                break;
        }
        
        /* Early return in some cases */
        if (result > 1000000) {
            return result;
        }
    }
    
    return result;
}

/* Function 3: Mixed data types and many function arguments */
double test_mixed_types(
    char c1, short s1, int i1, long l1,
    float f1, double d1,
    char c2, short s2, int i2, long l2,
    float f2, double d2,
    struct DataBlock* block
) {
    /* Many intermediate values in registers */
    double result = 0.0;
    int temp_int;
    double temp_double;
    float temp_float;
    long temp_long;
    
    /* Operations mixing all types */
    temp_int = c1 * s1 + i1;
    temp_long = l1 * temp_int;
    temp_float = f1 * (float)temp_long;
    temp_double = d1 * (double)temp_float;
    
    result += temp_double;
    
    /* More mixing */
    temp_int = c2 << 4;
    temp_int += s2 * 3;
    temp_int ^= i2;
    temp_long = l2 / (temp_int + 1);
    temp_float = f2 * (float)temp_long;
    temp_double = d2 + (double)temp_float;
    
    result += temp_double;
    
    /* Access structure with pointer aliasing */
    int* alias1 = block->values;
    volatile int* alias2 = block->volatile_ptr;
    
    for (int i = 0; i < 8; i++) {
        alias1[i] = alias1[i] * 2 + i;
        *alias2 += alias1[i];  /* Volatile access prevents optimization */
        result += sin(alias1[i] * 0.01);
    }
    
    /* Complex floating point chain */
    for (int i = 0; i < 4; i++) {
        block->fp_values[i] = 
            cos(block->fp_values[i]) * 
            sin(result * 0.1) +
            tan((double)i * 0.5);
        result += block->fp_values[i];
    }
    
    return result;
}

/* Function 4: Irreducible control flow with computed goto */
void test_irreducible_cfg(int* data, int size) {
    void* labels[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int i = 0;
    int state = 0;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    
    start:
    if (i >= size) goto end;
    
    /* Many variables live across the computed goto */
    a = data[i];
    b = data[i + 1];
    c = a * b;
    d = c + i;
    e = d ^ 0x1234;
    f = e >> 2;
    g = f * 3;
    h = g % 256;
    
    /* Computed goto creates irreducible flow */
    state = (a + b + c + d) % 8;
    goto *labels[state];
    
    label0:
    a += b;
    goto update;
    
    label1:
    b += c;
    goto update;
    
    label2:
    c += d;
    goto update;
    
    label3:
    d += e;
    goto update;
    
    label4:
    e += f;
    goto update;
    
    label5:
    f += g;
    goto update;
    
    label6:
    g += h;
    goto update;
    
    label7:
    h += a;
    goto update;
    
    update:
    data[i] = a + b + c + d + e + f + g + h;
    i += 2;
    goto start;
    
    end:
    /* Force all variables to be used */
    global_counter += a + b + c + d + e + f + g + h;
}

/* Function 5: Many function calls within loops */
void test_function_calls(int* data, int size) {
    int i, j;
    double sum = 0.0;
    
    for (i = 0; i < size; i += 16) {
        /* Multiple calls with different arguments */
        sum += sin(data[i] * 0.01);
        sum += cos(data[i + 1] * 0.02);
        sum += tan(data[i + 2] * 0.005);
        sum += exp(data[i + 3] * 0.001);
        sum += log(fabs(data[i + 4]) + 1.0);
        sum += sqrt(fabs(data[i + 5]) + 1.0);
        
        /* Call with many arguments to stress register/stack passing */
        struct DataBlock block;
        for (j = 0; j < 8; j++) block.values[j] = data[i + j];
        for (j = 0; j < 4; j++) block.fp_values[j] = data[i + j] * 0.1;
        block.volatile_ptr = &global_counter;
        
        sum += test_mixed_types(
            data[i] & 0xFF,
            (data[i + 1] >> 8) & 0xFFFF,
            data[i + 2],
            (long)data[i + 3] * 1000,
            (float)data[i + 4] * 0.01f,
            (double)data[i + 5] * 0.001,
            data[i + 6] & 0xFF,
            (data[i + 7] >> 8) & 0xFFFF,
            data[i + 8],
            (long)data[i + 9] * 2000,
            (float)data[i + 10] * 0.02f,
            (double)data[i + 11] * 0.002,
            &block
        );
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    global_accumulator += sum;
}

/* Main function with warm-up and verification */
int main() {
    int i;
    uint64_t checksum = 0;
    clock_t start, end;
    
    /* Allocate and initialize large arrays */
    int* data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 10000;
        data2[i] = rand() % 10000;
    }
    
    printf("Starting register pressure tests...\n");
    start = clock();
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (i = 0; i < ITERATIONS / 10; i++) {
        test_nested_loops(data1, ARRAY_SIZE / 4);
        asm volatile("" ::: "memory");  /* Prevent optimization across calls */
    }
    
    /* Main test phase */
    printf("Main test phase...\n");
    for (i = 0; i < ITERATIONS; i++) {
        /* Call all test functions in sequence */
        test_nested_loops(data1, ARRAY_SIZE / 4);
        checksum += test_complex_switch(i, data2, ARRAY_SIZE / 2);
        
        struct DataBlock block;
        for (int j = 0; j < 8; j++) block.values[j] = data1[j];
        for (int j = 0; j < 4; j++) block.fp_values[j] = data2[j] * 0.1;
        block.volatile_ptr = &global_counter;
        
        checksum += (uint64_t)test_mixed_types(
            i & 0xFF, (i >> 8) & 0xFFFF, i, (long)i * 1000,
            (float)i * 0.01f, (double)i * 0.001,
            (i + 1) & 0xFF, (i >> 9) & 0xFFFF, i + 2, (long)i * 2000,
            (float)i * 0.02f, (double)i * 0.002,
            &block
        );
        
        test_irreducible_cfg(data1, ARRAY_SIZE / 8);
        test_function_calls(data2, ARRAY_SIZE / 2);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
        
        if (i % 10 == 0) {
            printf("Iteration %d, checksum so far: %lu\n", i, checksum);
        }
    }
    
    /* Final verification */
    printf("Verification phase...\n");
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i];
    }
    
    end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\n=== Results ===\n");
    printf("Final checksum: %lu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    printf("Total CPU time: %.2f seconds\n", cpu_time_used);
    printf("Iterations per second: %.2f\n", ITERATIONS / cpu_time_used);
    
    /* Expected checksum for verification */
    uint64_t expected_checksum = 1372670987520ULL;  /* Pre-computed with seed 42 */
    if (checksum == expected_checksum) {
        printf("✓ Checksum verification PASSED\n");
    } else {
        printf("✗ Checksum verification FAILED\n");
        printf("  Expected: %lu\n", expected_checksum);
        printf("  Got:      %lu\n", checksum);
    }
    
    free(data1);
    free(data2);
    
    return 0;
}
