/* Test input for gengtype parser coverage - targeting delimiter handling */

/* 1. Parentheses () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal_handler(int sig, void (*handler)(int)))(int);

/* 2. Brackets [] - Array declarations */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
static const char* string_array[] = {"test", "array"};

/* 3. Braces {} - Aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union MixedUnion {
    int i;
    float f;
    struct { int x; } nested;
};

enum Color { RED, GREEN, BLUE };

/* 4. Nested combinations - Array of function pointers */
int (*callback_array[5])(const char*);
void (*(*func_ptr_array[3])(int))[10];

/* 5. Complex nested - Function returning pointer to array */
int (*(*get_matrix_ptr(void))[10][20]);

/* 6. Struct with initialized array member */
struct DataContainer {
    int values[(2+3)];
    float matrix[2][2];
} global_data = { 
    .values = {1, 2, 3, 4, 5},
    .matrix = {{1.0, 2.0}, {3.0, 4.0}}
};

/* 7. GTY-marked declarations (if gengtype recognizes these) */
typedef struct GTY(()) GtyNode {
    struct GtyNode *GTY((skip)) next;
    int data[4];
} GtyNode;

/* 8. More complex nesting */
typedef union {
    struct {
        int (*compare)(const void*, const void*);
        void (*free)(void*);
    } ops;
    void* pointers[2];
} UtilityUnion;

/* 9. Function pointer with array parameter */
void (*sorter)(int arr[], int size);

/* 10. Typedef with all delimiters combined */
typedef struct {
    int (*methods[3])(int, char*);
    struct {
        float coords[3];
        void (*transform)(float[3]);
    } geometry;
} ObjectType;

/* 11. Declaration with parentheses in array size */
char buffer[ (sizeof(int) > 4) ? 128 : 64 ];

/* 12. Multiple levels of nesting */
int (*(*(*deep_nested)(int))[5])(void);

/* 13. Initializer with nested braces */
static struct {
    int a;
    struct {
        int b[2];
        char c;
    } inner;
} compound = {1, {{2, 3}, 'x'}};

/* 14. Function-like macro style (might be parsed) */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
int sized_array[MAX(2,3)];

/* 15. Final test - everything combined */
typedef int (*(*UltimateType[2])(struct {int x; int y[2];}))[10];
