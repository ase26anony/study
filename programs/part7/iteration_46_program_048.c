/* simple.h - Basic delimiter coverage */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
typedef unsigned long size_t;

/* Case '(': Function prototypes */
extern void simple_func(int arg);
static int calculate(int a, int b);
const char* get_message(void);

/* Case '[': Array declarations */
int global_array[100];
extern char buffer[256];
volatile double matrix[10][20];

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

/* Mixed default characters */
enum Color { RED, GREEN, BLUE };
int counter = 0;
float* pointer;
