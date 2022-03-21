.data
extern fov : xmmword
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
		movaps xmm0,xmmword ptr [fov]
		jmp qword ptr [returnAddress]
	FovAdjust ENDP
end