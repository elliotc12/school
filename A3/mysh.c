#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int mysystem(char** command_string, int num_words);
char* get_command_path(char* command);
void run_command_str(char** arr_words, int num_words, FILE* out_file);

int main(int argc, char** argv) {
	// First, implement the system() library call
	
	mysystem(&argv[1], argc-1);
	return 0;
}

int mysystem(char** command_string, int num_args) {
	
	run_command_str(command_string, num_args, stdout);
	
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


void run_command_str(char** arr_words, int num_words, FILE* out_file) {
	
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
			break;
		}
	}
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