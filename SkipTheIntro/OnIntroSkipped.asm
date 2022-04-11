.data
extern returnAddress : qword
extern introSkipped : byte

.code
	OnIntroSkipped proc
		repeat 19
			nop
		endm
		mov introSkipped,1
		jmp returnAddress
	OnIntroSkipped endp
end