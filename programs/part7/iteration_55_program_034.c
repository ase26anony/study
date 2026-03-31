/* tree_coverage.c - Program to exercise GCC tree node creation */
#include <stdio.h>
#include <stdlib.h>

/* For OMP_CLAUSE generation */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ========== CONSTRUCTOR nodes ========== */
/* Complex initializers with designated initializers and compound literals */
struct Point {
    int x;
    int y;
    int z;
};

struct Nested {
    struct Point p;
    int arr[3];
};

/* Function to prevent optimization */
__attribute__((noinline)) 
static int process_constructor_data(void) {
    /* Complex array initializer with designated indices */
    int arr[5] = {
        [0] = 1,
        [1] = ({ volatile int t = 5; t; }),  /* Compound literal creates CONSTRUCTOR */
        [2] = 3,
        [3] = ({ int u = 7; u * 2; }),       /* Another compound literal */
        [4] = 9
    };
    
    /* Structure with nested designated initializer */
    struct Nested n = {
        .p = { .x = ({ int a = 10; a; }),    /* CONSTRUCTOR in nested initializer */
               .y = 20,
               .z = 30 },
        .arr = { [0] = 100, [1] = 200, [2] = 300 }
    };
    
    /* Compound literal in expression */
    struct Point p2 = (struct Point){ 
        .x = ({ volatile int v = 42; v; }),
        .y = 43,
        .z = 44 
    };
    
    return arr[1] + n.p.x + p2.x;
}

/* ========== SSA_NAME nodes ========== */
/* Function with complex control flow to generate SSA form */
__attribute__((noinline, noipa))
static int ssa_intensive_function(int a, int b) {
    int x = a;
    int y = b;
    int z = 0;
    
    /* Complex arithmetic with multiple assignments */
    for (int i = 0; i < 10; i++) {
        x = x * i + 1;
        y = y + x;
        
        /* Conditional creates phi nodes in SSA */
        if (x > 50) {
            x = x / 2;
            z = z + 1;
        } else {
            x = x + 5;
            z = z - 1;
        }
        
        /* Another condition for more SSA complexity */
        switch (i % 3) {
            case 0: y = y * 2; break;
            case 1: y = y / 2; break;
            case 2: y = y + 100; break;
        }
    }
    
    /* Final computation with all variables */
    return x + y + z;
}

/* ========== BLOCK nodes ========== */
/* Function using labels and goto for BLOCK creation */
static int use_blocks(int val) {
    int result = val;
    
    /* Outer block with label */
    {
        __label__ outer_label;
        if (result > 100) {
            goto outer_label;
        }
        
        /* Inner block with its own label */
        {
            __label__ inner_label;
            if (result < 0) {
                goto inner_label;
            }
            result *= 2;
            inner_label: ;
        }
        
        result += 10;
        outer_label: ;
    }
    
    /* Another block with computed goto (GCC extension) */
    {
        void *labels[] = { &&label1, &&label2, &&label3 };
        int choice = result % 3;
        
        goto *labels[choice];
        
        label1:
            result += 1;
            goto end;
        label2:
            result += 2;
            goto end;
        label3:
            result += 3;
            goto end;
        end: ;
    }
    
    return result;
}

/* ========== TREE_VEC nodes ========== */
/* Function using __builtin_types_compatible_p for TREE_VEC */
static int type_checking(void) {
    int int_var = 0;
    double double_var = 0.0;
    char char_var = 'a';
    int arr[4];
    struct Point pt;
    
    /* Chain of type compatibility checks - each creates TREE_VEC */
    int type_matches = 0;
    
    /* Multiple __builtin_types_compatible_p calls in complex expression */
    if (__builtin_types_compatible_p(typeof(int_var), int) ||
        __builtin_types_compatible_p(typeof(double_var), double) ||
        __builtin_types_compatible_p(typeof(char_var), char) ||
        __builtin_types_compatible_p(typeof(arr), int[4]) ||
        __builtin_types_compatible_p(typeof(&pt), struct Point*) ||
        __builtin_types_compatible_p(typeof(main), int(void)) ||
        __builtin_types_compatible_p(typeof(process_constructor_data), int(void)) ||
        __builtin_types_compatible_p(typeof(ssa_intensive_function), int(int, int))) {
        type_matches++;
    }
    
    /* More complex type comparisons */
    if (__builtin_types_compatible_p(int[4], int[4]) &&
        !__builtin_types_compatible_p(int*, double*)) {
        type_matches++;
    }
    
    /* Nested type checks */
    type_matches += __builtin_types_compatible_p(typeof(type_matches), int) ? 1 : 0;
    
    return type_matches;
}

/* ========== OMP_CLAUSE nodes ========== */
/* Function with OpenMP pragmas */
static int omp_computation(int size) {
    int sum = 0;
    int i;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(size) reduction(+:sum) \
                schedule(static) if(size > 1000)
    for (i = 0; i < size; i++) {
        sum += i;
    }
    
    /* Another OpenMP section with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(i) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                if (i > max_val) max_val = i;
            }
        }
        
        #pragma omp section
        {
            for (i = 100; i < 200; i++) {
                if (i > max_val) max_val = i;
            }
        }
    }
    
    return sum + max_val;
}

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate various identifier types */
#define DECLARE_VARS(base) \
    int base##_var_1 = 0; \
    int base##_var_2 = 0; \
    int base##_var_3 = 0

#define CONCAT(a, b) a##b
#define UNIQUE_VAR(base) CONCAT(base, __COUNTER__)

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* 1. Generate CONSTRUCTOR nodes */
    checksum += process_constructor_data();
    printf("After constructors: %d\n", checksum);
    
    /* 2. Generate SSA_NAME nodes */
    checksum += ssa_intensive_function(1, 2);
    printf("After SSA: %d\n", checksum);
    
    /* 3. Generate BLOCK nodes */
    checksum += use_blocks(checksum);
    printf("After blocks: %d\n", checksum);
    
    /* 4. Generate TREE_VEC nodes */
    checksum += type_checking();
    printf("After type checking: %d\n", checksum);
    
    /* 5. Generate OMP_CLAUSE nodes */
    checksum += omp_computation(500);
    printf("After OpenMP: %d\n", checksum);
    
    /* 6. Generate various IDENTIFIER_NODE types */
    /* Function names (already have many) */
    /* Variable identifiers with macro expansion */
    DECLARE_VARS(my);
    my_var_1 = 10;
    my_var_2 = 20;
    my_var_3 = 30;
    checksum += my_var_1 + my_var_2 + my_var_3;
    
    /* Unique identifiers using __COUNTER__ */
    int UNIQUE_VAR(temp_) = 100;
    int UNIQUE_VAR(temp_) = 200;
    int UNIQUE_VAR(temp_) = 300;
    
    /* Label identifiers */
    {
        __label__ exit_label, error_label, retry_label;
        checksum += 5;
        goto exit_label;
        
        error_label:
            checksum += 10;
            goto retry_label;
            
        retry_label:
            checksum += 15;
            
        exit_label:
            checksum += 20;
    }
    
    /* 7. Print final checksum to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
