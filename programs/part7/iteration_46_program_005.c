/* simple.h - Basic delimiter patterns to trigger all switch cases */

/* Default case triggers: keywords, identifiers, operators */
typedef unsigned long int size_t;
extern volatile int global_counter;
static const double PI = 3.14159;

/* '(' case: Function prototypes */
void simple_function(int param);
char* allocate_memory(size_t bytes);
int calculate(int x, int y, int z);

/* '[' case: Array declarations */
int numbers[100];
extern char buffer[256];
float matrix[10][20];

/* '{' case: Structure definitions */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed: All delimiters in one */
struct Container {
    void (*callback)(int event);
    int items[50];
    struct Point points[10];
};
