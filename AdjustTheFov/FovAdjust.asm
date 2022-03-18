.data
extern fov : real4
extern returnAddr : qword

.code
	FovAdjust PROC
		movaps xmm0, [fov]
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		jmp qword ptr [returnAddr]
	FovAdjust ENDP
end