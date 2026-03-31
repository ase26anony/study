/* simple.h - Basic delimiter cases for gengtype parser coverage */

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

/* Enum with trailing semicolon - default case characters */
enum SimpleEnum {
    VALUE1,
    VALUE2,
    VALUE3
};
