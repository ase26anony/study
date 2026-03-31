But the clobber list includes 64-bit registers (`rax`, `rbx`, etc.). This is inconsistent.

### 2. **Clobber list problems**
- `"rax", "rbx", "rcx", "rdx"` - These are 64-bit register names in AT&T syntax
- The assembly instruction is 32-bit (`movl`), so it shouldn't clobber entire 64-bit registers
- If you're compiling for x86-64, you should use the 64-bit register names properly

### 3. **Better alternatives**

**Option A: Fix for 32-bit operation:**
