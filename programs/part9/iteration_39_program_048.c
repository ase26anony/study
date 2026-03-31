i₀ = 0
goto loop_header

loop_header:
i₁ = Φ(i₀, i₂)  // Phi node merges initial value and increment
if (i₁ ≥ n) goto exit
if (i₁ == 0) goto then_body else goto after_if

then_body:
// body executes only on first iteration

after_if:
i₂ = i₁ + 1
goto loop_header

exit:
// rest of program
