.data
extern fov : xmmword
extern returnAddress : qword
extern resolvedRelativeAddress : qword

.code
	FovAdjust PROC
		REPEAT 9
			nop
		ENDM
		call [resolvedRelativeAddress]
		movaps xmm0,xmmword ptr [fov]
		jmp qword ptr [returnAddress]
	FovAdjust ENDP
end