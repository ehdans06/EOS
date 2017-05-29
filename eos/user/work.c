#include <core/eos.h>

#define STACK_SIZE 8096

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;
static int8u_t stack1[STACK_SIZE];
static int8u_t stack2[STACK_SIZE];
static int8u_t stack3[STACK_SIZE];

void task1() {
	while(1) {
		PRINT("A\n");
		eos_sleep(0);
	}
}

void task2() {
	while(1) {
		PRINT("B\n");
		eos_sleep(0);
	}
}

void task3() {
	while(1) {
		PRINT("C\n");
		eos_sleep(0);
	}
}

void eos_user_main() {
	eos_create_task(&tcb1, (addr_t)stack1, 8096, task1, NULL, 1);
	eos_set_period(&tcb1, 2);

	eos_create_task(&tcb2, (addr_t)stack2, 8096, task2, NULL, 10);
	eos_set_period(&tcb2, 4);

	eos_create_task(&tcb3, (addr_t)stack3, 8096, task3, NULL, 50);
	eos_set_period(&tcb3, 8);
}
