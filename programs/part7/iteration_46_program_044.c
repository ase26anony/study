/* simple.h - Basic delimiter cases for coverage */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100

/* Function prototype - triggers '(' case */
extern void simple_function(int param);

/* Array declaration - triggers '[' case */
extern int simple_array[10];

/* Structure definition - triggers '{' case */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Union definition - another '{' case */
union SimpleUnion {
    int int_val;
    float float_val;
};

/* Enum - contains '{' */
enum SimpleEnum {
    VALUE1,
    VALUE2,
    VALUE3
};

/* Mixed default characters: * & ; , . */
static const volatile int *pointer_var = 0;
int &reference_var = 0;
int comma, separated, variables;
float floating.point = 3.14;
