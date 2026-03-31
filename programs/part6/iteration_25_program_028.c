/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*FuncPtr1)(void);
typedef FuncPtr1 (*FuncPtr2)(int);
typedef int (*(*FuncPtr3)(FuncPtr2))(char, double);

/* Function to be pointed to by complex function pointers */
int simple_func(void) { return 42; }
FuncPtr1 get_func_ptr(int x) { return simple_func; }
int (*receive_func_ptr(FuncPtr2 fp))(char, double) {
    static int result = 100;
    return (int (*)(char, double))(&result);
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[3])(int))[4];
char (*strings[(2+3)])[20];
int (*multi_array[2][(1+2)])[3][4];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func)(int);
};

struct Middle {
    int data[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } value;
};

struct Outer {
    struct Middle mid;
    int (*callback)(struct Inner);
    struct {
        unsigned flags : 4;
        unsigned count : 12;
    } bits;
};

/* Complex union with nested struct */
union Container {
    struct {
        int (*array_ptrs[2])(void);
        struct Inner nested;
    } s;
    void (*func_ptr)(int, ...);
    long long big_num;
};

/* 4. Combine constructs in single declarations */
int (*complex_global)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };
struct Outer (*get_outer_ptr(void))[2] {
    static struct Outer arr[2];
    return &arr;
}

/* Function with complex parameter */
void process_data(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Do nothing, just for declaration */
}

/* 5. Global scope declarations with initializers */
int (*(*global_func_ptr)(void))[3] = NULL;

struct Data {
    int (*func)(int);
    struct Inner inner;
    int matrix[2][(1+1)];
} global_data = { 
    NULL, 
    { (char[]){'t','e','s','t','\0'}, NULL },
    { {1,2}, {3,4} }
};

/* Initialize array with nested braces */
int nested_arr[2][3][2] = { 
    [0] = { {1,2}, {3,4}, {5,6} }, 
    [1] = { {7,8}, {9,10}, {11,12} } 
};

/* Complex static initialization */
static struct Outer static_outer = {
    .mid = {
        .data = { {10,20,30}, {40,50,60} },
        .inner = { "static", NULL },
        .value = { .d = 3.14159 }
    },
    .callback = NULL,
    .bits = { 3, 255 }
};

/* 6. Main function to use the types and prevent dead code elimination */
int main(void) {
    /* Local variable using complex typedef */
    FuncPtr3 local_complex = receive_func_ptr;
    
    /* Assign to global function pointer */
    global_func_ptr = (int (*(*)(void))[3])get_outer_ptr;
    
    /* Access nested array elements */
    int sum = 0;
    sum += nested_arr[0][0][0];  /* 1 */
    sum += nested_arr[1][2][1];  /* 12 */
    sum += global_data.matrix[0][1];  /* 2 */
    sum += static_outer.mid.data[0][2];  /* 30 */
    
    /* Use array of function pointers */
    array_of_func_ptrs[0] = (int (*(*)(int))[4])0x1234;
    
    /* Access complex global */
    if (complex_global) {
        sum += (*complex_global)[0];  /* 1 */
    }
    
    /* Create and use a compound literal with all delimiters */
    struct Outer temp = {
        .mid = {
            .data = { {100,200}, {300,400} },
            .inner = { "temp", (int (*)(int))&sum },
            .value = { .l = 999 }
        },
        .callback = (int (*)(struct Inner))&sum,
        .bits = { 1, 100 }
    };
    
    sum += temp.mid.data[0][0];  /* 100 */
    
    /* Function pointer assignment */
    FuncPtr2 fp2 = get_func_ptr;
    if (fp2) {
        FuncPtr1 fp1 = fp2(5);
        if (fp1) {
            sum += fp1();  /* 42 */
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);  /* 1+12+2+30+1+100+42 = 188 */
    
    /* Additional complex local declaration mixing all delimiters */
    void (*local_mixed)(int, struct { int a[2]; }*) = 
        (void (*)(int, struct { int a[2]; }*))printf;
    
    return 0;
}

/* Additional global to ensure more parsing */
union Container global_container = {
    .s = {
        .array_ptrs = { simple_func, simple_func },
        .nested = { "global", NULL }
    }
};

/* Complex function prototype at global scope */
int (*(*final_global)(int (*)(double), char *argv[]))[10] = NULL;
