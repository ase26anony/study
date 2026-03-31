/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int func_taking_double(double d) { return (int)d; }
int (*func_returning_fp(char c))(char) { 
    static int result = 100;
    return &simple_func; /* Simplified return */
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][(3+2)])[4][5];

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
    {{1,2,3}, {4,5,6}}, 
    { 
        "test", 
        {{7,8}, {9,10}}
    },
    &simple_func
};

/* Array with designated initializers */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combined constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Struct with anonymous struct parameter */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* 5. Global scope complex declarations */
int (*(*global_func_ptr)(void))[3];
struct Data { 
    int (*func)(int);
    int matrix[2][(1+2)];
} global_data = { 
    NULL, 
    {{1,2,3}, {4,5,6}}
};

/* Complex typedef chain */
typedef int Matrix[3][4];
typedef Matrix *MatrixPtr;
typedef MatrixPtr (*MatrixFunc)(int);

/* Union with nested struct */
union ComplexUnion {
    struct {
        int (*func_array[2])(void);
        char data[4];
    } s;
    long long as_ll;
};

/* 6. Main function to use these types and prevent dead code elimination */
int main(void) {
    /* Use global_nested */
    int sum = 0;
    sum += global_nested.a[0][0];      /* 1 */
    sum += global_nested.a[1][2];      /* 6 */
    sum += global_nested.inner.values[0][1]; /* 8 */
    
    /* Use designated_arr */
    sum += designated_arr[0][2];       /* 3 */
    sum += designated_arr[1][0];       /* 4 */
    
    /* Use ptr_to_array */
    sum += ptr_to_array[0][0];         /* 1 */
    sum += ptr_to_array[2][1];         /* 6 */
    
    /* Use global_data */
    sum += global_data.matrix[0][1];   /* 2 */
    sum += global_data.matrix[1][2];   /* 6 */
    
    /* Create and use local complex type */
    fn_complex local_complex = NULL;
    /* In real usage, we'd assign a proper function here */
    
    /* Create and use union */
    union ComplexUnion u;
    u.s.func_array[0] = &simple_func;
    u.s.data[0] = 'A';
    sum += (int)u.s.data[0];           /* 65 (ASCII 'A') */
    
    /* Total sum: 1+6+8+3+4+1+6+2+6+65 = 102 */
    printf("Result: %d\n", sum);
    
    return 0;
}

/* Additional function definitions to satisfy references */
void process(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Dummy implementation */
    (void)table;
    (void)param;
}
