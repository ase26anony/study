#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct UndefinedType1;
class UndefinedType2;
template<typename T> class UndefinedTemplate;

// TYPE_UNDEFINED: Incomplete types with pointers
UndefinedType1* global_undefined_ptr1;
UndefinedType2* global_undefined_ptr2;
UndefinedTemplate<int>* global_undefined_template_ptr;

// TYPE_SCALAR: Fundamental types in various contexts
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 2000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14f;
double global_double = 2.71828;
long double global_ldouble = 1.41421356L;
bool global_bool = true;
wchar_t global_wchar = L'Ω';
char16_t global_char16 = u'字';
char32_t global_char32 = U'🍕';

// TYPE_STRING: String literals and character arrays
const char* global_cstr = "Hello, World!";
const wchar_t* global_wstr = L"Wide String";
const char16_t* global_u16str = u"UTF-16 String";
const char32_t* global_u32str = U"UTF-32 String";
char global_char_array[100] = "Character Array";
wchar_t global_wchar_array[50] = L"Wide Array";
const char* const global_const_ptr_to_const = "Immutable";

// TYPE_STRUCT: C structures with various compositions
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

// Global instances
SimpleStruct global_struct = {10, 3.14, "Test"};
NestedStruct global_nested = {{5, 2.718, "Inner"}, 100};
BitFieldStruct global_bitfield = {1, 5, 9, -512};
PackedStruct global_packed = {'X', 42, 3.14159};

// TYPE_USER_STRUCT: C++ classes with various features
class BaseClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { private_data++; }
    int get_private() const { return private_data; }
};

class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    virtual void virtual_func() override { derived_data = 3.14; }
};

class MultipleBase1 {
public:
    virtual void base1_func() = 0;
};

class MultipleBase2 {
public:
    virtual void base2_func() = 0;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    void base1_func() override {}
    void base2_func() override {}
};

// Template classes
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(const T& val) : data(val) {}
    T get() const { return data; }
    void set(const T& val) { data = val; }
};

// Explicit template instantiations
TemplateClass<int> global_template_int(42);
TemplateClass<double> global_template_double(3.14);
TemplateClass<SimpleStruct> global_template_struct{{1, 2.0, "Template"}};

// TYPE_UNION: Various unions
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union ComplexUnion {
    long long ll_value;
    double dbl_value;
    void* ptr_value;
    SimpleStruct struct_value;
};

struct StructWithAnonymousUnion {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    };
};

// Global union instances
SimpleUnion global_union = {.as_int = 0x41424344};
ComplexUnion global_complex_union = {.ll_value = 0xDEADBEEF};
StructWithAnonymousUnion global_anon_union_struct = {1, {.int_value = 42}};

// TYPE_POINTER: Pointers to everything
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
volatile int* global_volatile_int_ptr = &global_int;
int* restrict global_restrict_int_ptr = &global_int;
int** global_double_ptr = &global_int_ptr;
SimpleStruct* global_struct_ptr = &global_struct;
BaseClass* global_class_ptr = new DerivedClass();
void* global_void_ptr = static_cast<void*>(global_int_ptr);
const void* global_const_void_ptr = static_cast<const void*>(global_cstr);

// Function pointers
typedef int (*SimpleFuncPtr)(int, int);
typedef void (*ComplexFuncPtr)(const char*, int, ...);
typedef int (BaseClass::*MemberFuncPtr)() const;
typedef TemplateClass<int>* (*TemplateFuncPtr)(int);

// TYPE_ARRAY: Arrays of various types
int global_int_array[10] = {0,1,2,3,4,5,6,7,8,9};
int global_multi_array[2][3][4] = {
    {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
    {{13,14,15,16}, {17,18,19,20}, {21,22,23,24}}
};
SimpleStruct global_struct_array[5];
int* global_pointer_array[20];
const char* global_string_array[] = {"One", "Two", "Three", nullptr};

// Variable-length array simulation
struct VLAContainer {
    int size;
    int data[1];  // Simulated VLA
};

// TYPE_CALLBACK: Function pointers and callbacks
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

SimpleFuncPtr global_func_ptr = &add;
ComplexFuncPtr global_vararg_func_ptr = nullptr;

struct CallbackContainer {
    SimpleFuncPtr callback;
    void* user_data;
};

CallbackContainer global_callback = {&multiply, nullptr};

// Lambda expressions as callbacks
auto lambda_callback = [](int x, int y) -> int {
    return x * y + x + y;
};

template<typename Func>
int apply_callback(Func f, int a, int b) {
    return f(a, b);
}

// TYPE_LANG_STRUCT: C++ language-specific constructs
// Lambda with different captures
auto lambda_with_capture = [global_int, &global_double](int x) -> double {
    return x * global_int + global_double;
};

// Initializer list usage
auto process_initializer_list(initializer_list<int> list) -> int {
    int sum = 0;
    for (auto val : list) sum += val;
    return sum;
}

// Structured bindings (C++17)
auto get_tuple() -> tuple<int, double, string> {
    return {42, 3.14, "Test"};
}

// Fold expression (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#ifdef __cpp_coroutines
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleCoroutine dummy_coroutine() {
    co_return;
}
#endif

// Main function with argc/argv usage to prevent dead code elimination
int main(int argc, char* argv[]) {
    // Force usage of all types based on argc
    int result = 0;
    
    // TYPE_UNDEFINED usage
    if (argc > 1) {
        // Use undefined type pointers
        result += reinterpret_cast<uintptr_t>(global_undefined_ptr1) % 256;
        result += reinterpret_cast<uintptr_t>(global_undefined_template_ptr) % 256;
    }
    
    // TYPE_SCALAR usage
    result += global_int + global_uint + global_char + global_schar;
    result += static_cast<int>(global_float + global_double + global_ldouble);
    result += global_bool ? 1 : 0;
    
    // TYPE_STRING usage
    if (argc > 2) {
        result += global_cstr[0];
        result += global_wstr[0];
        result += global_char_array[0];
    }
    
    // TYPE_STRUCT usage
    result += global_struct.x + static_cast<int>(global_struct.y);
    result += global_nested.outer + global_nested.inner.x;
    result += global_bitfield.flag1 + global_bitfield.flag2 + global_bitfield.flag3;
    result += global_packed.a + global_packed.b + static_cast<int>(global_packed.c);
    
    // TYPE_USER_STRUCT usage
    BaseClass* base_ptr = new DerivedClass();
    base_ptr->virtual_func();
    result += base_ptr->get_private();
    
    MultipleDerived md;
    md.base1_func();
    md.base2_func();
    
    result += global_template_int.get();
    result += static_cast<int>(global_template_double.get());
    
    // TYPE_UNION usage
    global_union.as_int = argc;
    result += global_union.as_int;
    
    global_complex_union.dbl_value = 3.14159 * argc;
    result += static_cast<int>(global_complex_union.dbl_value);
    
    global_anon_union_struct.type = argc % 3;
    if (global_anon_union_struct.type == 0) {
        global_anon_union_struct.int_value = argc;
        result += global_anon_union_struct.int_value;
    }
    
    // TYPE_POINTER usage
    result += *global_int_ptr;
    result += **global_double_ptr;
    result += global_struct_ptr->x;
    
    // TYPE_ARRAY usage
    for (int i = 0; i < 10 && i < argc; i++) {
        result += global_int_array[i];
    }
    
    result += global_multi_array[0][0][0] + global_multi_array[1][2][3];
    
    // TYPE_CALLBACK usage
    result += global_func_ptr(10, 20);
    result += apply_callback(lambda_callback, 5, 6);
    result += global_callback.callback(3, 4);
    
    // TYPE_LANG_STRUCT usage
    result += static_cast<int>(lambda_with_capture(argc));
    result += process_initializer_list({1, 2, 3, 4, 5});
    
    auto [ival, dval, sval] = get_tuple();
    result += ival + static_cast<int>(dval) + sval.length();
    
    result += sum_all(1, 2, 3, 4, 5);
    
    #ifdef __cpp_coroutines
    dummy_coroutine();
    #endif
    
    // Prevent optimization
    volatile int anti_opt = result;
    
    // Output for observability
    cout << "Result hash: " << (result * 2654435761U) << endl;
    cout << "Address samples: " << &global_int << " " << &global_struct << " " 
         << &global_union << " " << &global_template_int << endl;
    
    delete base_ptr;
    delete global_class_ptr;
    
    return result % 256;
}

#else
// C version (simplified)
#include <stdio.h>
#include <stddef.h>

// Forward declaration for TYPE_UNDEFINED
struct UndefinedType;

// TYPE_UNDEFINED
struct UndefinedType* global_undefined_ptr;

// TYPE_SCALAR
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
float global_float = 3.14f;
double global_double = 2.71828;

// TYPE_STRING
const char* global_cstr = "Hello, World!";
char global_char_array[100] = "Character Array";

// TYPE_STRUCT
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

// Global instances
struct SimpleStruct global_struct = {10, 3.14, "Test"};
struct PackedStruct global_packed = {'X', 42, 3.14159};

// TYPE_UNION
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union SimpleUnion global_union = {.as_int = 0x41424344};

// TYPE_POINTER
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
int** global_double_ptr = &global_int_ptr;
struct SimpleStruct* global_struct_ptr = &global_struct;
void* global_void_ptr = (void*)global_int_ptr;

// Function pointers
typedef int (*SimpleFuncPtr)(int, int);
int add(int a, int b) { return a + b; }
SimpleFuncPtr global_func_ptr = &add;

// TYPE_ARRAY
int global_int_array[10] = {0,1,2,3,4,5,6,7,8,9};
int global_multi_array[2][3][4];
struct SimpleStruct global_struct_array[5];

// TYPE_CALLBACK
struct CallbackContainer {
    SimpleFuncPtr callback;
    void* user_data;
};

struct CallbackContainer global_callback = {&add, NULL};

int main(int argc, char* argv[]) {
    int result = 0;
    
    // Use all types based on argc
    if (argc > 1) {
        result += (int)(ptrdiff_t)global_undefined_ptr;
    }
    
    result += global_int + global_uint + global_char;
    result += (int)(global_float + global_double);
    
    if (argc > 2) {
        result += global_cstr[0] + global_char_array[0];
    }
    
    result += global_struct.x + (int)global_struct.y;
    result += global_packed.a + global_packed.b + (int)global_packed.c;
    
    global_union.as_int = argc;
    result += global_union.as_int;
    
    result += *global_int_ptr;
    result += **global_double_ptr;
    result += global_struct_ptr->x;
    
    for (int i = 0; i < 10 && i < argc; i++) {
        result += global_int_array[i];
    }
    
    result += global_func_ptr(10, 20);
    result += global_callback.callback(3, 4);
    
    // Prevent optimization
    volatile int anti_opt = result;
    
    printf("Result hash: %u\n", (unsigned)(result * 2654435761U));
    printf("Address samples: %p %p %p\n", 
           (void*)&global_int, (void*)&global_struct, (void*)&global_union);
    
    return result % 256;
}
#endif
