#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_ARGS 16

char* strip_whitespace(char *start){
	for(; isspace(*start); start++);
	char *actual_text_start = start;
	char inquotes = 0;
	for(; (inquotes || !isspace(*start)) && *start; start++)
		if(*start == '"')
			inquotes = !inquotes;
	*start = 0;
	return actual_text_start;
}
void make_argv(char **argv, int *p_argc, char *cmd)
{
		*p_argc = 1;
		argv[0] = cmd;
		char *maybe_arguement;
		do{ //a good place to add on project 2 functionality
			maybe_arguement = strip_whitespace(argv[*p_argc - 1] + strlen(argv[*p_argc -1]) +1);
			if(*maybe_arguement)
			{
				argv[*p_argc] = maybe_arguement;
				//printf("%s\n", maybe_arguement);
				(*p_argc)++;
			}
			
		}while(*maybe_arguement);
		argv[*p_argc] =0;

}
int run_command(char *command, int standard_in, int standard_out, int standard_error){

		char *cmd = strip_whitespace(command);
		char *argv[MAX_ARGS];
		int argc = 1;
		//argv[0] = cmd;
		make_argv(argv, &argc, cmd);	
		pid_t pid = fork();
		if(pid){ //we are in parent process
			if(standard_in != 0)
				close(standard_in);
			if(standard_out != 1)
				close(standard_out);
			if(standard_error != 2)
					close(standard_error);
		}
		else{ //we are in child process, also where redirects should take place
			if(standard_in !=0)
				dup2(standard_in, 0);
			if(standard_out != 1)
				dup2(standard_out, 1);
			if(standard_error != 2)
				dup2(standard_error, 2);
			execvp(cmd, argv);
			perror(cmd);
			exit(1);
		}
}

int main()
{
	char *prompt = " >>>>> ";
	char cmdbuffer[1024];
	ssize_t readlen;

	while(1){
		write(1, prompt, strlen(prompt));
		readlen = read(0, cmdbuffer, 1024);
		cmdbuffer[readlen] = 0;
		if(!readlen)
			break;
		int pipe_index = 0;
		char inquotes = 0;
		char *commands[16];
		int total_commands = 1;
		commands[0] = cmdbuffer;
		//parses and looks for pipes
		for(int i = 0; cmdbuffer[i]; i++)
		{
			if(cmdbuffer[i] == '"' && cmdbuffer[i] != '\\')
				inquotes = !inquotes;
			if(cmdbuffer[i] == '|' && !inquotes)
			{
				commands[total_commands] = cmdbuffer + i + 1;
				printf("%s\n", commands[total_commands]);
				total_commands++;
				cmdbuffer[i] = 0;
				i++;
			}
		}
		char*redirect_operators[16];
		bool redirect = false;
		for(int i = 0; cmdbuffer[i]; i++)
		{
			if(cmdbuffer[i] == '"' && cmdbuffer[i] != '\\')
				inquotes = !inquotes;
			if(cmdbuffer[i] == '2' && cmdbuffer[i+1] == '>' && cmdbuffer[i+2] == '>' && !inquotes) //for 2>> redirect
			{
				redirect_operators[0] = strip_whitespace(cmdbuffer);
				redirect_operators[1] = strip_whitespace(cmdbuffer + i + 4);
				cmdbuffer[i] = 0;
				cmdbuffer[i+1] = 0;
				cmdbuffer[i+2] = 0;
				int fd = open(redirect_operators[1], O_WRONLY | O_CREAT | O_APPEND, 0644);
				run_command(redirect_operators[0], 0, 1, fd);
				wait(0);
				redirect = true;

			}
			else if(cmdbuffer[i] == '2' && cmdbuffer[i+1] == '>' && !inquotes) // for 2> redirect
			{
				redirect_operators[0] = strip_whitespace(cmdbuffer);
				redirect_operators[1] = strip_whitespace(cmdbuffer + i + 3);
				cmdbuffer[i] = 0;
				cmdbuffer[i+1] = 0;
				int fd = open(redirect_operators[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				run_command(redirect_operators[0], 0, 1, fd);	
				wait(0);
				redirect = true;

			}
			else if(cmdbuffer[i] == '>' && cmdbuffer[i+1] == '>' && !inquotes) //for >> redirect
			{
				redirect_operators[0] = strip_whitespace(cmdbuffer);
				redirect_operators[1] = strip_whitespace(cmdbuffer + i + 3);
				cmdbuffer[i] = 0;
				cmdbuffer[i+1] = 0;
				int fd = open(redirect_operators[1], O_WRONLY | O_CREAT | O_APPEND, 0644);
				run_command(redirect_operators[0], 0, fd, 2);
				wait(0);
				redirect = true;
			}

			else if(cmdbuffer[i] == '>' && !inquotes)
			{
				redirect_operators[0] = strip_whitespace(cmdbuffer);
				redirect_operators[1] = strip_whitespace(cmdbuffer + i + 2);
				cmdbuffer[i] = 0;
				int fd = open(redirect_operators[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				run_command(redirect_operators[0], 0, fd, 2);
				wait(0);
				redirect = true;
				
			}
			else if(cmdbuffer[i] == '<' && !inquotes)
			{
				redirect_operators[0] = strip_whitespace(cmdbuffer);
				redirect_operators[1] = strip_whitespace(cmdbuffer + i + 2);
				cmdbuffer[i] = 0;
				int fd = open(redirect_operators[1], O_RDONLY, 0644); 
				run_command(redirect_operators[0], fd, 1, 2);
				wait(0);
				redirect = true;
				
			}
			else	
				continue;
		}
		//for shell built ins
		if(!strncmp(cmdbuffer, "cd ", 3))
		{
			char *argv[16];
			int argc;
			char *cmd = strip_whitespace(cmdbuffer);
			make_argv(argv, &argc, cmd);
			if(chdir(argv[1]))
				printf("No such dictionary %s\n", argv[1]);
			continue;
		}
	
		//for executing programs
		int input = 0;
		for(int i = 0; i < total_commands; i++)
		{
			if(redirect == true)
				break;
			if(i + 1 == total_commands) // if on last command
				run_command(commands[i], input, 1, 2);
			else //not on last one
			{
				int pipe_fds[2];
				pipe(pipe_fds);
				run_command(commands[i], input, pipe_fds[1], 2);
				input = pipe_fds[0];
			}	
		}
		for(int i = 0; i < total_commands; i++)
			wait(0);
		
	}
	write(1, "\n", 1);
	return 0;
}
