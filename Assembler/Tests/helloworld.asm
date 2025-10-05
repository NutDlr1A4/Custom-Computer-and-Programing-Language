@data
.helloworld	
	"Hello World!\n\0"

@prog
.start		port 1				; Port set to terminal
			ld r2 helloworld	; r2 points to string
.loop		ldb r1 r2			; Reads character at pointer
			cmp r1 '\0'			
			beq endloop			; If char == '\0', break from loop
			wport r1			; Write char to terminal
			add r2 1
			bra loop			; Loop back
.endloop	hlt					; Halt program