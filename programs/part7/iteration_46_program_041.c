/* simple.h - Basic delimiter cases for coverage */

/* Default case triggers: keywords, identifiers, operators */
typedef unsigned long size_t;
extern volatile int global_counter;
static const double PI = 3.14159;

/* '(' case: function prototypes */
void simple_func(int param);
char *allocate_memory(size_t bytes);
int calculate(int x, int y, int z);

/* '[' case: array declarations */
int numbers[100];
extern char buffer[256];
float matrix[10][20];

/* '{' case: structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed: struct with array member */
struct Collection {
    int ids[50];
    struct Point points[100];
};
