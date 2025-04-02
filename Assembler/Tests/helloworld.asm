; Simple hello world program (hopefully)
@code	
.start		sport 1				; Port set to terminal
			ld r2 helloworld	; r2 points to string
.loop		ldb r1 r2			; Reads character at pointer
			cmp r1 '\0'			
			beq endloop			; If char == '\0', break from loop
			wport r1			; Write char to terminal
			add r2 1
			bra loop			; Loop back
.endloop	hlt					; Halt program

@data
.helloworld		
			"Hello World!\n\0"



			asdasd
			"aasd
.123123

'\s'

; I know it's full of errors, leave me alone :(