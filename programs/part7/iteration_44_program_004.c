/* Test input for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. PARENTHESES () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *input, int length);

/* 2. BRACKETS [] - Array declarations */
int simple_array[10];
extern int multi_dim_array[5][(sizeof(int)*2)];
static const char *string_array[] = {"test1", "test2", "test3"};

/* 3. BRACES {} - Struct/union definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
    int int_val;
    float float_val;
    struct {
        int x;
        int y;
    } point;
};

/* 4. NESTED CONSTRUCTS - Combining multiple delimiter types */

/* Array of function pointers (combines [] and ()) */
int (*callback_array[5])(const char *);

/* Function pointer returning pointer to array (combines (), *, and []) */
int (*(*get_matrix_ptr(void))[10]);

/* Struct with array member and initializer (combines {} and []) */
struct Container {
    int values[3];
    char *(*processor)(int);
} GTY(()) container_instance = {
    .values = {1, 2, 3},
    .processor = NULL
};

/* 5. COMPLEX NESTED EXAMPLE */
typedef struct Node {
    struct Node * GTY((skip)) next;
    int (* GTY((tag("NODE_TYPE"))) compare)(struct Node *, struct Node *);
    union {
        int int_data;
        struct {
            char *name;
            int id;
        } GTY((desc("1"))) info;
    } data;
} Node;

/* 6. MORE GTY-MARKED DECLARATIONS */
static GTY(()) Node *(*node_factory[5])(const char *name, int id);

/* 7. ENUM WITH INITIALIZER */
enum Status {
    OK = 0,
    ERROR = 1,
    PENDING = {2}
};

/* 8. FUNCTION-LIKE MACRO WITH PARENTHESES (should be skipped) */
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* 9. POINTER TO ARRAY OF FUNCTION POINTERS */
int (*(*complex_array[2][3])(float))[5];

/* 10. FINAL COMPREHENSIVE EXAMPLE */
typedef struct {
    int (*methods[4])(void);
    struct {
        char buffer[256];
        int (*handler)(char *);
    } GTY((chain_next("%h.next"))) config;
} GTY((length("%h.method_count"))) AppContext;

AppContext GTY(()) global_context = {
    .methods = {NULL, NULL, NULL, NULL},
    .config = {
        .buffer = {0},
        .handler = NULL
    }
};
