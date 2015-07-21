#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

enum MODES {
	REDIR_OUT_APP,
	REDIR_OUT_OVW,
	REDIR_IN,
	REDIR_IN_HERE,
	PIPE
};

int mysystem(char* command_string);
char* get_command_path(char* command);
void run_command_str(char** arr_words, int num_words, int out_fd);
void command_string_slice(char* command_string, char*** argument_string_ptr, int* num_args);

int main(int argc, char** argv) {
	// First, implement the system() library call
	
	mysystem(argv[1]);
	return 0;
}

int mysystem(char* command_string) {
	int cmd_start = 0;
	int cmd_current = 0;
	
	char** argument_string;
	int num_args;
	printf("command_string: |%s|\n", command_string);
	command_string_slice(command_string, &argument_string, &num_args);
	
	int i;
	for(i=0; i<num_args; i++)
	{
		printf("arg: |%s|\n", argument_string[i]);
	}
	
	while (cmd_current < num_args)
	{
		printf("cmd_current: |%s|\n", argument_string[cmd_current]);
		if (cmd_current == num_args-1)
		{
			
			run_command_str(&argument_string[cmd_start], cmd_current - cmd_start + 1, STDOUT_FILENO);
			cmd_current++;
			
		} 
		
		else if (argument_string[cmd_current][0] == '>') {
			
			// Lookahead, Open (TRUNC) new file, run command with file as stdout
			char* outfile = argument_string[cmd_current + 1];
			int out_fd;
			if ((out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
			{
				perror("open out file: ");
				exit(EXIT_FAILURE);
			}
			
			run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, out_fd);
			cmd_current += 2;
			
			if (close(out_fd) == -1)
			{
				perror("close out file: ");
				exit(EXIT_FAILURE);
			}
			
		} 
		
		else {
			cmd_current++;
		}
	}
	return 0;
}

char* get_command_path(char* command) {
	char* path_env_ptr = getenv("PATH");
	char* end_path_ptr;
	char* path;
	
	while ((end_path_ptr = strchr(path_env_ptr, ':')) != NULL)
	{
		char possible_path[end_path_ptr - path_env_ptr + 1 + strlen(command) + 1];
		snprintf(possible_path, end_path_ptr - path_env_ptr + 1, "%s", path_env_ptr);
		
		strcat(possible_path, "/");
		strcat(possible_path, command);
		
		if (access(possible_path, F_OK) == 0) {
			path = malloc(strlen(possible_path));
			strcpy(path, possible_path);
			return path;
		}
		
		path_env_ptr = end_path_ptr + 1;
	}
	
	fprintf(stderr, "Error. Could not find command %s in path.\n", command);
	exit(EXIT_FAILURE); 
}


void run_command_str(char** arr_words, int num_words, int out_fd) {
	
	if (dup2(out_fd, STDOUT_FILENO) == -1) 
	{
		perror("dup2 on pipe/redirection into stdout: ");
		exit(EXIT_FAILURE);
	}
	
	char* command_file = get_command_path(arr_words[0]);
	
	char* params[num_words - 1 + 2];		// num_words minus command name plus command file and \0
	params[0] = command_file;
	params[num_words] = NULL;
	
	int c;
	for(c = 1; c < num_words; c++) {
		params[c] = arr_words[c];
	}
	
	int f_val = fork();
	switch(f_val) {
		case 0:
		{
			
			if (execvp(command_file, params) == -1) {
				perror("execvp: ");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
			break;
		}
		case -1:
		{
			perror("fork: ");
			exit(EXIT_FAILURE);
			break;
		}
		default:
		{
			wait(NULL);
			
			if (dup2(STDOUT_FILENO, out_fd) == -1) 
			{
				perror("dup2 on pipe/redirection into stdout: ");
				exit(EXIT_FAILURE);
			}
			
			break;
		}
	}
}

void command_string_slice(char* command_string, char*** argument_string_ptr, int* num_args) {
	int count = 1;
	char* pos = command_string;
	char* c;
	while ((c = strchr(pos, ' ')) != NULL)
	{	
		count++;
		pos = c + 1;
	}
	
	char** str_ptr = malloc(sizeof(char*) * count);
	str_ptr[0] = "hello";
	
	pos = command_string;
	int i;
	for (i = 0; i < count-1; i++)
	{	
		c = strchr(pos, ' ');
		str_ptr[i] = malloc((int) (c - pos + 1));
		strncpy(str_ptr[i], pos, c - pos);
		pos = c + 1;
	}
	
	char* command_end = command_string + strlen(command_string);
	str_ptr[count-1] = malloc(command_end - pos + 1);
	strncpy(str_ptr[count-1], pos, command_end - pos);
	
	*num_args = count;
	*argument_string_ptr = str_ptr;
}



















/* Functions to use: 
		fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
		
		int setenv(const char *name, const char *value, int overwrite);
		int unsetenv(const char *name);
		
		open(argv[1],O_WRONLY|O_CREAT, 0666) // can specify permissions in octal prefix 0
		
		dup2(f, STDOUT_FILENO);	// Set stdout to f
		
		execlp("ls","ls", "-lartn", (char*)NULL); // change process. Null terminate arglist
		
		getenv(argv[1]) // get a particular environmental var
		
		getpid(), getppid()
		
		fork() // fork the process. returns 0 to child, child ID to parent
		
		child = wait(&status); // block until a child process child returns, status in status
		
		pipe(pfds) // Create a pipe. Write to pfds[1], read from pfds[0]
		
		
*/