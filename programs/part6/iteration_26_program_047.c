/* test_early_remat.c - Test program to trigger GCC's early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_compute(int x, int y) {
    return (x * 3) + (y / 2);
}

/* Another pure function with different mode */
static double __attribute__((const)) pure_double(double x, double y) {
    return (x * 3.14159) + (y / 2.71828);
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
static long hot_function1(struct mixed_data *data, int size) {
    long total = 0;
    int i, j;
    
    /* Outer loop creates values that need to be rematerialized in inner loop */
    for (i = 0; i < size; i++) {
        /* These computations are cheap but create register pressure */
        int base_val = data[i].id * 3;
        int offset = (int)(data[i].value * 100.0);
        int scale = pure_compute(data[i].id, offset);
        
        /* Inner loop with high pressure - prevents keeping values in registers */
        for (j = 0; j < 8; j++) {
            /* Multiple uses of computed values force register allocation decisions */
            int temp1 = base_val + j * scale;
            int temp2 = offset - j * 2;
            int temp3 = pure_compute(temp1, temp2);
            
            /* Complex addressing mode */
            int idx = (temp1 + temp2 + temp3) % size;
            if (idx < 0) idx = -idx;
            
            /* Use all temporaries in computation */
            total += data[idx].counter * temp1;
            total -= data[idx].id * temp2;
            total += temp3 * j;
            
            /* Force spill/reload by using many variables */
            int extra1 = base_val * 2;
            int extra2 = offset / 3;
            int extra3 = scale + 1;
            total += extra1 + extra2 + extra3;
        }
        
        /* Control flow to inhibit CSE */
        if (base_val % 7 == 0) {
            goto special_case;
        } else if (base_val % 5 == 0) {
            total += base_val * 100;
        } else {
            total += offset * 50;
        }
        
        continue;
        
    special_case:
        total += scale * 200;
    }
    
    return total;
}

/* Hot function 2: Mixes floating point and integer operations */
static double hot_function2(struct mixed_data *data, int size) {
    double result = 0.0;
    int i;
    
    /* Use pragma to ensure optimization level */
    #pragma GCC optimize ("O2")
    for (i = 0; i < size; i++) {
        /* Create floating point register pressure */
        double dval = data[i].value;
        float fval = data[i].fval;
        
        /* Expensive but pure computation - good candidate for remat */
        double computed = pure_double(dval, fval);
        
        /* Multiple uses with different addressing modes */
        result += computed * i;
        result -= pure_double(dval * 2.0, fval / 2.0);
        
        /* Integer computations intermixed */
        int int_part = (int)dval;
        long counter = data[i].counter;
        
        /* Complex expression that can't be easily optimized away */
        result += (double)(int_part * counter) / (i + 1);
        
        /* Array indexing with computed index */
        int idx = (int_part + i) % size;
        if (idx < 0) idx = -idx;
        
        /* Use inline assembly to create specific register references */
        asm volatile (
            "/* Force register usage */"
            : "+r" (int_part), "+r" (counter)
            :
            : "memory"
        );
        
        result += data[idx].value * computed;
    }
    
    return result;
}

/* Hot function 3: Complex control flow with gotos */
static int hot_function3(int *array, int size) {
    int sum = 0;
    int i = 0;
    
    /* Non-trivial control flow graph */
start_loop:
    if (i >= size) goto end;
    
    /* Create values that need rematerialization */
    int base = array[i];
    int scaled = base * 3;
    int shifted = base >> 2;
    
    /* Multiple basic blocks */
    if (base % 2 == 0) {
        goto even_case;
    } else {
        goto odd_case;
    }
    
even_case:
    {
        /* Use register keyword to hint importance */
        register int temp = scaled + shifted;
        sum += temp * i;
        
        /* Nested computation */
        int nested = pure_compute(temp, i);
        sum += nested;
        
        i++;
        goto start_loop;
    }
    
odd_case:
    {
        int temp = scaled - shifted;
        sum -= temp * i;
        
        /* Different computation path */
        int nested = pure_compute(i, temp);
        sum += nested * 2;
        
        i += 2;
        goto start_loop;
    }
    
end:
    return sum;
}

/* Hot function 4: Pointer chasing with mixed types */
static long hot_function4(struct mixed_data **ptr_array, int count) {
    long total = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        struct mixed_data *current = ptr_array[i];
        if (!current) continue;
        
        /* Pointer computations create address register pressure */
        char *name_ptr = current->name;
        int name_len = 0;
        
        /* String length computation - creates loop within loop */
        while (name_ptr && *name_ptr) {
            name_len++;
            name_ptr++;
            
            /* Intermix with other computations */
            int temp = current->id * name_len;
            double dtemp = current->value * name_len;
            
            total += (long)(temp + (int)dtemp);
        }
        
        /* More computations with the struct */
        total += (long)(current->value * 1000.0);
        total += current->counter;
        
        /* Switch statement for varied control flow */
        switch (current->id % 4) {
            case 0:
                total += name_len * 10;
                break;
            case 1:
                total += name_len * 20;
                break;
            case 2:
                total += name_len * 30;
                /* fall through */
            case 3:
                total += name_len * 40;
                break;
        }
    }
    
    return total;
}

int main(void) {
    const int DATA_SIZE = 100;
    const int PTR_COUNT = 50;
    int i;
    
    /* Allocate and initialize test data */
    struct mixed_data *data = malloc(DATA_SIZE * sizeof(struct mixed_data));
    struct mixed_data **ptr_array = malloc(PTR_COUNT * sizeof(struct mixed_data*));
    
    if (!data || !ptr_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data structures */
    for (i = 0; i < DATA_SIZE; i++) {
        data[i].id = i;
        data[i].value = sin(i * 0.1) * 100.0;
        data[i].fval = cos(i * 0.05) * 50.0f;
        data[i].counter = i * 1000L;
        
        /* Allocate small strings */
        data[i].name = malloc(20);
        if (data[i].name) {
            snprintf(data[i].name, 20, "Item%d", i);
        }
    }
    
    /* Setup pointer array */
    for (i = 0; i < PTR_COUNT; i++) {
        ptr_array[i] = &data[i % DATA_SIZE];
    }
    
    /* Create integer array for function3 */
    int *int_array = malloc(DATA_SIZE * sizeof(int));
    for (i = 0; i < DATA_SIZE; i++) {
        int_array[i] = i * 3 - 50;
    }
    
    /* Call hot functions to trigger rematerialization */
    long result1 = hot_function1(data, DATA_SIZE);
    double result2 = hot_function2(data, DATA_SIZE);
    int result3 = hot_function3(int_array, DATA_SIZE);
    long result4 = hot_function4(ptr_array, PTR_COUNT);
    
    /* Combine results in non-trivial way to prevent optimization */
    long final_result = (long)(result1 + (long)result2 + result3 + result4);
    
    /* Add some more computation to increase pressure */
    for (i = 0; i < 1000; i++) {
        /* Quick computation that might get rematerialized */
        int temp = pure_compute(i, final_result % 100);
        final_result += temp;
        
        /* Mix with floating point */
        double dtemp = pure_double(i * 0.01, final_result * 0.001);
        final_result += (long)(dtemp * 100.0);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %ld\n", final_result);
    
    /* Cleanup */
    for (i = 0; i < DATA_SIZE; i++) {
        free(data[i].name);
    }
    free(data);
    free(ptr_array);
    free(int_array);
    
    return 0;
}
