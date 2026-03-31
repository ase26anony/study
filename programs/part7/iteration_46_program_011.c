/* simple.h - Basic delimiter cases to trigger '(' '[' '{' cases */

/* Function prototype - triggers '(' case */
void simple_function(int arg);

/* Array declaration - triggers '[' case */
extern int simple_array[10];

/* Structure definition - triggers '{' case */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Union definition - also triggers '{' case */
union SimpleUnion {
    int as_int;
    float as_float;
};

/* Enum with trailing semicolon */
enum SimpleEnum {
    VALUE1,
    VALUE2,
    VALUE3
};
