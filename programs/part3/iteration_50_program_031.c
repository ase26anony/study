**Key features that target the uncovered lines:**

1. **Multiple pointer variables** (`p`, `q`, `r`, `pc`, `cont_ptr`, `mix_ptr`) with different types create opportunities for `[reg + 0]` addressing.

2. **Post-increment/decrement operations** in loops: `p++`, `q++`, `cont_ptr++`, `mix_ptr++`, `r--`.

3. **Structure field accesses** like `cont_ptr->id` (offset 0) and `mix_ptr->c` (offset 0) directly correspond to the `mem_insn.mem_loc = address_of_x` pattern.

4. **Array indexing patterns**: `arr_c[i]`, `*(r + inner)`, `base_s + idx` provide various addressing modes.

5. **Compiler barriers** with `sin()`/`cos()` calls force re-evaluation of addressing modes.

6. **`__restrict` qualifiers** and `register` hints influence alias analysis and register allocation.

7. **Mixed data types** with different alignments stress the offset calculation logic.

8. **Nested loops** and helper function create multiple optimization contexts.

**Recommended compilation commands:**
