/* test_complex_types.c - Designed to exercise gengtype-parse.cc's consume_balanced() function */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
int (*returns_fn_ptr(int x))(char) { 
    static int result = 0;
    return (int (*)(char))&simple_func; 
}

/* Global variable using complex function pointer type */
int (*(*global_func_ptr)(int (*)(double)))(char) = NULL;

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];

/* Array with parenthesized size expression */
int matrix[3 * (2 + 1)][4];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { 
        "test", 
        {{7, 8}, {9, 10}}
    },
    &simple_func
};

/* Union with nested struct and array */
union ComplexUnion {
    struct {
        int (*func_array[2])(void);
        long data[3];
    } s;
    double d;
    int (*(*nested_fp)(void))[5];
};

/* Designated initializers with nested braces */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

/* Struct with anonymous struct member */
struct Container {
    int (*table[3])[5];
    struct { 
        int x; 
        char (*strings[2])[10];
    } param;
};

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct Container param);

/* 5. More global declarations */
int (*(*global_var)(void))[3];

struct Data { 
    int (*func)(int); 
    union ComplexUnion u;
} global_data = { 
    NULL, 
    { 
        .s = { 
            {NULL, NULL}, 
            {100, 200, 300}
        }
    } 
};

/* Function returning pointer to array of function pointers */
int (*(*get_func_array(void))[3])(int) {
    static int (*arr[3])(int) = {NULL, NULL, NULL};
    return &arr;
}

/* 6. Main function - uses the complex types to prevent dead code elimination */
int main(void) {
    int result = 0;
    
    /* Use global_nested */
    result += global_nested.a[0][0];
    result += global_nested.inner.values[1][1];
    
    /* Use arr with designated initializers */
    result += arr[0][1];
    result += arr[1][2];
    
    /* Use ptr_to_array (compound literal) */
    result += ptr_to_array[0][0];
    result += ptr_to_array[1][1];
    
    /* Initialize and use function pointers */
    global_func_ptr = &returns_fn_ptr;
    if (global_func_ptr) {
        /* This would normally be called, but we just check it's not NULL */
        result += 1;
    }
    
    /* Use global_data */
    result += global_data.u.s.data[0] / 100;
    
    /* Initialize array_of_func_ptrs (simplified for example) */
    for (int i = 0; i < 5; i++) {
        /* Just mark as initialized */
        result += i;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 3 * (2 + 1); i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 10 + j;
            if (i == 0 && j == 0) result += matrix[i][j];
        }
    }
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", result);
    
    /* Also use the simple_func through typedef */
    fn_simple local_fp = &simple_func;
    if (local_fp) {
        printf("Function pointer works: %d\n", local_fp());
    }
    
    return 0;
}

/* Implementation of process function */
void process(int (*table[])[5], struct Container param) {
    /* Dummy implementation */
    (void)table;
    (void)param;
}
