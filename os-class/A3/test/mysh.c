#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

enum MODES {
	REDIR_OUT_APP,
	REDIR_OUT_OVW,
	REDIR_IN,
	REDIR_IN_HERE,
	PIPE
};

int mysystem(char* command_string);
char* get_command_path(char* command);
void run_command_str(char** arr_words, int num_words, int out_fd, int in_fd);
void command_string_slice(char* command_string, char*** argument_string_ptr, int* num_args);

void signore(int sig) {
	;
}

int main(int argc, char** argv) {
	// First, implement the system() library call
	int BUF_SIZE = 100000;
	char buf[BUF_SIZE];
	int read_bytes;
	
	signal(SIGINT, signore);
	signal(SIGQUIT, signore);
	
	char pwdbuf[2048];
	
	while(1)
	{
		getcwd(pwdbuf, 2048);
		printf("%s: ", pwdbuf);
		fflush(stdout);
		if ((read_bytes = read(STDOUT_FILENO, buf, BUF_SIZE)) == -1)
		{
			perror("Couldn't read from stdout: ");
			exit(EXIT_FAILURE);
		}
		buf[read_bytes-1] = '\0';
		if (strcmp(buf, "exit") == 0){
			printf("Exiting shell.\n");
			exit(EXIT_SUCCESS);
		} else if (read_bytes > 1) {
			mysystem(buf);
		}
	}
	
	return 0;
}

int mysystem(char* command_string) {
	int cmd_start = 0;
	int cmd_current = 0;
	
	int saved_stderr = dup(STDERR_FILENO);
	
	int piping = 0;
	int pipe_fds[2];
	
	char** argument_string;
	int num_args;

	command_string_slice(command_string, &argument_string, &num_args);
	
	while (cmd_current < num_args)
	{
		if (cmd_current == num_args-1)
		{
			if (piping)
			{
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start + 1, STDOUT_FILENO, pipe_fds[0]);
				piping = 0;
				if (close(pipe_fds[0]) == -1)
				{
					perror("close read pipe: ");
					//exit(EXIT_FAILURE);
				}
				if (close(pipe_fds[1]) == -1)
				{
					perror("close write pipe: ");
					//exit(EXIT_FAILURE);
				}
				if (dup2(saved_stderr, STDERR_FILENO) == -1) 
				{
					perror("restoring file pointer of stderr: ");
				}
				saved_stderr = dup(STDERR_FILENO);
				cmd_current++;
			}
			
			else 
			{
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start + 1, STDOUT_FILENO, -1);
				cmd_current++;
			}
		} 
		
		else if (strcmp(argument_string[cmd_current], ">") == 0) {
			
			// Lookahead, Open (TRUNC) new file, run command with file as stdout
			char* outfile = argument_string[cmd_current + 1];
			int out_fd;
			if ((out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
			{
				perror("open out file: ");
				//exit(EXIT_FAILURE);
			}
			
			if (piping)
			{
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, out_fd, pipe_fds[0]);
				piping = 0;
				if (close(pipe_fds[0]) == -1)
				{
					perror("close read pipe: ");
					//exit(EXIT_FAILURE);
				}
				if (close(pipe_fds[1]) == -1)
				{
					perror("close write pipe: ");
					//exit(EXIT_FAILURE);
				}
				cmd_current += 2;
			}
			
			else {
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, out_fd, -1);
				cmd_current += 2;
			}
			
			if (close(out_fd) == -1)
			{
				perror("close out file: ");
				//exit(EXIT_FAILURE);
			}
			
		}
		
		else if (strcmp(argument_string[cmd_current], ">>") == 0) {
			
			// Lookahead, Open (APPEND) new file, run command with file as stdout
			char* outfile = argument_string[cmd_current + 1];
			int out_fd;
			if ((out_fd = open(outfile, O_WRONLY | O_APPEND)) == -1)
			{
				perror("open out file: ");
				//exit(EXIT_FAILURE);
			}
			
			if (piping)
			{
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, out_fd, pipe_fds[0]);
				piping = 0;
				if (close(pipe_fds[0]) == -1)
				{
					perror("close read pipe: ");
					//exit(EXIT_FAILURE);
				}
				if (close(pipe_fds[1]) == -1)
				{
					perror("close write pipe: ");
					//exit(EXIT_FAILURE);
				}
				cmd_current += 2;
				if (dup2(saved_stderr, STDERR_FILENO) == -1) 
				{
					perror("restoring file pointer of stderr: ");
				}
				saved_stderr = dup(STDERR_FILENO);
			}
			
			else {
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, out_fd, -1);
				cmd_current += 2;
			}
			
			if (close(out_fd) == -1)
			{
				perror("close out file: ");
				//exit(EXIT_FAILURE);
			}
			
		}
		
		else if (strcmp(argument_string[cmd_current], "<") == 0) {
			
			// Lookahead, Open (TRUNC) new file, run command with file as stdout
			char* infile = argument_string[cmd_current + 1];
			int in_fd;
			if ((in_fd = open(infile, O_RDONLY)) == -1)
			{
				perror("open in file: ");
				//exit(EXIT_FAILURE);
			}
			
			if (piping)
			{
				printf("Error. Attempting to pipe and redirect. Exiting.\n");
				//exit(EXIT_FAILURE);
			}
			
			else {
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, STDOUT_FILENO, in_fd);
				cmd_current += 2;
			}
			
			if (close(in_fd) == -1)
			{
				perror("close in file: ");
				//exit(EXIT_FAILURE);
			}
			
		}
		
		else if (strcmp(argument_string[cmd_current], "|") == 0) {
			// Open a pipe, pass its write end to program, set piping flag

			if (pipe(pipe_fds) == -1)
			{
				perror("open pipe: ");
				//exit(EXIT_FAILURE);
			}
			
			if (piping)
			{
				printf("Error, cannot chain pipes. Exiting.\n");
				//exit(EXIT_FAILURE);
			}
			
			else {
				run_command_str(&argument_string[cmd_start], cmd_current - cmd_start, pipe_fds[1], -1);
			}
			
			cmd_current += 1;
			cmd_start = cmd_current;
			
			piping = 1;
			
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
	//exit(EXIT_FAILURE); 
	return NULL;
}


void run_command_str(char** arr_words, int num_words, int out_fd, int in_fd) {
	
	int saved_stdout = dup(STDOUT_FILENO);
	int saved_stdin  = dup(STDIN_FILENO);
	
	if (in_fd != -1)
	{
		if (dup2(in_fd, STDIN_FILENO) == -1) 
		{
			perror("dup2 on pipe/redirection into stdin: ");	 // now close pipe
			//exit(EXIT_FAILURE);
		}
	}
	
	if (dup2(out_fd, STDOUT_FILENO) == -1) 
	{
		perror("dup2 on pipe/redirection into stdout: ");
		//exit(EXIT_FAILURE);
	}
	
	char* command_file = get_command_path(arr_words[0]);
	
	if (command_file == NULL) {
		return;
	}
	
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
			
			if (execve(command_file, params, NULL) == -1) {
				perror("execvp: ");
				//exit(EXIT_FAILURE);
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
			
			if (dup2(saved_stdout, STDOUT_FILENO) == -1) 
			{
				perror("dup2 on pipe/redirection into stdout: ");
				//exit(EXIT_FAILURE);
			}
			
			if (in_fd != -1)
			{
				if (dup2(saved_stdin, STDIN_FILENO)  == -1) 
				{
					perror("dup2 on pipe/redirection into stdin: ");	 // now close pipe
					//exit(EXIT_FAILURE);
				}
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
	str_ptr[count-1][command_end - pos] = '\0';
	
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