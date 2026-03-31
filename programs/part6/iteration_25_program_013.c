/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_returning_fn_ptr)(double))(char);

/* Global function pointer using complex typedef */
fn_returning_fn_ptr global_complex_fp = NULL;

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[3])(int))[4];
char (*string_arrays[(2+3)])[20];

/* Array with parenthesized size and complex element type */
int (*complex_array[2+1])[5][6];

/* 3. Nested aggregate initializers and type definitions */
struct InnerStruct {
    char *name;
    int values[2][2];
};

struct OuterStruct {
    int id;
    struct InnerStruct inner;
    int (*compute)(struct InnerStruct *);
    int matrix[3][3];
};

/* Global struct with deeply nested initializer */
struct OuterStruct global_nested = {
    .id = 42,
    .inner = {
        .name = "test",
        .values = { {1, 2}, {3, 4} }
    },
    .compute = NULL,
    .matrix = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    }
};

/* Union with nested struct and array */
union ComplexUnion {
    struct {
        int (*func_ptr)(int);
        char data[4][4];
    } s;
    long (*array_of_ptrs[2])(void);
};

/* 4. Combine constructs in single declarations */
/* Function returning pointer to array of function pointers */
int (*(*get_func_table(void))[5])(int, int);

/* Variable with compound literal initializer */
int (*ptr_to_2d_array)[3] = (int[][3]){ 
    {1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9} 
};

/* Struct containing array of function pointers */
struct Container {
    int (*(*funcs[3])(void))[2];
    struct {
        int x;
        int y[2][2];
    } nested;
};

/* 5. More global declarations with mixed delimiters */
/* Function prototype with complex parameter */
void process_data(int (*data[][5])[3], struct OuterStruct *config);

/* Typedef with all three delimiter types */
typedef void (*(*complex_callback[2])(int[][3]))(struct InnerStruct);

/* Global variable using the complex typedef */
complex_callback global_callbacks;

/* Simple function compatible with our function pointers */
int sample_func(int x) {
    return x * 2;
}

/* Function returning pointer to array */
int (*func_returning_array_ptr(int size))[4] {
    static int arr[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    return arr;
}

/* 6. Main function to use the complex types */
int main(void) {
    int result = 0;
    
    /* Use global_nested struct */
    result += global_nested.id;
    result += global_nested.inner.values[0][0];
    result += global_nested.inner.values[1][1];
    result += global_nested.matrix[0][0];
    result += global_nested.matrix[1][1];
    result += global_nested.matrix[2][2];
    
    /* Use ptr_to_2d_array */
    result += ptr_to_2d_array[0][0];
    result += ptr_to_2d_array[1][1];
    result += ptr_to_2d_array[2][2];
    
    /* Initialize and use array_of_func_ptrs */
    for (int i = 0; i < 3; i++) {
        /* Create a simple function pointer type */
        int (*(*simple_func)(int))[4];
        /* We'll leave it NULL for now, just ensure the type is parsed */
    }
    
    /* Initialize string_arrays */
    static char strings[5][20] = {
        "Hello",
        "World",
        "Test",
        "Array",
        "Strings"
    };
    for (int i = 0; i < 5; i++) {
        string_arrays[i] = &strings[i];
    }
    
    /* Create and use a local struct with initializer */
    struct Container local_container = {
        .funcs = { NULL, NULL, NULL },
        .nested = {
            .x = 100,
            .y = { {1, 2}, {3, 4} }
        }
    };
    result += local_container.nested.x;
    result += local_container.nested.y[0][1];
    result += local_container.nested.y[1][0];
    
    /* Use complex_array */
    static int local_complex[3][5][6] = {0};
    complex_array[0] = local_complex[0];
    complex_array[1] = local_complex[1];
    complex_array[2] = local_complex[2];
    
    /* Call func_returning_array_ptr */
    int (*arr_ptr)[4] = func_returning_array_ptr(3);
    result += arr_ptr[0][0];
    result += arr_ptr[2][3];
    
    /* Assign to global_complex_fp (simplified for example) */
    /* We can't easily create a compatible function here, so we'll just note it */
    
    printf("Result: %d\n", result);
    
    /* Additional complex declaration inside main to ensure parsing */
    {
        /* Nested block with complex declaration */
        int (*(*local_var)(int (*(*)(double))[3]))(char) = NULL;
        
        /* Array with designated initializer and nested braces */
        int deep_array[2][3][2] = {
            [0] = { {1, 2}, {3, 4}, {5, 6} },
            [1] = { {7, 8}, {9, 10}, {11, 12} }
        };
        result += deep_array[0][0][0];
        result += deep_array[1][2][1];
    }
    
    printf("Final result: %d\n", result);
    return 0;
}

/* Additional global declarations after main */

/* Function with complex return type and parameters */
int (*(*register_callback(int (*(*cb)(int[][3]))(struct InnerStruct)))(int))[2] {
    static int (*(*stored_callback)(int))[2] = NULL;
    /* Complex logic would go here */
    return stored_callback;
}

/* Struct with anonymous union and struct */
struct AnonymousExample {
    union {
        struct {
            int x;
            int y;
        } point;
        int coordinates[2];
    } data;
    void (*operation)(struct AnonymousExample *);
};

/* Global instance with initializer */
struct AnonymousExample anon_example = {
    .data = {
        .point = { .x = 10, .y = 20 }
    },
    .operation = NULL
};
