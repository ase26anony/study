/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func)(int);
typedef int (*(*func_returning_func_ptr)(double))(char);

/* Function pointer with nested parameter */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int multi_dim_array[2][3][4];
char (*array_of_strings[5])[20];
int (*(*array_of_func_ptrs[3])(int))[5];
char (*strings_with_expr[(2+3)*2])[15];

/* 3. Nested aggregate initializers and type definitions */
struct InnerStruct {
    int x;
    char *name;
    int (*callback)(int);
};

struct OuterStruct {
    int id;
    struct InnerStruct inner[2];
    int matrix[2][3];
    union {
        int ival;
        float fval;
        void (*func)(void);
    } data;
};

/* Global struct with complex initializer */
struct OuterStruct global_struct = {
    .id = 42,
    .inner = {
        { 
            .x = 1, 
            .name = "first",
            .callback = NULL
        },
        {
            .x = 2,
            .name = "second",
            .callback = NULL
        }
    },
    .matrix = {
        {1, 2, 3},
        {4, 5, 6}
    },
    .data = { .func = NULL }
};

/* Array with nested designated initializers */
int nested_array[3][2][2] = {
    [0] = { {1, 2}, {3, 4} },
    [1] = { {5, 6}, {7, 8} },
    [2] = { {9, 10}, {11, 12} }
};

/* 4. Combined constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

/* Function with complex parameter */
void (*signal_handler)(int, struct __siginfo *, void *);

/* Struct containing function pointer array */
struct ComplexContainer {
    int (*(*func_table[4])(int, int))[3];
    struct {
        char *(*get_name)(void);
        void (*set_name)(const char *);
    } ops;
};

/* 5. More global declarations with all delimiters */
/* Function returning pointer to array of function pointers */
int (*(*(*get_callback_table(void))[5])(int))[3];

/* Union with anonymous struct and array */
union MixedData {
    struct {
        int (*compare)(const void *, const void *);
        void (*free)(void *);
    } funcs;
    void *data[2];
    int (*array_of_ptrs[2])(void);
};

/* 6. Helper functions for execution */
int simple_callback(int x) {
    return x * 2;
}

int process_double(double d) {
    return (int)(d * 10);
}

int func_for_ptr(char c) {
    return (int)c;
}

/* Main function to use the complex types */
int main(void) {
    int result = 0;
    
    /* Use global_struct */
    result += global_struct.id;
    result += global_struct.inner[0].x;
    result += global_struct.matrix[1][2];  /* 6 */
    
    /* Use nested_array */
    result += nested_array[0][1][1];  /* 4 */
    result += nested_array[2][0][0];  /* 9 */
    
    /* Use ptr_to_array */
    result += ptr_to_array[1][1];  /* 4 */
    
    /* Create and use local complex type */
    int (*(*local_func_ptr)(int (*)(double)))(char) = NULL;
    
    /* Create array of complex type */
    struct ComplexContainer container = {
        .func_table = { NULL, NULL, NULL, NULL },
        .ops = { .get_name = NULL, .set_name = NULL }
    };
    
    /* Use union */
    union MixedData mixed = {
        .funcs = { .compare = NULL, .free = NULL }
    };
    
    /* Calculate final result */
    result = result % 100;  /* Keep it small */
    
    printf("Result: %d\n", result);
    printf("Test completed successfully!\n");
    
    return 0;
}
