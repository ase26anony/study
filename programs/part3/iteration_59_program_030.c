   - Compares the high parts as **unsigned integers**
   - This handles the sign properly for signed comparison because:
     - Negative numbers have their high bit set when interpreted as unsigned
     - In two's complement, all negative numbers appear larger than positive numbers when interpreted as unsigned
     - This gives the correct ordering for signed comparison

3. **Low part comparison**:
