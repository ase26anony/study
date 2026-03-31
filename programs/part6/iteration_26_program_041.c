/* test_early_remat.c - Test program to trigger GCC's early rematerialization */
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

/* Structure with mixed types to create varied register modes */
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
    
    /* Outer loop creates values that inner loop uses */
    for (i = 0; i < size; i++) {
        /* Compute several values that can't all stay in registers */
        int base = data[i].id;
        int offset = pure_compute(base, i);
        int scaled = offset * 3;
        int shifted = scaled >> 2;
        int masked = shifted & 0x3F;
        
        /* Inner loop with high pressure */
        for (j = 0; j < 8; j++) {
            /* Many live values competing for registers */
            int temp1 = base + j;
            int temp2 = offset - j;
            int temp3 = scaled / (j + 1);
            int temp4 = shifted ^ j;
            int temp5 = masked | j;
            
            /* Use all temporaries in computation */
            int result = pure_compute(temp1, temp2) +
                        pure_compute(temp3, temp4) +
                        temp5;
            
            /* Complex addressing with pointer arithmetic */
            char *ptr = data[i].name + result % 16;
            total += *ptr + result;
            
            /* Force register pressure with more computations */
            double dval = pure_double(data[i].value, j * 0.1);
            total += (long)(dval * 100);
        }
        
        /* Use goto to create non-trivial CFG */
        if (base % 7 == 0) {
            goto special_case;
        }
        
        /* Normal path continues */
        data[i].counter += total;
        continue;
        
    special_case:
        /* Different computation path */
        data[i].counter -= total;
    }
    
    return total;
}

/* Hot function 2: Uses inline assembly to force specific RTL patterns */
static int hot_function2(int *array, int n) {
    int i, sum = 0;
    
    /* Disable some optimizations for this function */
    #pragma GCC optimize ("O2")
    #pragma GCC push_options
    
    for (i = 0; i < n; i++) {
        /* Create value that's expensive but pure */
        int val = pure_compute(array[i], i);
        
        /* Use inline assembly to create register references */
        int result1, result2;
        
        /* First use of val */
        asm volatile ("/* asm1 */" : "=r"(result1) : "r"(val), "r"(i));
        
        /* Several other computations to increase pressure */
        int temp1 = val * 3;
        int temp2 = val / 2;
        int temp3 = val + array[i];
        int temp4 = val ^ 0x55AA;
        
        /* Second use of val through inline asm */
        asm volatile ("/* asm2 */" : "=r"(result2) : "r"(val), "r"(temp1));
        
        /* Use all values to prevent elimination */
        sum += result1 + result2 + temp1 + temp2 + temp3 + temp4;
        
        /* Array access with complex index */
        int idx = (val + i) % n;
        sum += array[idx];
    }
    
    #pragma GCC pop_options
    return sum;
}

/* Hot function 3: Mixed floating point and integer pressure */
static double hot_function3(double *data, int count) {
    double total = 0.0;
    int i;
    
    /* Register hint for critical variable */
    register int loop_counter asm ("ebx") = count;
    
    for (i = 0; i < loop_counter; i++) {
        /* Multiple floating point computations */
        double base = data[i];
        double d1 = pure_double(base, 1.0);
        double d2 = pure_double(base, 2.0);
        double d3 = pure_double(base, 3.0);
        double d4 = pure_double(base, 4.0);
        
        /* Integer computations intermixed */
        int i1 = (int)(d1 * 100);
        int i2 = (int)(d2 * 100);
        int i3 = (int)(d3 * 100);
        int i4 = (int)(d4 * 100);
        
        /* Complex conditional to prevent optimization */
        switch (i % 5) {
            case 0:
                total += d1 * i1;
                break;
            case 1:
                total += d2 * i2 - d1;
                break;
            case 2:
                total += d3 * i3 / (i2 + 1);
                break;
            case 3:
                total += d4 * i4 + d3 * i1;
                break;
            default:
                total += (d1 + d2 + d3 + d4) / 4.0;
                /* Use goto for CFG complexity */
                if (total > 1000.0) goto reset;
        }
        
        continue;
        
    reset:
        total = 0.0;
    }
    
    return total;
}

/* Hot function 4: Nested loops with struct access */
static long hot_function4(struct mixed_data *data, int rows, int cols) {
    long grand_total = 0;
    int r, c;
    
    for (r = 0; r < rows; r++) {
        /* Outer loop computation - candidate for remat */
        struct mixed_data *row = &data[r * cols];
        long row_base = row->id * 1000L;
        double scale = row->value;
        
        for (c = 0; c < cols; c++) {
            /* High pressure in inner loop */
            struct mixed_data *cell = &row[c];
            
            /* Many computations using outer loop values */
            int id_part = pure_compute(cell->id, c);
            double val_part = pure_double(cell->value, scale);
            float f_part = cell->fval * (r + 1);
            
            /* Pointer chasing */
            char *name_ptr = cell->name;
            int name_len = 0;
            while (name_ptr && *name_ptr) {
                name_len += *name_ptr++;
            }
            
            /* Final computation using all parts */
            long cell_total = (long)(id_part * val_part * f_part) + 
                             name_len + row_base;
            
            grand_total += cell_total;
            
            /* Update in place to create dependencies */
            cell->counter += cell_total % 1000;
        }
    }
    
    return grand_total;
}

int main(void) {
    const int DATA_SIZE = 100;
    const int ARRAY_SIZE = 500;
    const int ROWS = 20;
    const int COLS = 25;
    
    /* Allocate and initialize test data */
    struct mixed_data *data = calloc(DATA_SIZE, sizeof(struct mixed_data));
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i].id = i;
        data[i].value = sin(i * 0.1);
        data[i].fval = cos(i * 0.05);
        data[i].name = "test_name";
        data[i].counter = 0;
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = pure_compute(i, i * 2);
        double_array[i] = pure_double(i * 0.5, i * 0.25);
    }
    
    /* Call hot functions to trigger rematerialization */
    long result1 = hot_function1(data, DATA_SIZE / 2);
    int result2 = hot_function2(int_array, ARRAY_SIZE);
    double result3 = hot_function3(double_array, ARRAY_SIZE);
    long result4 = hot_function4(data, ROWS, COLS);
    
    /* Final computation to prevent dead code elimination */
    long final_result = result1 + result2 + (long)result3 + result4;
    
    /* Print result to prevent optimization */
    printf("Final result: %ld\n", final_result);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(double_array);
    
    return 0;
}
