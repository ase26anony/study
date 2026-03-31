i_1 = 0;
goto loop_header;

loop_header:
i_2 = Φ(i_1, i_3);  // Merges initial value and increment
if (i_2 >= n) goto exit;

if (i_2 == 0) {
    // body - executes only on first iteration
}

i_3 = i_2 + 1;
goto loop_header;

exit:
// rest of program
