#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

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
			
			if ((ar_fd = open(ar_file, O_WRONLY | O_APPEND)) == -1)
			{
				perror("open archive file: ");
				exit(EXIT_FAILURE);
			}
			int c;
			for (c = 3; c < argc; c++)
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
				
				char out_desc[60];
				int j;
				for (j = 0; j < 59; j++)
				{
					out_desc[j] = ' ';
				}
				
				snprintf(out_desc, 16, "%s", in_file);
				snprintf(&out_desc[16], 12, "%ld", file_info.st_mtime);
				snprintf(&out_desc[28], 6, "%d", file_info.st_uid);
				snprintf(&out_desc[34], 6, "%d", file_info.st_gid);
				snprintf(&out_desc[40], 8, "%o", file_info.st_mode);
				snprintf(&out_desc[48], 10, "%lld", file_info.st_size);
				out_desc[58] = '`';
				
				int k;
				for (k = 0; k < 59; k++)
				{
					if (out_desc[k] == '\0') {
						out_desc[k] = ' ';
					}
				}
				
				out_desc[59] = '\0';
				
				char out_buf[1 + strlen(out_desc) + 1 + strlen(buf)];
				
				int file_pos;
				if ((file_pos = lseek(ar_fd, 0, SEEK_CUR)) == -1)
				{
					perror("lseek archive file: ");
					exit(EXIT_FAILURE);
				}
				
				if (file_pos % 2 != 0) {
					strcpy(out_buf, "\n");
					strcat(out_buf, out_desc);
					strcat(out_buf, "\n");
					strcat(out_buf, buf);
				} else {
					strcpy(out_buf, out_desc);
					strcat(out_buf, "\n");
					strcat(out_buf, buf);
				}
				
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
			
			char* ar_ptr = strchr(ar_buf, '\n') + 1;
					
			char* ar_end = ar_buf + ar_size;
			
			while(ar_ptr < ar_end) 
			{
				if (*ar_ptr == '\n')
				{
					ar_ptr++;
				}
				
				char* entry_start = strchr(ar_ptr, '\n') + 1;
				
				int entry_name_str_len = strchr(&ar_ptr[0], ' ') - &ar_ptr[0];
				if (entry_name_str_len > 16)	entry_name_str_len = 16;
				char entry_name[entry_name_str_len + 1];
				strncpy(entry_name, &ar_ptr[0], entry_name_str_len);
				entry_name[entry_name_str_len] = '\0';
				//printf("name: %s\t", entry_name);
				
				int entry_ts_str_len = strchr(&ar_ptr[16], ' ') - &ar_ptr[16];
				if (entry_ts_str_len > 12)	entry_ts_str_len = 12;
				char entry_ts_str[entry_ts_str_len + 1];
				strncpy(entry_ts_str, &ar_ptr[16], entry_ts_str_len);
				entry_ts_str[entry_ts_str_len] = '\0';
				time_t entry_ts = atol(entry_ts_str);
				//printf("ts: %s\t", ctime( &entry_ts));
				
				int entry_uid_str_len = strchr(&ar_ptr[28], ' ') - &ar_ptr[28];
				if (entry_uid_str_len > 6)	entry_uid_str_len = 6;
				char entry_uid_str[entry_uid_str_len + 1];
				strncpy(entry_uid_str, &ar_ptr[28], entry_uid_str_len);
				entry_uid_str[entry_uid_str_len] = '\0';
				long entry_uid = atol(entry_uid_str);
				//printf("uid: %ld\t", entry_uid);
				
				int entry_gid_str_len = strchr(&ar_ptr[34], ' ') - &ar_ptr[34];
				if (entry_gid_str_len > 6)	entry_gid_str_len = 6;
				char entry_gid_str[entry_gid_str_len + 1];
				strncpy(entry_gid_str, &ar_ptr[34], entry_gid_str_len);
				entry_gid_str[entry_gid_str_len] = '\0';
				long entry_gid = atol(entry_gid_str);
				//printf("gid: %ld\t", entry_gid);
				
				int entry_mode_str_len = strchr(&ar_ptr[40], ' ') - &ar_ptr[40];
				if (entry_mode_str_len > 6)	entry_mode_str_len = 6;
				char entry_mode_str[entry_mode_str_len + 1];
				strncpy(entry_mode_str, &ar_ptr[40], entry_mode_str_len);
				entry_mode_str[entry_mode_str_len] = '\0';
				long entry_mode = strtol(entry_mode_str, NULL, 8);
				//printf("mode: %o\n", (int) entry_mode);
				
				int entry_size_str_len = strchr(&ar_ptr[48], ' ') - &ar_ptr[48];
				if (entry_size_str_len > 10)	entry_size_str_len = 10;
				char entry_size_str[entry_size_str_len + 1];
				strncpy(entry_size_str, &ar_ptr[48], entry_size_str_len);
				entry_size_str[entry_size_str_len] = '\0';
				int entry_size = atoi(entry_size_str);
				//printf("size: %d\n", entry_size);
				
				char entry_str[entry_size + 1];
				strncpy(entry_str, entry_start, entry_size);
				entry_str[entry_size] = '\0';
				
				int out_fd;
				if ((out_fd = open(entry_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
				{
					perror("open out file: ");
					exit(EXIT_FAILURE);
				}
				
				struct timeval out_time_arr[2];
				out_time_arr[0].tv_sec = entry_ts;
				out_time_arr[0].tv_usec = 0;
				out_time_arr[1].tv_sec = entry_ts;
				out_time_arr[1].tv_usec = 0;	
							
				if (write(out_fd, entry_str, entry_size) == -1)
				{
					perror("write to out file: ");
					exit(EXIT_FAILURE);
				}
				
				if (futimes(out_fd, out_time_arr) == -1) {
					perror("futimes on out file: ");
					exit(EXIT_FAILURE);
				}
				
				if (fchown(out_fd, entry_uid, entry_gid) == -1) {
					perror("fchown on out file: ");
					exit(EXIT_FAILURE);
				}
				
				if (fchmod(out_fd, entry_mode) == -1) {
					perror("fchmod on out file: ");
					exit(EXIT_FAILURE);
				}
				
				
				printf("atime: %ld\n", file_info.st_atime);
				printf("mtime: %ld\n", file_info.st_mtime);
				printf("mode:  %d\n", file_info.st_mode);
				printf("uid:   %d\n", file_info.st_uid);
				printf("gid:   %d\n", file_info.st_gid);
				printf("size:  %lld\n\n", file_info.st_size);
				
				if (close(out_fd) == -1) {
					perror("close out file: ");
					exit(EXIT_FAILURE);
				}
		
				ar_ptr = entry_start + entry_size;
			}
			
		}
	}
	
	return 0;
}
