#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_ENTRY_NUMBER_SIZE 100
#define OUT_DESCRIPTION_MAX_SIZE 200

enum MODES {
	APPEND,
	EXTRACT
};

int main(int argc, char** argv) {
	
	char opt;
	int mode;
	char* ar_file;
	
	ar_file = argv[2];
	
	while ((opt = getopt(argc, argv, "qxtvdA")) != -1)
	{
		switch(opt)
		{
			case 'q':
				mode = APPEND;
				break;
				
			case 'x':
				mode = EXTRACT;
				break;
				
			case 't':
				break;
				
			case 'v':
				break;
				
			case 'd':
				break;
				
			case 'A':
				break;
				
			default:
				printf("You must supply an option: myar [qxtvdA].\n");
				exit(EXIT_FAILURE);
		}
		
	}
	
	switch(mode)
	{
		case APPEND:	// Open each input file and print labeled contents to archive file
		{
			int ar_fd;
			struct stat test;
			
			if (stat(ar_file, &test) != 0) 
			{
				if ((ar_fd = open(ar_file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
				{
					perror("exclusive open archive file: ");
					exit(EXIT_FAILURE);
				}
				
				char init_str[] = "!<arch>\n";
				
				if (write(ar_fd, init_str, sizeof(char) * strlen(init_str)) == -1)
				{
					perror("write to arfile: ");
					exit(EXIT_FAILURE);
				}
				
				printf("myar: creating archive %s\n", ar_file);
				if (close(ar_fd) == -1)
				{
					perror("close arfile: ");
					exit(EXIT_FAILURE);
				}
			}
			
			if ((ar_fd = open(ar_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
			{
				perror("open archive file: ");
				exit(EXIT_FAILURE);
			}
			for (int c = 3; c < argc; c++)
			{
				char* in_file = argv[c];
				int in_fd;
				
				if ((in_fd = open(in_file, O_RDONLY)) == -1)
				{
					perror("open input file: ");
					exit(EXIT_FAILURE);
				}
				
				int in_size;
				if ((in_size = lseek(in_fd, 0, SEEK_END)) == -1 || lseek(in_fd, 0, SEEK_SET) == -1)
				{
					perror("lseek input file: ");
					exit(EXIT_FAILURE);
				}
				
				char buf[in_size + 1];
				
				if (read(in_fd, buf, in_size) == -1) {
					perror("read input file: ");
					exit(EXIT_FAILURE);
				}
				buf[in_size] = '\0';
				
				int out_size = sizeof(char) * (strlen(in_file) + 1 + strlen(buf) + 2);
				char size_str[MAX_ENTRY_NUMBER_SIZE];
				snprintf(size_str, MAX_ENTRY_NUMBER_SIZE, "%d", out_size);
				
				struct stat file_info;
				if (fstat(in_fd, &file_info))
				{
					perror("lstat input file: ");
					exit(EXIT_FAILURE);
				}
				
				char st_mtime_str[OUT_DESCRIPTION_MAX_SIZE];
				snprintf(st_mtime_str, OUT_DESCRIPTION_MAX_SIZE, "%ld", file_info.st_mtime);
				
				char st_uid_str[OUT_DESCRIPTION_MAX_SIZE];
				snprintf(st_uid_str, OUT_DESCRIPTION_MAX_SIZE, "%d", file_info.st_uid);
				
				char st_gid_str[OUT_DESCRIPTION_MAX_SIZE];
				snprintf(st_gid_str, OUT_DESCRIPTION_MAX_SIZE, "%d", file_info.st_gid);
				
				char st_mode_str[OUT_DESCRIPTION_MAX_SIZE];
				snprintf(st_mode_str, OUT_DESCRIPTION_MAX_SIZE, "%o", file_info.st_mode);
				
				char st_size_str[OUT_DESCRIPTION_MAX_SIZE];
				snprintf(st_size_str, OUT_DESCRIPTION_MAX_SIZE, "%lld", file_info.st_size);
				
				char out_desc[strlen(st_mtime_str) + 2 + strlen(st_uid_str) + 3 + strlen(st_gid_str) + 4 +
					strlen(st_mode_str) + 2 + strlen(st_size_str) + 10];
				
				strcpy(out_desc, st_mtime_str);
				strcat(out_desc, "  ");
				strcat(out_desc, st_uid_str);
				strcat(out_desc, "   ");
				strcat(out_desc, st_gid_str);
				strcat(out_desc, "    ");
				strcat(out_desc, st_mode_str);
				strcat(out_desc, "  ");
				strcat(out_desc, st_size_str);
				strcat(out_desc, "         `");
				
				char out_buf[strlen(in_file) + 11 + strlen(out_desc) + 1 + strlen(buf) + 1];
				
				strcpy(out_buf, in_file);
				strcat(out_buf, "           ");
				strcat(out_buf, out_desc);
				strcat(out_buf, "\n");
				strcat(out_buf, buf);
				strcat(out_buf, "\n");
				
				if (write(ar_fd, out_buf, sizeof(char) * strlen(out_buf)) == -1)
				{
					perror("write to arfile: ");
					exit(EXIT_FAILURE);
				}
			}
			
			if (close(ar_fd) == -1)
			{
				perror("close: ");
				exit(EXIT_FAILURE);
			}
			
			break;	
		}
		
		case EXTRACT:
		{
			int ar_fd;
			if ((ar_fd = open(ar_file, O_RDONLY)) == -1)
			{
				perror("open archive file: ");
				exit(EXIT_FAILURE);
			}
			
			int ar_size;
			if ((ar_size = lseek(ar_fd, 0, SEEK_END)) == -1 || lseek(ar_fd, 0, SEEK_SET) == -1)
			{
				perror("lseek archive file: ");
				exit(EXIT_FAILURE);
			}
			
			char ar_buf[ar_size];
			if (read(ar_fd, ar_buf, ar_size) == -1) {
				perror("read archive file: ");
				exit(EXIT_FAILURE);
			}
			
			char* ar_ptr = ar_buf;
			
			char* ar_end = ar_buf + ar_size;
			while(ar_ptr < ar_end) 
			{
				
				char* entry_start = strchr(ar_ptr, '\n') + 1;
				char entry_size_str[MAX_ENTRY_NUMBER_SIZE];
				
				strncpy(entry_size_str, ar_ptr, entry_start - ar_ptr - 1);
				int entry_size = atoi(entry_size_str);
				
				char entry_str[entry_size + 1];
				strncpy(entry_str, entry_start, entry_size);
				entry_str[entry_size] = '\0';
				
				char* content_start = strchr(entry_start, '\n') + 1;
				int content_size = entry_size - (content_start - entry_start) - 2;	// Do not include trailing \n\n
				char content[content_size + 1];
				strncpy(content, content_start, content_size);
				content[content_size] = '\0';
				printf("content: %s\n", content);
				
				ar_ptr = entry_start + entry_size;
			}
		}
	}
	
	return 0;
}