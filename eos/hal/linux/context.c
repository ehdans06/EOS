#include <core/eos.h>
#include <core/eos_internal.h>
#include "emulator/private.h"

typedef struct _os_context {
	/* low address */
	int32u_t edi;
	int32u_t esi;
	int32u_t ebp;
	int32u_t esp;
	int32u_t ebx;
	int32u_t edx;
	int32u_t ecx;
	int32u_t eax;
	int32u_t eflags;
	int32u_t eip;
	/* high address */	
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
	PRINT("eip     =0x%x\n", ctx->eip);
	PRINT("eflags  =0x%x\n", ctx->eflags);
	PRINT("eax     =0x%x\n", ctx->eax);
	PRINT("ecx     =0x%x\n", ctx->ecx);
	PRINT("edx     =0x%x\n", ctx->edx);
	PRINT("ebx     =0x%x\n", ctx->ebx);
	PRINT("esp     =0x%x\n", ctx->esp);
	PRINT("ebp     =0x%x\n", ctx->ebp);
	PRINT("esi     =0x%x\n", ctx->esi);
	PRINT("edi     =0x%x\n", ctx->edi);
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
	int32u_t* sp = stack_base + stack_size;
	*(--sp) = arg;
	*(--sp) = 0;
	*(--sp) = entry;
	*(--sp) = 1;
	return sp-=8;
}

void _os_restore_context(addr_t sp){

	__asm__ __volatile__ ("\
		mov %0, %%esp;\
		popa;\
		pop %1;\
		ret;"
		::"m"(sp), "m"(_eflags):);
}

addr_t _os_save_context(){
	
	__asm__ __volatile__ ("\
		push $resume_point;\
		push %0;\
		mov $0, %%eax;\
		pusha;\
		mov %%esp, %%eax;\
		push 4(%%ebp);\
		push 0(%%ebp);\
		mov %%esp, %%ebp;\
		resume_point:\
		"
		:: "m"(_eflags):);
}