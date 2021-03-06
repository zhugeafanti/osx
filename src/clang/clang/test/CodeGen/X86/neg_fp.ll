; RUN: llvm-as < %s | llc -march=x86 -mattr=+sse41 -o %t -f
; RUN: grep xorps %t | count 1

; Test that when we don't -enable-unsafe-fp-math, we don't do the optimization
; -0 - (A - B) to (B - A) because A==B, -0 != 0

define float @negfp(float %a, float %b) {
entry:
	%sub = sub float %a, %b		; <float> [#uses=1]
	%neg = sub float -0.000000e+00, %sub		; <float> [#uses=1]
	ret float %neg
}