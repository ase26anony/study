i₀ = 0
goto loop_header

loop_header:
i₁ = Φ(i₀, i₂)  // Phi node: i₀ on first entry, i₂ on loop back edge
if (i₁ ≥ n) goto exit

if (i₁ == 0) {
    // This only executes in the first iteration
}

i₂ = i₁ + 1
goto loop_header

exit:
