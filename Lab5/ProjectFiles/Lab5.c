/*============================================================================
 *                    Exemplos de utiliza��o do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. Andr� Schneider de Oliveira
 *            Universidade Tecnol�gica Federal do Paran� (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 *      Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 *      Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 *      Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 *      Mariana Carri�o        - <mcarriao@alunos.utfpr.edu.br>
 *===========================================================================*/
#include "cmsis_os.h"
#include <stdbool.h>
#include "grlib/grlib.h"
#include "uart_functions.h"
#include "cfaf128x128x16.h"
#include <string.h>
#include "Communication.h"
#include <math.h>
#include "Utils.h"
#include "queue.h"
#include "Task.h"
#include "gantt.h"

#define M_PI 3.14159265358979323846
#define GANTT
#define SIMULATION
#define OS_ROBIN 0

#define SIGNAL_EXECUTE_THREAD 0x0010

tContext sContext;

// THREADS
void Thread_A (void const *args);
void Thread_B (void const *args);
void Thread_C (void const *args);
void Thread_D (void const *args);
void Thread_E (void const *args);
void Thread_F (void const *args);
void dispatcher();
void dispatcher_run();

osThreadDef (Thread_A, osPriorityNormal, 1, 0);
osThreadDef (Thread_B, osPriorityNormal, 1, 0);
osThreadDef (Thread_C, osPriorityNormal, 1, 0);
osThreadDef (Thread_D, osPriorityNormal, 1, 0);
osThreadDef (Thread_E, osPriorityNormal, 1, 0);
osThreadDef (Thread_F, osPriorityNormal, 1, 0);

Task task_A;
Task task_B;
Task task_C;
Task task_D;
Task task_E;
Task task_F;

osThreadId Dispatcher_id;

osTimerDef(timer, dispatcher_run);
osTimerId timer_id;

osMutexDef (mutex_running_thread);
osMutexId  (mutex_running_thread_id);

void dispatcher_run(){
	osSignalSet (Dispatcher_id, 0x0001);
}

void task_yield(Task* running_thread) {
	#ifdef GANTT
	gantt_thread_exit(running_thread->name, (int)(osKernelSysTick()/120000));
	#endif
	running_thread->status = READY;
	osSignalWait(SIGNAL_EXECUTE_THREAD, osWaitForever);
}

double factorial (int n) {
	if (n > 1)
		return n*(factorial(n-1));
	else
		return 1.0;
}

void Thread_A (void const *args) {
	int x;
	double soma;
		
	while (true) {
		for(soma = 0, x = 0; x <= 256; x++) {
			if(task_A.status != RUNNING)
				task_yield(&task_A);
			soma += x + (x + 2);
		}
		
		x = 0;
		task_yield(&task_A);
	}
}

void Thread_B (void const *args) {
	int n;
	double soma;
	
	while (true) {		
		for(soma = 0, n = 1; n <= 16; n++) {
			if(task_B.status != RUNNING)
				task_yield(&task_B);
			soma += pow(2.0, n) / factorial(n);
		}

		n = 0;
		task_yield(&task_B);
	}
}

void Thread_C (void const *args) {
	int n;
	double soma;
	
	while (true) {
		for(soma = 0, n = 1; n <= 72; n++) {
			if(task_C.status != RUNNING)
				task_yield(&task_C);
			soma += (double)(n+1)/(double)n;
		}
		
		n = 0;
		task_yield(&task_C);
	}
}

void Thread_D (void const *args) {
	double soma;
	
	while(true) {
		if(task_D.status != RUNNING)
			task_yield(&task_D);
		soma = 1 + (5.0/factorial(3)) + (5.0/factorial(5)) + (5.0/factorial(7)) + (5.0/factorial(9));
		
		soma = 0;
		task_yield(&task_D);
	}
}

void Thread_E (void const *args) {
	int x;
	double soma;

	while(true) {
		for(soma = 0, x = 1; x <= 100; x++) {
			if(task_E.status != RUNNING)
				task_yield(&task_E);		
			soma += x*M_PI*M_PI;
		}
		
		x = 0;
		task_yield(&task_E);
	}
}

void Thread_F (void const *args) {
	int y;
	double soma;
	
	while(true) {
		for(soma = 0, y = 1; y <= 128; y++) {
			if(task_F.status != RUNNING)
				task_yield(&task_F);
			soma += pow((double) y, 3.0) / pow(2.0, (double) y);
		}
		
		y = 0;
		task_yield(&task_F);
	}
}

void load_threads(Task* tasks[]) {
	tasks[0] = &task_A;
	tasks[1] = &task_B;
	tasks[2] = &task_C;
	tasks[3] = &task_D;
	tasks[4] = &task_E;
	tasks[5] = &task_F;
}

void scheduler(Task* tasks[], int size) {
	// reorder
}

void dispatcher() {
	int number_of_tasks;
	Task* ready_tasks[6];
	Task* waiting_tasks[6];
	Task* current_task;
	Task* previous_task;
	osEvent event;
	int ready_tasks_index = 0;
	int ready_tasks_size;
	int waiting_tasks_index = 0;
	number_of_tasks = 6;
	ready_tasks_size = number_of_tasks;
	
	load_threads(ready_tasks);
	current_task = 0;
	
	while(true) {
		event = osSignalWait (0x0001, osWaitForever);
		if (event.status == osEventSignal) {
			osTimerStop(timer_id);
			osMutexWait(mutex_running_thread_id, osWaitForever);
			
			//Adds previous task to the "ready tasks" queue			
			if (current_task > 0)
				put_element((void*)ready_tasks, number_of_tasks, (void*)current_task, &ready_tasks_size);			
			
			//Selects next task to be executed
			scheduler(ready_tasks, number_of_tasks);
			previous_task = current_task;
			current_task = (Task*)get_first_element((void*)&ready_tasks, &ready_tasks_size);
			
			//Sets selected task to running mode
			#ifdef GANTT
			gantt_thread_enter(current_task->name, (int)(osKernelSysTick()/120000));
			#endif
			current_task->status = RUNNING;
			osSignalSet(current_task->task_id, SIGNAL_EXECUTE_THREAD);
			
			osMutexRelease(mutex_running_thread_id);
			osTimerStart(timer_id, 10);
		}
	}
}

int main (void) {
	#ifdef GANTT
	initUART();
	gantt_generate_header();
	#endif
	
	#ifndef SIMULATION
	cfaf128x128x16Init();
	#endif
	
    osKernelInitialize();
	Dispatcher_id = osThreadGetId();	
	// os dados de executiong time are not correct
	task_A = createTask("Task A", osThreadCreate(osThread(Thread_A), NULL), 10, 8, 1000, 70);
	task_B = createTask("Task B", osThreadCreate(osThread(Thread_B), NULL), 0, 2, 1000, 50);
	task_C = createTask("Task C", osThreadCreate(osThread(Thread_C), NULL), -30, 5, 1000, 30);
	task_D = createTask("Task D", osThreadCreate(osThread(Thread_D), NULL), 0, 1, 1000, 50);
	task_E = createTask("Task E", osThreadCreate(osThread(Thread_E), NULL), -30, 6, 1000, 30);
	task_F = createTask("Task F", osThreadCreate(osThread(Thread_F), NULL), -100, 10, 1000, 10);
    osKernelStart();
	
	timer_id = osTimerCreate(osTimer(timer), osTimerPeriodic, NULL);
	mutex_running_thread_id = osMutexCreate(osMutex(mutex_running_thread));
    
	osTimerStart(timer_id, 10);
	dispatcher();
}