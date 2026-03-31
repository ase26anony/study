/* simple.h - Basic delimiter cases for coverage */

/* Function prototype - triggers '(' case */
void simple_function(int arg);

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

/* Enum with default case identifiers */
enum SimpleEnum {
    VALUE1,    /* default case for ',' */
    VALUE2,    /* default case for identifiers */
    VALUE3 = 5 /* default case for '=' */
};
