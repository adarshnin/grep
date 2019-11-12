#include "head.h"

int check(char *token, const char *substr){
	char *ptr;
	int pos, len;
	len = strlen(substr);
	if (iflag)
		ptr = strcasestr(token, substr);
	else
		ptr = strstr(token, substr);
	if (!ptr)
		return 0;
	pos = ptr - token;
	
	if (valid(token[pos -1]) || valid(token[pos + len])){
		return 0;
	}
	return 1;
}
int valid(char ch){
	if ((isalpha(ch) || isdigit(ch) || ch == '_')){
		return 1;
	}
	return 0;
}	
/*Check inside a word, used for -w*/
/*If input = &^hi&$*%`*/
char *excep(char *token, const char *substr){
	char *buffer = strdup("");
	int pos = 0, ref = 0;
	int len_str = strlen(substr);
	char *ptr = NULL;
	if (iflag)
		ptr = strcasestr(token, substr);
	else
		ptr = strstr(token, substr);
	pos = ptr - token;
	int g;
	while(ptr){

		if (!valid(token[pos -1]) && !valid(token[pos + len_str])){
			if (!qflag)
				printf("%.*s",pos - ref, token + ref);
			ref = pos + len_str;
			g = asprintf(&buffer, "%s%s%s%s", buffer, TRED, substr, TNRM);
			/*Check for malloc*/
			if (g == -1){
				exit(EXIT_FAILURE);
			}
		}
		else{
		}
		ptr = strstr(ptr +1, substr);
		pos = ptr - token;
	}     
	g = asprintf(&buffer, "%s%.*s", buffer, pos - ref, token + ref);  
	if (g == -1){
		exit(EXIT_FAILURE);
	}
	return buffer;
}

void modify(char *line, int read){
	if (line[read - 1] == '\n')
		line[read -1] = '\0';
}


void recursive(const char *basePath, const char *substr){
	struct dirent *dent;
	char path[1024];
	DIR *dir = opendir(basePath);
	static int enter = 0;
	if (!dir){
		/*Check if single file entered*/
		if (enter == 0){
			enter = 1;
			FILE *fs;
			fs = fopen(basePath, "r");
			if (fs == NULL){
				printf("File not found\n");
				exit(EXIT_FAILURE);
			}
			if(wflag){
				search((char *)basePath, substr);
			}
			else{
				find((char *)basePath, substr);
			}

		}
		enter = 0;
		return;
	}

	while ((dent = readdir(dir)) != NULL)
	{
		enter = 1;
		if (strcmp(dent->d_name, ".") != 0 && strcmp(dent->d_name, "..") != 0)
		{
            /* Construct new path from base path */
			strcpy(path, basePath);
			if (path[strlen(path) - 1] != '/')
				strcat(path, "/");
			strcat(path, dent->d_name);

			if (!(dent->d_type == DT_DIR)){
				if(wflag){
					search(path, substr);
				}
				else{
					find(path, substr);
				}
			}
			recursive(path, substr);
		}
		else{
			if (!hflag)
				fileflag = 1;
		}
	}

	enter = 0;
	closedir(dir);
}	
void find(char *path, const char *substr){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *fs;
	fs = fopen(path, "r");
	if (fs == NULL){
		printf("File not found");
		exit(EXIT_FAILURE);
	}
	char *ptr = NULL;

	int ccnt = 0;
	char *tline = NULL;
	int vccnt = 0;
	int pos, prevpos;
	int mcnt = 0;
	int mvcnt = 0;

	while ((read = getline(&line, &len, fs)) != -1) {
		modify(line, read);
		if (mnum == 0){
			break;
		}
		if (mflag && mcnt == mnum){
			break;
		}

		if(tline)
			free(tline);
		tline = (char *)malloc(read);
		if(!tline){
			printf("Error! memory not allocated.");
			exit(EXIT_FAILURE);
		}
		strcpy(tline, line);

		if(iflag)
			ptr = strcasestr(tline, substr);
		else
			ptr = strstr(tline, substr);
		
		if (ptr == NULL){
			if (mflag){
				if (mvcnt == mnum && ( (vflag && cflag) || (vflag && !cflag ))){
					break;
				}
				mvcnt++;
			}
			if (vflag){
				if (!cflag){
					if(!qflag && nl++)
						printf("\n");
					if (fileflag){
						print_filename(path);
					}
					if (bflag){
						byte_offset(fs, tline);
					}
					if (!qflag)
						printf("%s", tline);
				}
				else if(cflag){
					vccnt++;
				}
			}
			continue;
		}
		else if (!vflag && cflag){
			ccnt++;
		}
		prevpos = 0;
		pos = 0;
		if (!vflag && !cflag){
			if(!qflag && nl++)
				printf("\n");
			if (fileflag){
				print_filename(path);
			}
			if (bflag){
				byte_offset(fs, tline);
			}
			while(ptr){
				pos = ptr - tline;
				/*For printing unmatched characters in matched line*/
				if (iflag){
					substr = (char *)icase(substr, tline, ptr);
				}

				for (int i = prevpos; i < pos; i++){
					if (!qflag)
						printf("%c", tline[i]);
				}
				/*For printing matched characters*/
				for(int i = pos; i < pos + strlen(substr); i++){
					if (!qflag)
						printf("%s%c%s",TRED, tline[i], TNRM);
				}
				if(iflag)
					ptr = strcasestr(ptr +1, substr);  
				else
					ptr = strstr(ptr +1, substr);  
				prevpos = pos + strlen(substr);
			}
			pos = pos + strlen(substr);
			for (int i = pos; i < strlen(tline); i++){
				if (!qflag)
					printf("%c", tline[i]);
			}
		}

		mcnt++;
	}	
	if (vflag && cflag){
		if (fileflag){
			print_filename(path);
		}
		if (!qflag)
			printf("%d", vccnt);
		if (!qflag)
			printf("\n");
	}
	if (!vflag && cflag){
		if (fileflag){
			print_filename(path);
		}
		if (!qflag)
			printf("%d", ccnt);
		if (!qflag)
			printf("\n");
	}
	if(tline)
		free(tline);

}
void search(char *path, const char *substr){
	if (rflag || wflag || (wflag && cflag) || (wflag && vflag)){
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		FILE *fs;
		fs = fopen(path, "r");
		if (fs == NULL){
			printf("File not found");
			exit(EXIT_FAILURE);
		}
		/*For -wmv and -wvcm*/
		int mcnt = 0;
		int mvcnt = 0;

		char *token = NULL, *saveptr = NULL, *chk = NULL;
		int len_str = strlen(substr);
		char *ptr = NULL, *x = NULL;
		int exist, enter = 0;
		int ccnt = 0;
		char *tline = NULL;
		int vccnt = 0;
		while ((read = getline(&line, &len, fs)) != -1) {
			modify(line, read);

			if (mnum == 0){
				break;
			}
			if (mflag && mcnt == mnum){
				break;
			}

			if(tline){
				free(tline);
			}
			tline = (char *)malloc(read);
			if(!tline){
				printf("Error! memory not allocated.");
				exit(EXIT_FAILURE);
			}
			strcpy(tline, line);

			if (iflag)
				ptr = strcasestr(tline, substr);
			else
				ptr = strstr(tline, substr);
			
			if (ptr == NULL){
				if (mflag){
					if (mvcnt == mnum && ( (vflag && cflag) || (vflag && !cflag ))){
						break;
					}
					mvcnt++;
				}
				vccnt++;
				if (vflag && !cflag){

					if(!qflag && nl++)
						printf("\n");
					if (fileflag){
						print_filename(path);
					}	
					if (bflag){
						byte_offset(fs, tline);
					}
					if (!qflag)
						printf("%s", line);
				}
				continue;
			}
			while(ptr){
				if (x){
					free(x);
				}
				x = (char *)malloc(strlen(ptr));
				strcpy(x, ptr);
				chk = strtok(x, " ");
				enter = check(chk, substr);
				if (valid(tline[ptr - tline - 1])){
					enter = 0;
				}
				if (enter == 1){
					if (iflag){
						substr = (char *)icase(substr, tline, ptr);
					}
					/*for -rw*/
					if (!cflag && !vflag){
						if(!qflag && nl++)
							printf("\n");
						if (fileflag){
							print_filename((char *)path);
						}
						if (bflag){
							byte_offset(fs, tline);
						}
					}
					break;
				}
				if (iflag)
					ptr = strcasestr(ptr + 1, substr);
				else
					ptr = strstr(ptr + 1, substr);
			}
			if (enter == 0){
				if (mflag){
					if (mvcnt == mnum && ( (vflag && cflag) || (vflag && !cflag ))){
						break;
					}
					mvcnt++;
				}
				vccnt++;
				if (vflag && !cflag){
					if(!qflag && nl++)
						printf("\n");
					if (fileflag){
						print_filename(path);
					}	
					if (bflag){
						byte_offset(fs, tline);
					}
					if (!qflag)
						printf("%s", line);

				}
				continue;
			}
			exist = 0;
			token = strtok_r(line, " ", &saveptr);
			char  *tab_token = NULL;

			while(token != NULL){

				tab_token = strtok(token, "\t");
				UNUSED(tab_token);
				
				if (check(token, substr)){
					if (strlen(token) > len_str){
						exist = 1;
						if (!cflag && !vflag){
							char *buffer;
							buffer = excep(token, substr);
							if (!qflag)
								printf("%s", buffer);
							free(buffer);
						}
					}
					else{
						exist = 1;

						if (!cflag && !vflag && !qflag){
							printf("%s%s%s", TRED, substr, TNRM);
						}
					}

				}
				else {
					if (!cflag && !vflag && !qflag)
						printf("%s", token);

				}
				token = strtok_r(NULL, " ", &saveptr);
				if (token){
					if (!cflag && !vflag && !qflag)
						printf(" ");
				}
			}
			if (exist){
				if (!cflag && !vflag && !rflag && !qflag && !eflag && !fflag)
					printf("\n");
			}

			if (cflag && exist && !vflag)
				ccnt++;

			mcnt++;
		}
		if (cflag && !vflag){

			if (fileflag){
				print_filename((char *)path);
			}
			if (!qflag)
				printf("%d", ccnt);
			if (!qflag)
				printf("\n");
		}

		if (cflag && vflag){
			if (fileflag){
				print_filename(path);
			}
			if (!qflag)
				printf("%d", vccnt);
			if (!qflag)
				printf("\n");
		}

		if (x)
			free(x);
		if(tline)
			free(tline);

	}	
}

void printline(char *line, pat_info *p, int j, int exist, int blank, const char *path, FILE *fs){
	int i = 0;
	if(hflag){
		fileflag = 0;
	}

	/*To not print for lines which do not have match*/
	if (exist == 0){
		return;
	}
	int t = 0;
	int z = 0;

	if(!qflag && nl++){
		printf("\n");
	}
	if (fileflag){
		print_filename((char *)path);
	}
	if (bflag){
		byte_offset(fs, line);
	}

	PRINT:
	while(line[i] != '\0'){
		t = 0;
		z = 0;

		while(t < j){
			z = 0;

			while(p[t].pos[z] != -1){
				if (p[t].pos[z] == i){
					if (iflag){
						int len = strlen(p[t].pat);
						int i = 0;
						while(i < len){
							p[t].pat[i] = line[p[t].pos[z] + i];
							i++;
						}
						p[t].pat[i] = '\0';
					}
					if (!qflag)
						printf("%s%s%s", TRED, p[t].pat, TNRM);
					i = i + strlen(p[t].pat);
					goto PRINT;
				}
				z++;
			}

			t++;
		}
		if (!qflag)
			printf("%c",line[i]);
		i++;
	}
}

void print_filename(char *fname){
	if (!qflag)
		printf("%s%s%s%s:%s", TMAG, fname,TNRM, TCYN, TNRM);
}
void byte_offset(FILE *fp, char *line){
	long int pos;
	long int len = strlen (line);
	if ((pos = ftell(fp)) != EOF){
		 /*Current position of file pointer found */
		pos = pos - len -1;
		if (!qflag)
			printf("%s%ld%s%s:%s", TGRN, pos,TNRM, TCYN, TNRM);	
	}
}

char *icase (const char* substr, char *tline, char *ptr){
	int a = 0;
	char *s = malloc(strlen(substr) +1);
	while(a < strlen(substr)){
		s[a] = tline[ptr - tline + a];
		a++;
	}
	s[a] = '\0';

	return s;
}

void frecursive(const char *basePath, int num, char newpath[][100]){
	struct dirent *dent;
	char path[1024];

	DIR *dir = opendir(basePath);
	static int enter = 0;
	static int i = 0;
	if (!dir){
		/*Check if single file entered*/
		if (enter == 0){
			enter = 1;
			FILE *fs;
			fs = fopen(basePath, "r");
			if (fs == NULL){
				printf("File not found\n");
				exit(EXIT_FAILURE);
			}

			strcpy(newpath[i++], basePath);

		}
		enter = 0;
		return;
	}

	while ((dent = readdir(dir)) != NULL)
	{
		enter = 1;
		if (strcmp(dent->d_name, ".") != 0 && strcmp(dent->d_name, "..") != 0)
		{

            /* Construct new path from base path */
			strcpy(path, basePath);
			if (path[strlen(path) - 1] != '/')
				strcat(path, "/");
			strcat(path, dent->d_name);

			if (!(dent->d_type == DT_DIR)){

				strcpy(newpath[i++], path);
			}
			frecursive(path, num, newpath);
		}
		else{
			if (!hflag)
				fileflag = 1;
		}
	}

	strcpy(newpath[i], "\0");
	enter = 0;
	closedir(dir);
}	
