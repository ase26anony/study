- `custom_op` is a **custom RISC-V instruction** (part of the custom instruction space)
- It takes **10 operands** (9 inputs, 1 output destination)

## **Operand Mapping**
The `%0` through `%9` refer to the C variables:

### **Outputs (3 registers):**
- `%0` → `out0` (result register 1)
- `%1` → `out1` (result register 2)  
- `%2` → `out2` (result register 3)

### **Inputs (8 registers):**
- `%3` → `in0`
- `%4` → `in1`
- `%5` → `in2`
- `%6` → `in3`
- `%7` → `in4`
- `%8` → `in5`
- `%9` → `in6`
- **Note:** There's a mismatch - the format string shows 10 operands but only 9 are provided in the constraints

## **Constraint Syntax**
