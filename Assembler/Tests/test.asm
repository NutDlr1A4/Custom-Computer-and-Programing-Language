; v1.0
@prog
.start		port 1				; Port set to terminal

			ld r2 helloworld	; r2 points to string
.loop		ldb r1 r2			; Reads character at pointer
			cmp r1 '\0'			
			beq endloop			; If char == '\0', break from loop
			wport r1			; Write char to terminal
			add r2 1
			bra loop			; Loop back

.endloop	ld r2 helloworld2	; r2 points to the second string
.loop2		ldb r1 r2			; Reads character at pointer
			cmp r1 '\0'			
			beq endloop2		; If char == '\0', break from loop
			wport r1			; Write char to terminal
			add r2 1
			bra loop2			; Loop back

.endloop2	hlt					; Halt program

@data
.helloworld	
	"Hello World!\n"
.helloworld2
	"I'm also a Hello World!\n"
	"I even support multiple lines!\n"