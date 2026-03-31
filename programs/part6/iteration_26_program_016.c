/* test_early_remat.c - Test program to trigger GCC's early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_compute(int x, int y) {
    return (x * 3 + y * 7) & 0xFF;
}

/* Another pure function with different mode */
static double __attribute__((const)) pure_double(double x, double y) {
    return x * 3.14159 + y * 2.71828;
}

/* Struct with mixed types to create varied register modes */
struct mixed_data {
    int id;
    double value;
    float fval;
    char *name;
    long counter;
};

/* Hot function 1: Creates high integer register pressure */
int __attribute__((noinline)) hot_function1(int *array, int size) {
    int sum = 0;
    int i, j;
    
    /* Outer loop creates values that inner loop needs but can't keep in registers */
    for (i = 0; i < size; i++) {
        /* These computations are cheap but create register pressure */
        int base = pure_compute(array[i], i);
        int offset = (base * 3) / 2;
        int scale = (offset + 7) & 0xF;
        int mask = 1 << scale;
        
        /* Inner loop with high pressure prevents keeping values in registers */
        for (j = 0; j < 8; j++) {
            /* Force recomputation by using all values multiple times */
            int t1 = pure_compute(base, j);  /* Candidate for remat */
            int t2 = pure_compute(offset, j);
            int t3 = pure_compute(scale, j);
            
            /* Use all computed values in different ways */
            sum += t1 * t2;
            sum -= t3 & mask;
            sum += (t1 + t2) | t3;
            
            /* More computations to increase pressure */
            int t4 = pure_compute(t1, t2);
            int t5 = pure_compute(t3, t4);
            sum ^= t4 * t5;
            
            /* Conditional to create complex CFG */
            if ((sum & 1) == 0) {
                sum += pure_compute(t5, mask);
            } else {
                sum -= pure_compute(scale, offset);
            }
        }
        
        /* Use goto to create non-trivial control flow */
        if (base % 3 == 0) {
            goto skip_add;
        }
        sum += base;
    skip_add:
        /* Empty label for goto target */
        ;
    }
    
    return sum;
}

/* Hot function 2: Mixes integer and floating point for varied modes */
double __attribute__((noinline)) hot_function2(struct mixed_data *data, int count) {
    double total = 0.0;
    int i;
    
    /* Use pragma to ensure optimization level */
    #pragma GCC optimize ("O2")
    for (i = 0; i < count; i++) {
        /* Create floating point computations */
        double base_val = pure_double(data[i].value, data[i].fval);
        double scaled = base_val * 2.5;
        double offset = scaled + 1.618;
        
        /* Integer computations mixed in */
        int int_part = (int)base_val;
        int scale_int = pure_compute(int_part, data[i].id);
        
        /* Pointer arithmetic */
        char *ptr = data[i].name;
        if (ptr) {
            /* Force register pressure with pointer operations */
            int offset2 = scale_int % 16;
            ptr += offset2;
            total += *ptr * scaled;
        }
        
        /* More mixed computations */
        total += offset * data[i].fval;
        total -= (double)scale_int / 256.0;
        
        /* Nested loop for additional pressure */
        int j;
        for (j = 0; j < 4; j++) {
            /* These should be rematerialized */
            double temp = pure_double(base_val, j);
            int temp_int = pure_compute(scale_int, j);
            
            total += temp * temp_int;
            total -= pure_double(offset, temp);
            
            /* Switch to create complex CFG */
            switch (j % 3) {
                case 0:
                    total += 1.0;
                    break;
                case 1:
                    total *= 1.1;
                    break;
                case 2:
                    total -= 0.5;
                    /* fall through */
                default:
                    total = fabs(total);
            }
        }
        
        /* Use all local variables one more time */
        total += base_val + scaled + offset;
    }
    
    return total;
}

/* Hot function 3: Uses inline assembly to force specific RTL patterns */
int __attribute__((noinline)) hot_function3(int *arr, int n) {
    int result = 0;
    int i;
    
    /* Mark some variables as register for hinting */
    register int reg_a asm ("r12");
    register int reg_b asm ("r13");
    
    for (i = 0; i < n; i++) {
        /* Complex expression that should be rematerialized */
        int comp1 = pure_compute(arr[i], i);
        int comp2 = pure_compute(comp1, arr[i] >> 4);
        int comp3 = pure_compute(comp2, i * 3);
        
        /* Use inline assembly to create artificial register references */
        asm volatile (
            "addl %1, %0\n\t"
            "imull %2, %0\n\t"
            : "+r" (result)
            : "r" (comp1), "r" (comp2)
            : "cc"
        );
        
        /* More computations using all values */
        reg_a = comp3 * 2;
        reg_b = comp1 + comp2;
        
        /* Force use of all computed values */
        result += reg_a - reg_b;
        result ^= pure_compute(reg_a, reg_b);
        
        /* Array indexing with complex expression */
        int idx = pure_compute(i, result) % n;
        result += arr[idx] * comp3;
        
        /* Conditional goto for complex CFG */
        if (result % 7 == 0) {
            goto special_case;
        }
        result += 1;
        continue;
        
    special_case:
        result -= 2;
    }
    
    return result;
}

/* Main function that orchestrates everything */
int main(void) {
    const int ARRAY_SIZE = 256;
    const int STRUCT_COUNT = 128;
    
    /* Initialize data */
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    struct mixed_data *mixed_array = malloc(STRUCT_COUNT * sizeof(struct mixed_data));
    
    if (!int_array || !mixed_array) {
        return 1;
    }
    
    /* Fill arrays with data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) & 0xFF;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        mixed_array[i].id = i;
        mixed_array[i].value = sin(i * 0.1) * 100.0;
        mixed_array[i].fval = cos(i * 0.05) * 50.0f;
        mixed_array[i].counter = i * 1000L;
        mixed_array[i].name = malloc(16);
        if (mixed_array[i].name) {
            snprintf(mixed_array[i].name, 16, "Item%d", i);
        }
    }
    
    /* Call hot functions to trigger rematerialization */
    int result1 = hot_function1(int_array, ARRAY_SIZE);
    double result2 = hot_function2(mixed_array, STRUCT_COUNT);
    int result3 = hot_function3(int_array, ARRAY_SIZE);
    
    /* Combine results in non-trivial way */
    int final_result = (int)(result1 + result2) ^ result3;
    
    /* Print to prevent elimination */
    printf("Final result: %d (int1=%d, double=%f, int3=%d)\n", 
           final_result, result1, result2, result3);
    
    /* Cleanup */
    for (int i = 0; i < STRUCT_COUNT; i++) {
        free(mixed_array[i].name);
    }
    free(int_array);
    free(mixed_array);
    
    return 0;
}
