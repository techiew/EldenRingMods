.data
extern fov : xmmword
extern returnAddress : qword
extern resolvedRelativeAddress : qword

.code
	FovAdjust proc
		repeat 9
			nop
		endm
		call [resolvedRelativeAddress]
		movaps xmm0,xmmword ptr [fov]
		jmp qword ptr [returnAddress]
	FovAdjust endp
end