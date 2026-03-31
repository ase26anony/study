/* test_complex_types.c - Designed to exercise gengtype-parse.cc balanced delimiter parsing */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_ptr_simple)(void);
typedef fn_ptr_simple (*fn_ptr_returning_fn_ptr)(int, double);
typedef int (*(*fn_ptr_complex)(int (*)(double)))(char);

/* Function to be pointed to by complex function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
int returns_int_from_char(char c) { return (int)c; }
int (*(*returns_complex_fn_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int multi_dim_array[2+3][(4*2)/4][5];  /* Parenthesized expression in size */
char (*array_of_pointers_to_arrays[5])[10];
int (*(*array_of_func_ptrs[3])(int))[4];

/* Array with parenthesized size expression */
int sized_array[(sizeof(int) > 2) ? 10 : 20];

/* 3. Nested aggregate initializers and type definitions */
struct InnerStruct {
    char *p;
    int values[2][2];
};

struct OuterStruct {
    int a[2][3];
    struct InnerStruct inner;
    fn_ptr_simple func_ptr;
    int (*array_ptr)[4];
};

union ComplexUnion {
    struct OuterStruct os;
    int (*func_array[2])(int);
    long long big_array[2][2];
};

/* Global variables with nested initializers */
struct OuterStruct global_outer = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { 
        "test", 
        {{7, 8}, {9, 10}}
    },
    &simple_func,
    NULL
};

int nested_array[2][3][2] = { 
    { 
        {1, 2}, 
        {3, 4}, 
        {5, 6} 
    }, 
    { 
        {7, 8}, 
        {9, 10}, 
        {11, 12} 
    } 
};

/* Designated initializers with nested braces */
int designated_array[3][2] = { 
    [0] = {1, 2}, 
    [1] = {3, 4}, 
    [2] = {5, 6} 
};

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

struct { 
    int (*table[2])[3]; 
    union ComplexUnion u; 
} combined = { 
    { 
        (int[][3]){{1,2,3}, {4,5,6}}, 
        (int[][3]){{7,8,9}, {10,11,12}} 
    }, 
    { 
        .os = global_outer 
    } 
};

/* Function prototype with complex parameter */
void process_complex(int (*table[])[5], struct OuterStruct param, 
                     int (*(*callback)(int (*)(double)))(char));

/* 5. Global scope complex declarations */
int (*(*global_complex_var)(void))[3];
struct OuterStruct *global_struct_ptr = &global_outer;
int (*global_func_ptr_array[2])(int) = { NULL, NULL };

/* Anonymous struct in global scope */
struct {
    int (*(*nested_func_ptr)(int[][2]))(void);
    char (*string_array[2])[10];
} anonymous_global = { NULL, { NULL, NULL } };

/* 6. Implementation of the complex processing function */
void process_complex(int (*table[])[5], struct OuterStruct param,
                     int (*(*callback)(int (*)(double)))(char)) {
    /* Use parameters to prevent optimization */
    if (table && param.a[0][0] > 0 && callback) {
        /* Do nothing meaningful, just reference them */
        volatile int dummy = param.a[0][0];
        (void)dummy;
    }
}

/* Helper function for complex function pointer chain */
int (*complex_helper(int (*fp)(double)))(char) {
    static int (*result)(char) = &returns_int_from_char;
    if (fp) {
        int val = fp(3.14);
        (void)val;
    }
    return result;
}

int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer typedefs */
    fn_ptr_simple local_fn_ptr = &simple_func;
    result += local_fn_ptr();  /* Add 42 */
    
    /* 2. Access nested array elements */
    result += nested_array[0][0][0];  /* Add 1 */
    result += nested_array[1][2][1];  /* Add 12 */
    
    /* 3. Use global struct with nested initializer */
    result += global_outer.a[0][1];   /* Add 2 */
    result += global_outer.inner.values[1][0];  /* Add 9 */
    
    /* 4. Complex function pointer setup and call */
    returns_complex_fn_ptr = &complex_helper;
    int (*(*local_complex_ptr)(int (*)(double)))(char) = returns_complex_fn_ptr;
    
    if (local_complex_ptr) {
        int (*intermediate)(char) = local_complex_ptr(&takes_double);
        if (intermediate) {
            result += intermediate('A');  /* Add 65 (ASCII 'A') */
        }
    }
    
    /* 5. Array of pointers to arrays */
    char local_array[10] = "test";
    array_of_pointers_to_arrays[0] = &local_array;
    result += (*array_of_pointers_to_arrays[0])[0];  /* Add 't' (116) */
    
    /* 6. Use combined structure */
    result += combined.table[0][0][1];  /* Add 2 */
    
    /* 7. Designated array access */
    result += designated_array[2][0];   /* Add 5 */
    
    /* 8. Pointer to array with compound literal */
    result += ptr_to_array[1][0];       /* Add 3 */
    
    /* 9. Initialize and use global complex var */
    int local_array_3[3] = {10, 20, 30};
    int (*return_array_ptr(void))[3] = { &local_array_3 };
    global_complex_var = &return_array_ptr;
    
    if (global_complex_var) {
        int (*arr_ptr)[3] = (*global_complex_var)();
        if (arr_ptr) {
            result += (*arr_ptr)[1];  /* Add 20 */
        }
    }
    
    /* 10. Call the complex processing function */
    int (*table_arg[2])[5];
    int row1[5] = {1, 2, 3, 4, 5};
    int row2[5] = {6, 7, 8, 9, 10};
    table_arg[0] = &row1;
    table_arg[1] = &row2;
    
    process_complex(table_arg, global_outer, returns_complex_fn_ptr);
    
    /* Print final result (42+1+12+2+9+65+116+2+5+3+20 = 277) */
    printf("Result: %d\n", result);
    
    return 0;
}
