/* test_complex_types.c - Targeting gengtype-parse.cc uncovered lines */

/* 1. Complex function declarators with nested parentheses */
typedef int (*func_ptr_t)(int);
typedef func_ptr_t (*meta_func_ptr_t)(func_ptr_t, int);
int (*(*global_func_ptr1)(int (*)(double)))(char);
int (*(*(*global_func_ptr2)(void))[3])(int, int);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[2][3])(float);
int (*(*nested_array[2])[3])[4];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*callback)(void);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } u;
};

struct Complex {
    struct Nested n;
    int (*(*func_array[2])(struct Inner))[5];
};

/* Global initializers with nested braces */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { (char*)0x1000, NULL },
    { .y = 3.14 }
};

int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combined constructs in single declarations */
int (*global_ptr)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct { 
    int x; 
    int (*process)(int (*)[2]);
} global_param = { 42, NULL };

/* Function prototypes with complex parameters */
void process_table(int (*table[])[5], struct { int x; } param);
int (*(*register_callback(int (*(*cb)(int[]))[3]))(void))(float);

/* 5. Simple function to assign to function pointers */
int simple_func(int x) { return x * 2; }
int (*get_simple_func(void))(int) { return simple_func; }

/* 6. More complex nested type with all delimiters */
typedef union {
    struct {
        int (*(*funcs[2])(int))[3];
        char data[4][5];
    } s;
    void (*actions[3])(struct Nested*);
} SuperComplex;

SuperComplex global_complex = {
    .s = {
        .funcs = { NULL, NULL },
        .data = { {'a','b','c','d','e'}, {'f','g','h','i','j'} }
    }
};

/* Main function to use the complex types */
int main(void) {
    /* Use global_nested */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* Use arr */
    sum += arr[0][0] + arr[1][2];
    
    /* Use global_ptr */
    sum += global_ptr[0][0] + global_ptr[1][1];
    
    /* Assign to function pointer */
    func_ptr_t local_fp = simple_func;
    sum += local_fp(10);
    
    /* Use global_param */
    sum += global_param.x;
    
    /* Use global_complex */
    sum += (int)global_complex.s.data[0][0];
    
    printf("Result: %d\n", sum);
    
    /* Additional complex local declaration */
    int (*(*local_complex)(int (*(*)(double))[3]))(char) = NULL;
    
    /* Array with parenthesized size */
    int (*dynamic[(sum % 5) + 1])(void);
    
    /* Nested initializer in local scope */
    struct {
        int a[2][2];
        struct { char c; } inner;
    } local_struct = { {{7,8},{9,10}}, {'X'} };
    
    sum += local_struct.a[0][0] + local_struct.a[1][1];
    
    printf("Final result: %d\n", sum);
    
    return 0;
}

/* Function definitions */
void process_table(int (*table[])[5], struct { int x; } param) {
    /* Empty implementation - just for declaration */
    (void)table;
    (void)param;
}

int (*(*register_callback(int (*(*cb)(int[]))[3]))(void))(float) {
    /* Return NULL for simplicity */
    (void)cb;
    return NULL;
}
