/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex structure to force register pressure */
struct DataPacket {
    int32_t a[8];
    double b[4];
    float c[16];
    char d[64];
    int64_t e[2];
};

/* Function 1: Deeply nested loops with many live ranges */
__attribute__((noinline))
int complex_loop_pressure(int* arr, int size) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int xor_val = 0;
    int shift_val = 0;
    
    /* Multiple variables with overlapping live ranges */
    for (int i = 0; i < size; i++) {
        int temp1 = arr[i];
        int temp2 = temp1 * global_seed;
        int temp3 = temp2 + i;
        
        for (int j = 0; j < 8; j++) {
            int inner_temp1 = temp3 + j;
            int inner_temp2 = inner_temp1 * 2;
            int inner_temp3 = inner_temp2 - temp1;
            
            for (int k = 0; k < 4; k++) {
                /* Many intermediate values in registers */
                int deep_temp1 = inner_temp3 * k;
                int deep_temp2 = deep_temp1 ^ inner_temp2;
                int deep_temp3 = deep_temp2 << 2;
                int deep_temp4 = deep_temp3 >> 1;
                int deep_temp5 = deep_temp4 + deep_temp1;
                
                sum += deep_temp5;
                prod *= (deep_temp5 & 0xFF) + 1;
                diff -= deep_temp3;
                xor_val ^= deep_temp2;
                shift_val = (shift_val << 1) | (deep_temp4 & 1);
            }
        }
        
        /* Keep variables alive across loop iterations */
        asm volatile("" : "+r"(sum), "+r"(prod), "+r"(diff), 
                      "+r"(xor_val), "+r"(shift_val) : : "memory");
    }
    
    return sum + prod + diff + xor_val + shift_val;
}

/* Function 2: Complex switch with fall-through cases */
__attribute__((noinline))
int switch_cfg_pressure(int value, int* arr, int size) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: {
            int a = arr[0];
            int b = arr[1];
            result = a + b;
            /* Fall through */
        }
        case 1: {
            int c = arr[2] * 2;
            int d = arr[3] / 2;
            result += c - d;
            if (result > 1000) return result;
            /* Fall through */
        }
        case 2: {
            double e = (double)arr[4];
            double f = (double)arr[5];
            result += (int)(e * f);
            break;
        }
        case 3: {
            for (int i = 0; i < 4; i++) {
                result += arr[6 + i] << i;
            }
            /* Fall through */
        }
        case 4:
        case 5: {
            result *= arr[10];
            break;
        }
        case 6: {
            int x = arr[11];
            int y = arr[12];
            int z = arr[13];
            result = (x & y) | (z ^ result);
            /* Fall through */
        }
        case 7: {
            result = ~result;
            break;
        }
        case 8: {
            result = result % 997;
            if (result < 0) result = -result;
            /* Fall through */
        }
        case 9: {
            result = result * 3 + 1;
            while (result > 10000) result >>= 1;
            break;
        }
        case 10: {
            /* Nested switch */
            switch (result % 5) {
                case 0: result += 100; break;
                case 1: result -= 50; break;
                case 2: result *= 2; break;
                case 3: result /= 2; break;
                case 4: result ^= 0xAAAA; break;
            }
            break;
        }
        case 11: {
            /* Early return creates CFG edge */
            if (result == 0) return -1;
            result = result << 4;
            break;
        }
        case 12: {
            result = result | 0xF0F0F0F0;
            break;
        }
        case 13: {
            result = (result + 7) & ~7;
            break;
        }
        case 14: {
            result = -result;
            /* Fall through to default */
        }
        default: {
            result = result % 256;
            break;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
__attribute__((noinline))
int asm_register_pressure(int a, int b, int c, int d, int e, int f, 
                          int g, int h, int i, int j) {
    int result1, result2, result3, result4;
    
    /* Force specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result1)
        : "r" (a), "r" (b)
        : "%eax", "%ebx", "memory"
    );
    
    /* Compete for same registers */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "movl %2, %%edx\n\t"
        "imull %%edx, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r" (result2)
        : "r" (c), "r" (d)
        : "%ecx", "%edx", "memory"
    );
    
    /* More register pressure */
    asm volatile (
        "movl %1, %%eax\n\t"  /* Reuse EAX */
        "movl %2, %%esi\n\t"
        "subl %%esi, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result3)
        : "r" (e), "r" (f)
        : "%eax", "%esi", "memory"
    );
    
    /* Vector operations */
    asm volatile (
        "movd %1, %%xmm0\n\t"
        "movd %2, %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "movd %%xmm0, %0\n\t"
        : "=r" (result4)
        : "r" (g), "r" (h)
        : "%xmm0", "%xmm1", "memory"
    );
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    return result1 + result2 + result3 + result4 + i + j;
}

/* Function 4: Mixed data types and address calculations */
__attribute__((noinline))
double mixed_type_pressure(struct DataPacket* packets, int count) {
    double total = 0.0;
    float faccum = 0.0f;
    int64_t iaccum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Force different register classes */
        for (int j = 0; j < 8; j++) {
            int32_t val = packets[i].a[j];
            double dval = (double)val;
            float fval = (float)val;
            
            total += dval * 1.5;
            faccum += fval * 2.0f;
            iaccum += val * 3;
            
            /* Complex address calculation */
            char* ptr = packets[i].d + j * 8;
            int char_val = *ptr;
            total += (double)char_val * 0.01;
        }
        
        /* SIMD-like operations */
        for (int j = 0; j < 4; j++) {
            double dval = packets[i].b[j];
            total += dval * dval;
            
            /* Force spill/reload */
            asm volatile("" : "+m" (total) : : "memory");
        }
        
        /* Keep all accumulators alive */
        asm volatile("" : "+r" (total), "+r" (faccum), "+r" (iaccum) : : "memory");
    }
    
    return total + (double)faccum + (double)iaccum;
}

/* Function 5: Irreducible control flow with computed goto */
__attribute__((noinline))
int irreducible_cfg(int* arr, int size) {
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int result = 0;
    int index = 0;
    
    /* Create irreducible region */
    goto *labels[arr[0] % 10];
    
label0:
    result += arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 1) % 10];
    
label1:
    result -= arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 2) % 10];
    
label2:
    result *= arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 3) % 10];
    
label3:
    result ^= arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 4) % 10];
    
label4:
    result |= arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 5) % 10];
    
label5:
    result &= arr[index++];
    if (index >= size) goto end;
    goto *labels[(result + 6) % 10];
    
label6:
    result <<= (arr[index++] & 3);
    if (index >= size) goto end;
    goto *labels[(result + 7) % 10];
    
label7:
    result >>= (arr[index++] & 3);
    if (index >= size) goto end;
    goto *labels[(result + 8) % 10];
    
label8:
    result = ~result;
    index++;
    if (index >= size) goto end;
    goto *labels[(result + 9) % 10];
    
label9:
    result = (result + arr[index++]) % 1000;
    if (index >= size) goto end;
    goto *labels[result % 10];
    
end:
    return result;
}

/* Function 6: Many function arguments (register + stack) */
__attribute__((noinline))
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Force register allocation for many args */
    int sum = a1 + a2 + a3 + a4 + a5;
    int prod = a6 * a7 * a8 * a9 * a10;
    int xor_val = a11 ^ a12 ^ a13 ^ a14 ^ a15;
    
    /* Complex expression with all args */
    int result = (sum * prod) ^ xor_val;
    result += (a1 << a2) | (a3 >> a4);
    result *= (a5 + a6 - a7) * (a8 ^ a9);
    result /= (a10 | a11) + 1;
    result -= (a12 & a13) * (a14 ^ a15);
    
    /* Force spills */
    asm volatile("" : "+r" (result) : : 
                 "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11", "r12", "r13",
                 "r14", "r15", "memory");
    
    return result;
}

/* Main test driver */
int main() {
    /* Initialize with random data */
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    struct DataPacket* packets = (struct DataPacket*)malloc(100 * sizeof(struct DataPacket));
    
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand();
    }
    
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 8; j++) packets[i].a[j] = rand();
        for (int j = 0; j < 4; j++) packets[i].b[j] = (double)rand() / RAND_MAX;
        for (int j = 0; j < 16; j++) packets[i].c[j] = (float)rand() / RAND_MAX;
        for (int j = 0; j < 64; j++) packets[i].d[j] = rand() % 256;
        for (int j = 0; j < 2; j++) packets[i].e[j] = rand();
    }
    
    int total_result = 0;
    
    /* Warm-up iterations */
    for (int iter = 0; iter < 5; iter++) {
        total_result ^= complex_loop_pressure(data, 1000);
        asm volatile("" ::: "memory");
    }
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex loops */
        total_result += complex_loop_pressure(data + iter * 10, 500);
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch CFG */
        total_result ^= switch_cfg_pressure(iter, data, 50);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        total_result += asm_register_pressure(
            data[iter], data[iter+1], data[iter+2], data[iter+3],
            data[iter+4], data[iter+5], data[iter+6], data[iter+7],
            data[iter+8], data[iter+9]
        );
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types */
        double dresult = mixed_type_pressure(packets, 10);
        total_result += (int)dresult;
        asm volatile("" ::: "memory");
        
        /* Test 5: Irreducible CFG */
        total_result ^= irreducible_cfg(data + iter * 20, 30);
        asm volatile("" ::: "memory");
        
        /* Test 6: Many arguments */
        total_result += many_arguments(
            data[iter], data[iter+1], data[iter+2], data[iter+3], data[iter+4],
            data[iter+5], data[iter+6], data[iter+7], data[iter+8], data[iter+9],
            data[iter+10], data[iter+11], data[iter+12], data[iter+13], data[iter+14]
        );
        asm volatile("" ::: "memory");
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            printf("Iteration %d, result so far: %d\n", iter, total_result);
        }
    }
    
    printf("Final result: %d\n", total_result);
    printf("Expected with seed 42: %d\n", 1758031673); /* Pre-computed */
    
    free(data);
    free(packets);
    
    return 0;
}
