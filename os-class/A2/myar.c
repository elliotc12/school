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
#include <dirent.h>

#define MAX_ENTRY_NUMBER_SIZE 100
#define OUT_DESCRIPTION_MAX_SIZE 200

enum MODES {
	APPEND = 1,
	EXTRACT,
	SUMMARIZE,
	DELETE,
	ADD
};

int main(int argc, char** argv) {
	
	char opt;
	int mode = 0;
	int verbose = 0;
	char* ar_file;
	
	ar_file = argv[2];
	
	while ((opt = getopt(argc, argv, "q:x:t:vd:A")) != -1)
	{
		switch(opt)
		{
			case 'q':
				if (mode != 0) {
					printf("Error. Options q, x, t, d and A are exclusive.\n");
					exit(EXIT_FAILURE);
				}
				if (argc < 4) {
					printf("Error. Option -q requires two or more arguments: myar -q arfile file [file2 ...]\n");
					exit(EXIT_FAILURE);
				}
				mode = APPEND;
				break;
				
			case 'x':
				if (mode != 0) {
					printf("Error. Options q, x, t, d and A are exclusive.\n");
					exit(EXIT_FAILURE);
				}
				if (argc != 3) {
					printf("Error. -x option takes only one argument: myar -x arfile\n");
					exit(EXIT_FAILURE);
				}
				mode = EXTRACT;
				break;
				
			case 't':
				if (mode != 0) {
					printf("Error. Options q, x, t, d and A are exclusive.\n");
					exit(EXIT_FAILURE);
				}
				if (argc != 3) {
					printf("Error. -t option takes only one argument: myar -t arfile\n");
					exit(EXIT_FAILURE);
				}
				mode = SUMMARIZE;
				break;
				
			case 'v':
				verbose = 1;
				break;
				
			case 'd':
				if (mode != 0) {
					printf("Error. Options q, x, t, d and A are exclusive.\n");
					exit(EXIT_FAILURE);
				}
				if (argc < 4) {
					printf("Error. -d option takes at least two arguments: myar -d arfile file [file2 ...]\n");
					exit(EXIT_FAILURE);
				}
				mode = DELETE;
				break;
				
			case 'A':
				if (mode != 0) {
					printf("Error. Options q, x, t, d and A are exclusive.\n");
					exit(EXIT_FAILURE);
				}
				if (argc != 3) {
					printf("Error. -A option takes only one argument: myar -A arfile\n");
					exit(EXIT_FAILURE);
				}
				mode = ADD;
				break;
				
			default:
				printf("Correct usage: myar [qxtvdA] archive [file ...].\n");
				exit(EXIT_FAILURE);
		}
		
	}
	
	if ((verbose == 1) && (mode != SUMMARIZE)) {
		printf("Error. -v flag must be accompanied by -t flag.\n");
		exit(EXIT_FAILURE);
	}
	
	switch(mode)
	{
		case APPEND:	// Open each input file and print labeled contents to archive file
		{
			int ar_fd;
			struct stat test;
			
			if (stat(ar_file, &test) != 0) 
			{
				if ((ar_fd = open(ar_file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH)) == -1)
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
				
				strcpy(out_buf, out_desc);
				strcat(out_buf, "\n");
				strcat(out_buf, buf);
				
				if ((file_pos + strlen(out_buf)) % 2 != 0) {
					strcat(out_buf, "\n");
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
			char* ar_start = ar_ptr;
					
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
				
				if (close(out_fd) == -1) {
					perror("close out file: ");
					exit(EXIT_FAILURE);
				}
		
				ar_ptr = entry_start + entry_size;
				if ((ar_ptr - ar_start) % 2 != 0) {
					ar_ptr++;
				}
				
			}
			break;
		}
		
		case SUMMARIZE: 
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
				
				if (verbose) {
					int entry_name_str_len = strchr(ar_ptr, ' ') - ar_ptr;
					if (entry_name_str_len > 16)	entry_name_str_len = 16;
					char entry_name[entry_name_str_len + 1];
					strncpy(entry_name, ar_ptr, entry_name_str_len);
					entry_name[entry_name_str_len] = '\0';

					int entry_size_str_len = strchr(&ar_ptr[48], ' ') - &ar_ptr[48];
					if (entry_size_str_len > 10)	entry_size_str_len = 10;
					char entry_size_str[entry_size_str_len + 1];
					strncpy(entry_size_str, &ar_ptr[48], entry_size_str_len);
					entry_size_str[entry_size_str_len] = '\0';
					int entry_size = atoi(entry_size_str);
					
					int entry_ts_str_len = strchr(&ar_ptr[16], ' ') - &ar_ptr[16];
					if (entry_ts_str_len > 12)	entry_ts_str_len = 12;
					char entry_ts_str[entry_ts_str_len + 1];
					strncpy(entry_ts_str, &ar_ptr[16], entry_ts_str_len);
					entry_ts_str[entry_ts_str_len] = '\0';
					time_t entry_ts = atol(entry_ts_str);
					
					int entry_uid_str_len = strchr(&ar_ptr[28], ' ') - &ar_ptr[28];
					if (entry_uid_str_len > 6)	entry_uid_str_len = 6;
					char entry_uid_str[entry_uid_str_len + 1];
					strncpy(entry_uid_str, &ar_ptr[28], entry_uid_str_len);
					entry_uid_str[entry_uid_str_len] = '\0';
					
					int entry_gid_str_len = strchr(&ar_ptr[34], ' ') - &ar_ptr[34];
					if (entry_gid_str_len > 6)	entry_gid_str_len = 6;
					char entry_gid_str[entry_gid_str_len + 1];
					strncpy(entry_gid_str, &ar_ptr[34], entry_gid_str_len);
					entry_gid_str[entry_gid_str_len] = '\0';
					
					int entry_mode_str_len = strchr(&ar_ptr[40], ' ') - &ar_ptr[40];
					if (entry_mode_str_len > 6)	entry_mode_str_len = 6;
					char entry_mode_str[entry_mode_str_len + 1];
					strncpy(entry_mode_str, &ar_ptr[40], entry_mode_str_len);
					entry_mode_str[entry_mode_str_len] = '\0';
					//int entry_mode = (int) strtol(entry_mode_str, NULL, 8);
					
					char* ls_perms = "rwxrwxrwx";
					char own_buf[30];
					
					strcpy(own_buf, entry_uid_str);
					strcat(own_buf, "/");
					strcat(own_buf, entry_gid_str);
					
					
					char out_desc[100];
					int k;
					for (k = 0; k < 100; k++)
					{
						out_desc[k] = ' ';
					}
					
					snprintf(out_desc, 10, "%s", ls_perms);
					snprintf(&out_desc[14], 8, "%s", own_buf);
					snprintf(&out_desc[32 - strlen(entry_size_str)], 12, "%s", entry_size_str);
					snprintf(&out_desc[34], 17, "%s", ctime(&entry_ts));
					snprintf(&out_desc[52], strlen(entry_name)+1, "%s", entry_name);
					
					for (k = 0; k < 100; k++)
					{
						if (out_desc[k] == '\0') {
							out_desc[k] = ' ';
						}
					}
					out_desc[52 + strlen(entry_name)] = '\n';
					out_desc[52 + strlen(entry_name) + 1] = '\0';
					
					printf("%s", out_desc);
					
					ar_ptr = entry_start + entry_size;
					
				} else {
				
				int entry_name_str_len = strchr(&ar_ptr[0], ' ') - &ar_ptr[0];
				if (entry_name_str_len > 16)	entry_name_str_len = 16;
				char entry_name[entry_name_str_len + 1];
				strncpy(entry_name, &ar_ptr[0], entry_name_str_len);
				entry_name[entry_name_str_len] = '\0';
				
				int entry_size_str_len = strchr(&ar_ptr[48], ' ') - &ar_ptr[48];
				if (entry_size_str_len > 10)	entry_size_str_len = 10;
				char entry_size_str[entry_size_str_len + 1];
				strncpy(entry_size_str, &ar_ptr[48], entry_size_str_len);
				entry_size_str[entry_size_str_len] = '\0';
				int entry_size = atoi(entry_size_str);
				
				printf("%s\n", entry_name);
				
				ar_ptr = entry_start + entry_size;
				}
			}
			break;
		}
		
		case DELETE:
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
			
			char temp_arfile[strlen(ar_buf)];
			strcpy(temp_arfile, "!<arch>\n");
			
			char* ar_ptr = ar_buf + 8; // Start at first entry description
			char* ar_end = ar_buf + ar_size;
			int to_add;
			
			while(ar_ptr < ar_end) 
			{	
				to_add = 1;
            
				int entry_name_str_len = strchr(&ar_ptr[0], ' ') - &ar_ptr[0];
				if (entry_name_str_len > 16)	entry_name_str_len = 16;
				char entry_name[entry_name_str_len + 1];
				strncpy(entry_name, ar_ptr, entry_name_str_len);
				entry_name[entry_name_str_len] = '\0';
            
				int entry_size_str_len = strchr(&ar_ptr[48], ' ') - &ar_ptr[48];
				if (entry_size_str_len > 10)	entry_size_str_len = 10;
				char entry_size_str[entry_size_str_len + 1];
				strncpy(entry_size_str, &ar_ptr[48], entry_size_str_len);
				entry_size_str[entry_size_str_len] = '\0';
				int entry_size = atoi(entry_size_str);
				
				int c;
				for (c = 3; c < argc; c++)
				{
					if (strcmp(entry_name, argv[c]) == 0)
						to_add = 0;
				}
			
				if (to_add == 1) {
					long next_entry = (long) ar_ptr + 60 + entry_size;
					if (entry_size % 2 != 0) {
						next_entry++;
					}
					strncat(temp_arfile, ar_ptr, (int) (next_entry - (long) ar_ptr));
				}
				
				ar_ptr += 60 + entry_size;
				if (entry_size % 2 != 0)
				{
					ar_ptr++;
				}
			}
			
			if (strlen(temp_arfile) == 8) {
				if (remove(argv[2]) != 0) {
					perror("Delete archive file: ");
					exit(EXIT_FAILURE);
				}
			} else {
				if (close(ar_fd) == -1) {
					perror("close arfile: ");
				}
				if ((ar_fd = open(argv[2], O_WRONLY | O_TRUNC)) == -1) {
					perror("open truncated arfile: ");
					exit(EXIT_FAILURE);
				}
				if (write(ar_fd, temp_arfile, strlen(temp_arfile)) == -1) {
					perror("write to truncated arfile: ");
					exit(EXIT_FAILURE);
				}
				if (close(ar_fd) == -1) {
					perror("close arfile: ");
				}
			}
			break;
		}
		
		case ADD:
		{	
			DIR *dir;
			struct dirent *dir_struct;
			
			dir = opendir(".");
			int ar_fd;
			struct stat test;
			
			if (stat(ar_file, &test) != 0) 
			{
				if ((ar_fd = open(ar_file, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH| S_IROTH)) == -1)
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
			
			while ((dir_struct = readdir(dir)) != NULL) {
				if (dir_struct->d_type == DT_REG && strcmp(dir_struct->d_name, ar_file) != 0 && strcmp(dir_struct->d_name, "myar") != 0) {
					printf("file: %s\n", dir_struct->d_name);
					
					char* in_file = dir_struct->d_name;
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

					strcpy(out_buf, out_desc);
					strcat(out_buf, "\n");
					strcat(out_buf, buf);

					if ((file_pos + strlen(out_buf)) % 2 != 0) {
						strcat(out_buf, "\n");
					}

					if (write(ar_fd, out_buf, sizeof(char) * strlen(out_buf)) == -1)
					{
						perror("write to arfile: ");
						exit(EXIT_FAILURE);
					}	
					
				}
			}
			
			if (close(ar_fd) == -1)
			{
				perror("close archive file: ");
				exit(EXIT_FAILURE);
			}
			
			if (closedir(dir) == -1) {
				perror("close dir: ");
				exit(EXIT_FAILURE);
			}
			
			break;
		}
		
	}
	
	return 0;
}
