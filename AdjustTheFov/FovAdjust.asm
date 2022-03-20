.data
extern fov : real4
extern returnAddress : qword
extern resolvedRelativeAddress : qword

.code
	FovAdjust PROC
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		call [resolvedRelativeAddress]
		movaps xmm0, [fov]
		jmp qword ptr [returnAddress]
	FovAdjust ENDP
end