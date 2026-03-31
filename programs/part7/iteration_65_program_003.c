```cpp
// test_parser_errors.cc
// Compile with: g++ -std=c++11 -fsyntax-only -fdiagnostics-parseable-fixits test_parser_errors.cc
// Or with: g++ -std=c++11 -O1 -fno-exceptions test_parser_errors.cc
// Or with: g++ -std=c++11 -O0 -fpermissive -Wno-error test_parser_errors.cc

#include <iostream>

// Valid code to establish normal parsing state
void valid_function() {
    int x = 42;
    std::cout << "Valid code" << std::endl;
}

// 1. RT_EXTERN: Expected 'extern' in linkage specification
void test_extern_error() {
    // Valid preceding code
    int a = 1;
    char b = 'c';
    
    // Missing 'extern' before linkage specifier
    "C" {
        void some_function();
    }
}

// 2. RT_STATIC_ASSERT: Expected 'static_assert'
void test_static_assert_error() {
    // Valid preceding code
    double d = 3.14;
    float f = 2.71f;
    
    // Missing 'static_assert' keyword
    (sizeof(int) == 4, "int must be 4 bytes");
}

// 3. RT_DECLTYPE: Expected 'decltype' in trailing return type
void test_decltype_error() {
    // Valid preceding code
    auto x = 5;
    auto y = 6.0;
    
    // Missing 'decltype' - ambiguous context
    auto foo() -> (x);
}

// 4. RT_OPERATOR: Expected 'operator' in conversion operator
class TestClass {
public:
    int value;
    
    // Valid member function
    void normal_func() { value = 0; }
    
    // Missing 'operator' keyword for conversion operator
    int() const { return value; }
};

// 5. RT_CLASS: Expected 'class' in class definition
void test_class_error() {
    // Valid code
    struct S { int x; };
    
    // Missing 'class' keyword
    MyClass {
    public:
        int x;
        void f() {}
    };
}

// 6. RT_TEMPLATE: Expected 'template' keyword
void test_template_error() {
    // Valid template usage
    template<typename T>
    void good_func(T t) {}
    
    // Missing 'template' keyword - could be parsed as comparison
    <typename T> void bad_func(T t) {}
}

// 7. RT_NAMESPACE: Expected 'namespace' keyword
void test_namespace_error() {
    // Valid namespace
    namespace Valid { int x = 1; }
    
    // Missing 'namespace' keyword
    MyNamespace {
        int y = 2;
    };
}

// 8. RT_USING: Expected 'using' keyword
void test_using_error() {
    // Valid using declarations
    using std::cout;
    using namespace std;
    
    // Missing 'using' keyword for directive
    namespace std;
    
    // Missing 'using' keyword for declaration
    std::cout;
}

// 9. RT_ASM: Expected 'asm' keyword
void test_asm_error() {
    // Valid code
    int i = 0;
    i++;
    
    // Missing 'asm' keyword for inline assembly
    volatile ("nop");
}

// 10. RT_TRY: Expected 'try' keyword
void test_try_error() {
    // Valid code
    bool flag = true;
    
    // Missing 'try' keyword before block
    {
        throw 42;
    } catch (...) {
        // Handle exception
    }
}

// Additional valid code between error cases
namespace IntermediateValidCode {
    int global_var = 100;
    
    template<typename T>
    T add(T a, T b) { return a + b; }
}

// More error cases in different contexts
class AnotherTest {
    // RT_OPERATOR in another context
    bool() { return true; }  // Missing 'operator' keyword
    
    // Valid member
    int data;
};

// Test decltype in different context
using MyAlias = (int);  // Missing 'decltype' keyword

// Test template in class context
class TemplateTest {
    // Missing 'template' in member template
    <typename U> void member_func(U u) {}
};

// Main function - valid code to end the file
int main() {
    valid_function();
    std::cout << "Compilation should produce errors for each case" << std::endl;
    return 0;
}
```
