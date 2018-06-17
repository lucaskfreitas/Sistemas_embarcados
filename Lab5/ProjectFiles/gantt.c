#include "gantt.h"

void gantt_generate_header () {
	sendString("gantt\n");
    sendString("    dateFormat SSS\n");
    sendString("    axisFormat %L\n");
	sendString("    title Thread Gantt Diagram\n");
}

void gantt_thread_enter(char *thread_name, int enter_time) {
	char to_send[100];
	char enter_time_string[3];
	
	intToString(enter_time, enter_time_string, 3, 10, 3);
	
	strcpy(to_send, "    ");
	strcat(to_send, thread_name);
	strcat(to_send, ": ");
	strcat(to_send, enter_time_string);
	strcat(to_send, ", \0");
	
	sendString(to_send);
}

void gantt_thread_exit(char *thread_name, int exit_time) {
	char to_send[100];
	char exit_time_string[3];
	intToString(exit_time, exit_time_string, 3, 10, 3);
	
	strcpy(to_send, exit_time_string);
	strcat(to_send, "\n");
	
	sendString(to_send);
}