// main.cpp - C++ part targeting explicit, mutable, location, segment attributes
#include <iostream>
#include <thread>
#include <cstring>

// For section attribute
#define SECTION_ATTR __attribute__((section(".mysection")))

// Classes with explicit constructors
class ExplicitClass {
    int value;
public:
    explicit ExplicitClass(int v) : value(v) {}
    int get() const { return value; }
};

// Class with mutable member
class MutableClass {
    mutable int counter;
    int value;
public:
    MutableClass(int v) : counter(0), value(v) {}
    int get() const { 
        ++counter;  // mutable can be modified in const method
        return value; 
    }
};

// Template class to force full debug info
template<typename T>
class TemplateClass {
    T data;
public:
    explicit TemplateClass(const T& d) : data(d) {}
    T get() const { return data; }
};

// Thread-local variable
thread_local int tl_var = 42;

// Variable in custom section
int SECTION_ATTR section_var = 123;

// Prototyped function (DW_AT_prototyped)
extern "C" void fortran_subroutine(char* str, int* len);
extern "C" void ada_procedure();
extern "C" void cobol_program();

// Function with register variable (may trigger location lists)
void use_register_var() {
    register int reg_var asm("ebx") = 42;
    asm volatile("" : : "r"(reg_var)); // Use in inline asm
}

// Complex inheritance
class Base {
protected:
    int base_data;
public:
    explicit Base(int d) : base_data(d) {}
    virtual ~Base() {}
};

class Derived : public Base {
    mutable int derived_mutable;
public:
    explicit Derived(int d) : Base(d), derived_mutable(0) {}
    void modify() const { ++derived_mutable; }
};

int main() {
    // Use explicit constructor
    ExplicitClass e1(42);
    ExplicitClass e2 = ExplicitClass(100); // Explicit construction
    
    // Use mutable class
    MutableClass m(99);
    const MutableClass& cm = m;
    cm.get(); // Calls mutable method
    
    // Use template class
    TemplateClass<double> tc(3.14159);
    
    // Use thread-local
    tl_var = 100;
    
    // Use section variable
    section_var = 456;
    
    // Use inheritance
    Derived d(777);
    d.modify();
    
    // Call Fortran
    char str[] = "Hello Fortran";
    int len = strlen(str);
    fortran_subroutine(str, &len);
    
    // Call Ada if available
    // ada_procedure();
    
    // Call COBOL if available  
    // cobol_program();
    
    // Use register variable function
    use_register_var();
    
    // Prevent dead code elimination
    volatile int sink = 0;
    sink += e1.get();
    sink += m.get();
    sink += tc.get();
    sink += tl_var;
    sink += section_var;
    sink += d.get();
    
    std::cout << "Test completed (sink=" << sink << ")\n";
    return 0;
}
