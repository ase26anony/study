/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
typedef unsigned long int size_t;
extern volatile int global_counter;
static const double PI = 3.14159;

/* Case '(': Function prototypes */
void simple_function(int param);
char *allocate_memory(size_t size);
int calculate(int x, int y, int z);

/* Case '[': Array declarations */
int numbers[100];
extern char buffer[256];
float matrix[10][20];

/* Case '{': Structure definitions */
struct Point {
    int x;
    int y;
};

union Data {
    int i;
    float f;
    char str[20];
};

enum Color { RED, GREEN, BLUE };
