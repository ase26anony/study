/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile long global_counter = 0;

/* Complex data structure with mixed types */
struct MixedData {
    char c_data[32];
    short s_data[16];
    int i_data[8];
    long l_data[4];
    float f_data[4];
    double d_data[2];
    void* ptr_data[2];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct MixedData* data, int size) {
    volatile int keep_alive1 = 0;
    volatile int keep_alive2 = 0;
    volatile int keep_alive3 = 0;
    
    /* Variables declared at function scope but used in nested blocks */
    int temp1, temp2, temp3, temp4, temp5;
    float ftemp1, ftemp2, ftemp3;
    double dtemp1, dtemp2;
    
    /* Complex expression with many intermediate values */
    for (int i = 0; i < size; i++) {
        keep_alive1 = data[i].i_data[0];
        
        for (int j = 0; j < 8; j++) {
            keep_alive2 = data[i].i_data[j];
            
            /* Multiple intermediate calculations */
            temp1 = data[i].i_data[0] * 3;
            temp2 = data[i].i_data[1] + temp1;
            temp3 = data[i].i_data[2] - temp2;
            temp4 = data[i].i_data[3] / (temp3 ? temp3 : 1);
            temp5 = temp1 + temp2 + temp3 + temp4;
            
            /* Floating point calculations */
            ftemp1 = data[i].f_data[0] * 1.5f;
            ftemp2 = data[i].f_data[1] + ftemp1;
            ftemp3 = ftemp1 * ftemp2 - data[i].f_data[2];
            
            for (int k = 0; k < 4; k++) {
                keep_alive3 = data[i].s_data[k];
                
                /* More intermediate values */
                dtemp1 = data[i].d_data[0] * 0.75;
                dtemp2 = data[i].d_data[1] + dtemp1;
                
                /* Complex expression spanning multiple lines */
                data[i].i_data[j % 4] = (temp5 * k) + 
                                       (int)(ftemp3 * 100) + 
                                       (int)(dtemp2 * 50) +
                                       data[i].s_data[k];
                
                /* Early continue to create complex CFG */
                if (data[i].i_data[j % 4] % 7 == 0) {
                    continue;
                }
                
                /* Break at different nesting levels */
                if (data[i].i_data[j % 4] > 1000000) {
                    break;
                }
            }
            
            /* Another level of nesting */
            if (i % 3 == 0) {
                int inner_temp = 0;
                for (int m = 0; m < 3; m++) {
                    inner_temp += data[i].i_data[m] * m;
                    if (inner_temp > 1000) {
                        goto early_exit;
                    }
                }
                data[i].l_data[0] = inner_temp;
            }
        }
        
        early_exit:
        /* Use all the volatile variables to keep them alive */
        data[i].c_data[0] = (char)(keep_alive1 + keep_alive2 + keep_alive3);
    }
}

/* Function 2: Complex switch statement with fall-through */
int test_complex_switch(int value, struct MixedData* data) {
    int result = 0;
    volatile int switch_temp = 0;
    
    /* Variables that will have long live ranges */
    int case_var1 = value * 2;
    int case_var2 = value + 100;
    float case_float = value * 1.5f;
    double case_double = value * 2.5;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[value % 100].i_data[0];
            /* Fall through */
        case 1:
            result += data[value % 100].i_data[1];
            case_var1 *= 2;
            /* Fall through */
        case 2:
            result += data[value % 100].i_data[2];
            case_var2 += case_var1;
            /* Fall through */
        case 3:
            result += data[value % 100].i_data[3];
            case_float *= 1.1f;
            /* Fall through */
        case 4:
            result += data[value % 100].i_data[4];
            case_double += case_float;
            break;
        case 5:
            result = data[value % 100].i_data[5] - case_var1;
            break;
        case 6:
            result = data[value % 100].i_data[6] + case_var2;
            break;
        case 7:
            result = (int)(data[value % 100].f_data[0] * case_float);
            break;
        case 8:
            result = (int)(data[value % 100].d_data[0] + case_double);
            break;
        case 9:
            result = case_var1 * case_var2;
            break;
        case 10:
            result = (int)(case_float * 100);
            break;
        case 11:
            result = (int)(case_double * 50);
            break;
        case 12:
            /* Nested switch */
            switch (value % 5) {
                case 0: result = 1; break;
                case 1: result = 2; break;
                case 2: result = 3; break;
                case 3: result = 4; break;
                case 4: result = 5; break;
            }
            break;
        case 13:
            result = value * value;
            break;
        case 14:
            result = value + case_var1 + case_var2;
            break;
        default:
            result = -1;
    }
    
    /* Use all variables to extend live ranges */
    switch_temp = case_var1 + case_var2 + (int)case_float + (int)case_double;
    return result + switch_temp;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(struct MixedData* data, int index) {
    int a, b, c, d;
    long la, lb;
    float fa, fb;
    double da, db;
    
    /* Force specific register allocation with constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (a)
        : "r" (data[index].i_data[0]), "r" (data[index].i_data[1])
        : "%eax", "memory"
    );
    
    asm volatile (
        "movq %1, %%rbx\n\t"
        "imulq %2, %%rbx\n\t"
        "movq %%rbx, %0\n\t"
        : "=r" (la)
        : "r" (data[index].l_data[0]), "r" (data[index].l_data[1])
        : "%rbx", "memory"
    );
    
    /* More assembly with different constraints */
    asm volatile (
        "movss %1, %%xmm0\n\t"
        "mulss %2, %%xmm0\n\t"
        "movss %%xmm0, %0\n\t"
        : "=x" (fa)
        : "x" (data[index].f_data[0]), "x" (data[index].f_data[1])
        : "%xmm0", "memory"
    );
    
    asm volatile (
        "movsd %1, %%xmm1\n\t"
        "addsd %2, %%xmm1\n\t"
        "movsd %%xmm1, %0\n\t"
        : "=x" (da)
        : "x" (data[index].d_data[0]), "x" (data[index].d_data[1])
        : "%xmm1", "memory"
    );
    
    /* Compete for the same registers */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "subl %2, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r" (b)
        : "r" (data[index].i_data[2]), "r" (a)
        : "%ecx", "memory"
    );
    
    asm volatile (
        "movl %1, %%ecx\n\t"  /* Competing for ECX again */
        "addl %2, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r" (c)
        : "r" (data[index].i_data[3]), "r" (b)
        : "%ecx", "memory"
    );
    
    /* Use all results */
    data[index].i_data[0] = a + b + c;
    data[index].l_data[0] = la;
    data[index].f_data[0] = fa;
    data[index].d_data[0] = da;
}

/* Function 4: Many function arguments to stress register/stack passing */
long test_many_args(int a1, int a2, int a3, int a4, int a5,
                    int a6, int a7, int a8, int a9, int a10,
                    float f1, float f2, float f3, double d1, double d2,
                    char* ptr1, short* ptr2, int* ptr3) {
    
    /* Complex calculations using all arguments */
    long result = 0;
    result += a1 * a2;
    result += a3 - a4;
    result += a5 / (a6 ? a6 : 1);
    result += a7 % (a8 ? a8 : 1);
    result += a9 << (a10 % 16);
    
    result += (long)(f1 * f2 * 100);
    result += (long)(f3 * 1000);
    result += (long)(d1 * d2 * 50);
    
    if (ptr1) result += *ptr1;
    if (ptr2) result += *ptr2;
    if (ptr3) result += *ptr3;
    
    return result;
}

/* Function 5: Vector operations using multiple SIMD registers */
void test_vector_ops(struct MixedData* data, int size) {
    /* Multiple vector-like operations */
    for (int i = 0; i < size - 3; i += 4) {
        /* Process 4 elements at a time */
        int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
        float fsum1 = 0.0f, fsum2 = 0.0f;
        double dsum1 = 0.0, dsum2 = 0.0;
        
        for (int j = 0; j < 4; j++) {
            sum1 += data[i + j].i_data[0];
            sum2 += data[i + j].i_data[1];
            sum3 += data[i + j].i_data[2];
            sum4 += data[i + j].i_data[3];
            
            fsum1 += data[i + j].f_data[0];
            fsum2 += data[i + j].f_data[1];
            
            dsum1 += data[i + j].d_data[0];
            dsum2 += data[i + j].d_data[1];
        }
        
        /* Store results back */
        data[i].i_data[0] = sum1;
        data[i].i_data[1] = sum2;
        data[i].i_data[2] = sum3;
        data[i].i_data[3] = sum4;
        
        data[i].f_data[0] = fsum1;
        data[i].f_data[1] = fsum2;
        
        data[i].d_data[0] = dsum1;
        data[i].d_data[1] = dsum2;
        
        /* Complex conditional with early exit */
        if (sum1 > 1000000 || sum2 < -1000000) {
            break;
        }
    }
}

/* Function 6: Irreducible control flow with computed goto */
void test_irreducible_cfg(struct MixedData* data, int size) {
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int state = 0;
    volatile int counter = 0;
    
    for (int i = 0; i < size && i < 1000; i++) {
        state = data[i].i_data[0] % 10;
        
        /* Computed goto creates irreducible CFG */
        goto *labels[state];
        
        label0:
            data[i].i_data[0] += 1;
            counter++;
            continue;
        label1:
            data[i].i_data[0] *= 2;
            counter += 2;
            continue;
        label2:
            data[i].i_data[0] -= 3;
            counter += 3;
            continue;
        label3:
            data[i].i_data[0] /= 4;
            counter += 4;
            continue;
        label4:
            data[i].i_data[0] %= 5;
            counter += 5;
            continue;
        label5:
            data[i].i_data[0] <<= 1;
            counter += 6;
            continue;
        label6:
            data[i].i_data[0] >>= 1;
            counter += 7;
            continue;
        label7:
            data[i].i_data[0] ^= 0xFF;
            counter += 8;
            continue;
        label8:
            data[i].i_data[0] |= 0xAA;
            counter += 9;
            continue;
        label9:
            data[i].i_data[0] &= 0x55;
            counter += 10;
            continue;
    }
    
    global_counter += counter;
}

/* Function 7: Pointer aliasing to prevent optimizations */
void test_pointer_aliasing(struct MixedData* data1, struct MixedData* data2, int size) {
    /* Create aliases */
    int* alias1 = data1[0].i_data;
    int* alias2 = data2[0].i_data;
    
    volatile int* volatile_alias = &data1[0].i_data[0];
    
    /* Complex loop with aliasing */
    for (int i = 0; i < size; i++) {
        /* These could alias, preventing dead store elimination */
        *alias1 = data1[i].i_data[0] + data2[i].i_data[0];
        *alias2 = data1[i].i_data[1] - data2[i].i_data[1];
        
        /* Volatile access forces memory operations */
        *volatile_alias = *alias1 + *alias2;
        
        /* More complex aliasing */
        int* temp_ptr = (i % 2) ? alias1 : alias2;
        *temp_ptr += i;
        
        /* Pointer arithmetic */
        alias1 = &data1[(i + 1) % size].i_data[0];
        alias2 = &data2[(i + 1) % size].i_data[0];
        volatile_alias = &data1[(i + 2) % size].i_data[0];
    }
}

/* Main function that orchestrates all tests */
int main() {
    /* Allocate large arrays */
    struct MixedData* data1 = (struct MixedData*)calloc(ARRAY_SIZE, sizeof(struct MixedData));
    struct MixedData* data2 = (struct MixedData*)calloc(ARRAY_SIZE, sizeof(struct MixedData));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 8; j++) {
            data1[i].i_data[j] = rand() % 1000;
            data2[i].i_data[j] = rand() % 1000;
        }
        for (int j = 0; j < 4; j++) {
            data1[i].f_data[j] = (float)rand() / RAND_MAX * 100.0f;
            data2[i].f_data[j] = (float)rand() / RAND_MAX * 100.0f;
            data1[i].l_data[j] = rand() * 1000L;
            data2[i].l_data[j] = rand() * 1000L;
        }
        for (int j = 0; j < 2; j++) {
            data1[i].d_data[j] = (double)rand() / RAND_MAX * 200.0;
            data2[i].d_data[j] = (double)rand() / RAND_MAX * 200.0;
        }
    }
    
    long total_checksum = 0;
    
    /* Warm-up iterations */
    printf("Starting warm-up iterations...\n");
    for (int warmup = 0; warmup < 10; warmup++) {
        test_nested_loops(data1, 1000);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    printf("Running main tests...\n");
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        test_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch */
        for (int i = 0; i < 1000; i++) {
            total_checksum += test_complex_switch(i, data1);
        }
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        for (int i = 0; i < 500; i++) {
            test_inline_asm(data1, i % ARRAY_SIZE);
        }
        asm volatile("" ::: "memory");
        
        /* Test 4: Many arguments */
        for (int i = 0; i < 100; i++) {
            total_checksum += test_many_args(
                i, i+1, i+2, i+3, i+4,
                i+5, i+6, i+7, i+8, i+9,
                i*1.1f, i*1.2f, i*1.3f,
                i*1.4, i*1.5,
                (char*)&data1[i].c_data[0],
                &data1[i].s_data[0],
                &data1[i].i_data[0]
            );
        }
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_ops(data1, ARRAY_SIZE / 20);
        asm volatile("" ::: "memory");
        
        /* Test 6: Irreducible CFG */
        test_irreducible_cfg(data1, ARRAY_SIZE / 50);
        asm volatile("" ::: "memory");
        
        /* Test 7: Pointer aliasing */
        test_pointer_aliasing(data1, data2, ARRAY_SIZE / 100);
        asm volatile("" ::: "memory");
        
        /* Update global counter */
        global_counter += iter;
    }
    
    /* Final checksum calculation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 8; j++) {
            total_checksum += data1[i].i_data[j];
            total_checksum += data2[i].i_data[j];
        }
        total_checksum += (long)data1[i].f_data[0];
        total_checksum += (long)data2[i].f_data[0];
        total_checksum += (long)data1[i].d_data[0];
        total_checksum += (long)data2[i].d_data[0];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    printf("Global counter: %ld\n", (long)global_counter);
    
    free(data1);
    free(data2);
    
    return 0;
}
