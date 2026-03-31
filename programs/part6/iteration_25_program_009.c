/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function pointers with nested parentheses */
int (*func_ptr_simple)(void);
int (*(*complex_func_ptr)(int (*)(double)))(char);
int (*(*(*nested_func_ptr)(void))(int))(float);

/* Typedef chain building complex types */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef fn2 (*fn3)(char*);
fn3 global_fn_chain;

/* 2. Multi-dimensional and complex array declarations */
int multi_dim_array[2][3][4];
char (*array_of_pointers[5])[10];
int (*(*array_of_func_ptrs[3])(int))[5];
char (*strings[(2+3)])[20];
int (*matrix_ptr)[(sizeof(int) > 2) ? 4 : 2];

/* Array with parenthesized size expression */
int sized_array[((2*3)+1)];

/* 3. Nested structures with initializers */
struct Inner {
    int values[2][2];
    char *name;
};

struct Middle {
    struct Inner inner;
    float (*calc)(int, int);
    int flags[3];
};

struct Outer {
    struct Middle mid[2];
    int (*processor)(struct Inner*);
    long counter;
};

/* Global struct with deeply nested initializer */
struct Outer global_struct = {
    .mid = {
        {
            .inner = {
                .values = {{1, 2}, {3, 4}},
                .name = "first"
            },
            .calc = NULL,
            .flags = {1, 0, 1}
        },
        {
            .inner = {
                .values = {{5, 6}, {7, 8}},
                .name = "second"
            },
            .calc = NULL,
            .flags = {0, 1, 0}
        }
    },
    .processor = NULL,
    .counter = 42
};

/* Union with nested array */
union DataUnion {
    int (*func_array[2])(void);
    struct {
        char data[4][3];
        int tag;
    } structured;
};

/* 4. Combined constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

struct Config {
    int (*handlers[3])(struct Inner*);
    char (*names[])[20];
} config_var = {
    .handlers = {NULL, NULL, NULL},
    .names = (char[][20]){"test1", "test2", "test3"}
};

/* Function with complex parameter */
void process_table(int (*table[])[5], struct { int x; double y; } param);

/* 5. Additional global declarations */
int (*(*global_complex)(void))[3];
struct { 
    int (*func)(int); 
    char data[2][(2+1)]; 
} global_data = { 
    .func = NULL, 
    .data = {{'a', 'b', 'c'}, {'d', 'e', 'f'}}
};

/* Array with nested designated initializers */
int nested_init_array[2][3] = { 
    [0] = {1, 2, 3}, 
    [1] = {4, 5, 6} 
};

/* Compound literal in global scope */
int *global_compound = (int[]){10, 20, 30, 40};

/* Helper functions for function pointers */
int simple_func(void) { return 1; }
int param_func(double d) { return (int)d; }
char return_char_func(int x) { return (char)(x + 'A'); }
float float_func(int x) { return (float)x * 1.5f; }

int handler_func(struct Inner *inner) { 
    return inner ? inner->values[0][0] : 0; 
}

/* Main function to use the complex types */
int main(void) {
    int result = 0;
    
    /* 1. Use function pointers */
    func_ptr_simple = simple_func;
    result += func_ptr_simple();
    
    /* Can't easily assign complex_func_ptr without compatible function,
       but we can assign NULL to show it's referenced */
    complex_func_ptr = NULL;
    nested_func_ptr = NULL;
    
    /* 2. Access nested array elements */
    result += global_struct.mid[0].inner.values[0][0];
    result += global_struct.mid[1].inner.values[1][1];
    result += global_struct.counter;
    
    /* 3. Access array with initializer */
    result += nested_init_array[0][1];
    result += nested_init_array[1][2];
    
    /* 4. Use compound literal array */
    result += global_compound[0];
    result += global_compound[2];
    
    /* 5. Access global_data */
    result += global_data.data[0][1] - 'a';
    result += global_data.data[1][2] - 'a';
    
    /* 6. Use combined_ptr */
    result += combined_ptr[0][0];
    result += combined_ptr[2][1];
    
    /* 7. Access sized_array */
    for (int i = 0; i < ((2*3)+1); i++) {
        sized_array[i] = i * 2;
        result += sized_array[i] % 5;
    }
    
    /* 8. Multi-dimensional array access */
    multi_dim_array[0][1][2] = 7;
    result += multi_dim_array[0][1][2];
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}

/* Function definition for the prototype */
void process_table(int (*table[])[5], struct { int x; double y; } param) {
    if (table && param.x > 0) {
        /* Access the table */
        int value = (*table)[0][0];
        printf("Processing: %d\n", value + param.x);
    }
}

/* Additional complex type not used in main but present for parsing */
union {
    struct {
        int (*(*nested)[3])(void);
        char data[2][2];
    } s;
    long long raw;
} unused_union = {
    .s = {
        .nested = NULL,
        .data = {{'x', 'y'}, {'z', 'w'}}
    }
};
