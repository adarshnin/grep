#include "head.h"
int noflag = 0, cflag = 0, vflag = 0, iflag = 0, wflag = 0, Hflag = 0, fflag = 0, rflag = 0, hflag = 0, bflag = 0, mflag = 0, mnum = -1, fileflag = 0, eflag = 0, qflag = 0;
/*For newline*/
int nl = 0;
int main(int argc, char *argv[])
{
	if (argc == 1){
		printf("Usage: grep [OPTION]... PATTERNS [FILE]...\nTry 'grep --help' for more information.\n");
		goto END;
	}

	FILE *fs;
	const char *substr;

	/*To store lines*/
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	opterr = 0;	
	int option;
	while((option = getopt(argc, argv, "icvwfrhHhmbeq")) != -1){ 
		switch(option){
			case 'i'	:
			iflag++;
			break;
			case 'v':
			vflag++;
			break;
			case 'c':
			cflag++;
			break;
			case 'w':
			wflag++;
			break;
			case 'r':
			rflag++;
			break;
			case 'H':
			fileflag = 1;
			Hflag++;
			break;
			case 'h':
			fileflag = 0;
			hflag++;
			break;
			case 'b':
			bflag++;
			break;
			case 'm':
			mflag++;
			break;
			case 'q':
			qflag++;
			break;
			case 'f':
			fflag++;
			break;
			case 'e':
			eflag++;
			break;
			case '?': 
	        /*used for some unknown options*/
			printf("unknown option: %c\n", option);
			break;
		}
	}

	
/*-----------------------------------------------------------------------------------------------------------*/

	int x = 0;
	const char *path[20];
	int j = 0; 
	int enter = 0;
	if (optind < argc)
	{
		do
		{
			char *object = argv[optind];
			if (eflag || fflag){
				/*For -m flag combination*/
				static int y = 0;
				if (mflag && j==0 && !y){
					mnum = atoi(object);
					enter = 1;
					y = 1;
					continue;
				}
				path[j++] = object;
				continue;
			}
			if (mflag && !enter){
				mnum = atoi(object);
				enter = 1;
				continue;
			}
			if(x++ == 0) {
				substr = object;
			}
			else{
				path[j] = object;
				if (rflag){
					j++;
					continue;
				}
				j++;

			}
		}
		while ( ++optind < argc);
	}

	/*j gives the total number of arguments*/
	if (j > 1 && !hflag){
		fileflag = 1;
	}

/*-----------------------------------------------------------------------------------------------------------*/
	if (fflag){
		goto FPAT;
	}
	if (eflag){
		goto EPAT;
	}	
	/*grep without options*/
	if (optind == 1 || ( (Hflag || hflag) && (!bflag && !cflag && !vflag && !wflag && !iflag && !mflag && !rflag && !qflag)))
		noflag++;
/*-----------------------------------------------------------------------------------------------------------*/
	if (noflag){

		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");	
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	if (rflag){

		for (int i = 0; i < j; i++){
			recursive(path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -f*/
	/*fp is pattern file*/
	/*fs is search file*/
	FPAT:
	if (fflag){
		if (j - fflag > 1 && !hflag)
			fileflag = 1;
		else if(!Hflag)
			fileflag = 0;


		int q = j;
		int t = 0;
		int i = 0;
		int size = 10;

		/*Structure to store patterns and their indices in the line*/
		pat_info *p = (pat_info *)malloc(sizeof(pat_info)* size);
		for (t = 0; t < fflag; t++){

			/*Assign number of files to a diff variable*/
			FILE *fp;
			fp = fopen(path[t], "r");
			if (fp == NULL){
				printf("grep : %s No such file or directory\n", path[t]);
				exit(EXIT_FAILURE);
			}

			char *linep = NULL;	
			size_t lenp = 0;
			ssize_t readp;

			int size = 10;
			int f;
			int repeat;

			while ((readp = getline(&linep, &lenp, fp)) != -1) {
			/*For handling newline character*/		
				modify(linep, readp);
				f = repeat = 0;
			/*Check if pattern repeats if it does skip*/
				while(f < i){
					if (!strcmp(linep, p[f].pat)){
						repeat = 1;
						break;
					}
					f++;
				}
				if (repeat)
					continue;
				if (i == size - 1){
					size = size * 2;
					p = (pat_info *)realloc(p, sizeof(pat_info)* size);
				}
				p[i].pat = (char *)malloc(readp);
				strcpy(p[i].pat, linep);

				i++;
			}	
		}

		char x[100][100];
		int num = fflag;
		if (rflag){
			/*For combination -rf and others*/
			frecursive(path[q -1], fflag, x);
			num = 0;
			t = 0;
			while(x[t][0] != '\0'){	
				t++;
			}
			q = t;
		}


		for (t = num; t < q; t++){
			char *ptr = NULL;
			int scnt = 0;
			/*This stores the ocuurence of each pattern in a single line*/
			int position;
			int j = 0;
			/*j gives the current pattern which is processed*/
			/*i gives the number of patterns*/
			int blank;
			/*used for empty line*/
			int exist;

			/*For -e -f combination*/
			if (fflag && eflag){

				for (int g = 0; g < eflag; g++){
					p[i].pat = (char *)malloc(strlen(path[t]) + 1);

					strcpy(p[i].pat, path[t]);
					i++;

					t = t + 1;
				}
				if (!Hflag)
					fileflag = 0;
			}


			if(rflag)
				fs = fopen(x[t], "r");
			else
				fs = fopen(path[t], "r");

			if (fs == NULL){
				printf("File not found");
				exit(EXIT_FAILURE);
			}

			int mcnt = 0;
			int mvcnt = 0;
			/*For -m*/
			if (mnum == 0){
				break;
			}

			int ccnt = 0;
			/*for wcf and cf*/
			int cvnt = 0;
			 /*for wcvf and cvf*/
			
			while ((read = getline(&line, &len, fs)) != -1) {
				if (mflag && mcnt == mnum){
					break;
				}
				if (mflag && vflag && mvcnt == mnum){
					break;
				}
				exist = 0;
				/*For handling newline character*/		
				modify(line, read);

				j = 0;

				while(i > j){
					/*This while loop runs once for each pattern*/
					scnt = 0;
					if (iflag){
						if (!strcasecmp(p[j].pat, "")){
							p[j].pos = (int *)malloc(sizeof(int));
							p[j].pos[0] = -1;
							blank = 1;
							j++;
							continue;
						}
					}
					else{
						if (!strcmp(p[j].pat, "")){
							p[j].pos = (int *)malloc(sizeof(int));
							p[j].pos[0] = -1;
							blank = 1;
							j++;
							continue;
						}
					}
					if(iflag)
						ptr = strcasestr(line, p[j].pat);
					else
						ptr = strstr(line, p[j].pat);
					if (ptr){
						if(!wflag)
							exist = 1;
						p[j].pos = (int *)malloc(sizeof(int));
					}
					else if (!ptr){
						p[j].pos = (int *)malloc(sizeof(int));
						p[j].pos[0] = -1;
						j++;
						continue;
					}
					while(ptr){
						if (scnt > 0){
							size = size + 1;
							p[j].pos = (int *)realloc(p[j].pos, sizeof(int)* size);
						}
						/*position in line*/
						position = ptr - line;
						if (wflag){
							if (!valid(line[position + strlen(p[j].pat)]) && !valid(line[position - 1])){
								p[j].pos[scnt] = position;
								scnt++;
								exist = 1;
							}
						}
						else{

							p[j].pos[scnt] = position;
							scnt++;
						}
						if (iflag)
							ptr = strcasestr(ptr + 1, p[j].pat);
						else
							ptr = strstr(ptr + 1, p[j].pat);
					}
					/*position can't be -1 used to denote end*/
					p[j].pos[scnt] = -1;
					j++;
				}
				if (exist == 1){
					ccnt ++;
					if(!vflag)
						mcnt++;
				}
				if (exist == 0){
					cvnt++;
					if (vflag)
						mvcnt++;
				}
				if (vflag && !cflag && exist == 0){
					if(!qflag && nl++)
						printf("\n");
					if (fileflag){
						if(rflag)
							print_filename((char *)x[t]);
						else
							print_filename((char *)path[t]);
					}
					if (!qflag)
						printf("%s", line);
				}
				if (!cflag && !vflag){

					if(rflag)
						printline(line, p, j, exist, blank, x[t], fs);
					else
						printline(line, p, j, exist, blank, path[t], fs);
				}

			}
			if (cflag && !vflag){
				if (fileflag){
					if(rflag)
						print_filename((char *)x[t]);
					else
						print_filename((char *)path[t]);
				}
				if (!qflag)
					printf("%d", ccnt);
				if (!qflag)
					printf("\n");
			}
			if (cflag && vflag){
				if (fileflag){
					if(rflag)
						print_filename((char *)x[t]);
					else
						print_filename((char *)path[t]);
				}
				if (!qflag)
					printf("%d", cvnt);
				if (!qflag)
					printf("\n");
			}

			fclose(fs);
		}
		if(!qflag && nl++)
			printf("\n");

		goto END;
	}

/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -w, -wc , -wv, -wcv*/
	if (wflag || (wflag && cflag) || (wflag && vflag)){
		for (int i = 0; i < j; i++){
			char *token = NULL, *saveptr = NULL, *chk = NULL;
			int len_str = strlen(substr);
			char *ptr = NULL, *x = NULL;
			int exist, enter = 0;
			int ccnt = 0;
			char *tline = NULL;
			int vccnt = 0;

			fs = fopen(path[i], "r");
			if (fs == NULL){
				printf("File not found");
				exit(EXIT_FAILURE);
			}
			int mcnt = 0;
			int mvcnt = 0;
			/*For -wmv and -wvcm*/

			if (mnum == 0){
				break;
			}
			while ((read = getline(&line, &len, fs)) != -1) {
				modify(line, read);
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

				/*Check if substr exists in the current line*/
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
							print_filename((char *)path[i]);
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
					chk = strtok(x, " \t");

					enter = check(chk, substr);

					if (valid(tline[ptr - tline - 1])){
						enter = 0;
					}

					if (enter == 1){

						if (iflag){
							substr = (char *)icase(substr, tline, ptr);
						}

						if (!cflag && !vflag){
							if (!qflag && nl++)
								printf("\n");
							if (fileflag){
								print_filename((char *)path[i]);
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
						if (mvcnt == mnum){
							break;
						}
						mvcnt++;
					}
					vccnt++;
					if (vflag && !cflag){
						if (!qflag && nl++)
							printf("\n");
						if (fileflag){
							print_filename((char *)path[i]);
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
					/*To supress compiler warnings*/

					if (check(token, substr)){
						if (strlen(token) > len_str){
							exist = 1;
							if (!cflag && !vflag){
								char *buffer;
								buffer = excep(token, substr);

								if (!qflag)
									printf("%s", buffer);
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
						if (!cflag && !vflag && !qflag){
							printf(" ");
						}
					}
				}


				if (cflag && exist && !vflag)
					ccnt++;


				mcnt++;
			}
			if (cflag && !vflag && !qflag){
				if (fileflag){
					print_filename((char *)path[i]);
				}
				printf("%d", ccnt);
				printf("\n");
			}

			if (cflag && vflag && !qflag){
				if (fileflag){
					print_filename((char *)path[i]);
				}
				printf("%d", vccnt);
				printf("\n");
			}

			if (x)
				free(x);
			if(tline)
				free(tline);
			fclose(fs);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}	

/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -c and -v*/

	if(cflag && vflag){

		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -c */

	if(cflag){

		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -v */

	if(vflag){
		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -m */

	if (mflag){
		
		for (int i = 0; i < j; i++){	
			find((char *)path[i], substr);
		}
		if (mnum != 0){
			if(!qflag && nl++)
				printf("\n");
		}
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -b */
	
	if (bflag){
		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -i*/
	
	if (iflag){
		for (int i = 0; i < j; i++){
			find((char *)path[i], substr);
		}
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}
/*-----------------------------------------------------------------------------------------------------------*/
	/*handling -e */
	
	EPAT:
	if (eflag){	
		if (j - eflag > 1 && !hflag)
			fileflag = 1;
		else if(!Hflag)
			fileflag = 0;

		int q = j;
		int t = 0;
		int i = 0;
		int size = 10;

		/*Structure to store patterns and their indices in the line*/
		pat_info *p;
		p = (pat_info *)malloc(sizeof(pat_info)* size);
		for (t = 0; t < eflag; t++){
			int size = 10;
			/*Assign number of files to a diff variable*/
			
			int f;
			int repeat;
			
			f = repeat = 0;
			/*Check if pattern repeats if it does skip*/
			
			while(f < i){
				if (!strcmp(path[t], p[f].pat)){
					repeat = 1;
					break;
				}
				f++;
			}
			if (repeat)
				continue;

			if (i == size - 1){
				size = size * 2;

				p = (pat_info *)realloc(p, sizeof(pat_info)* size);
			}
			p[i].pat = (char *)malloc(strlen(path[t]) + 1);
			strcpy(p[i].pat, path[t]);
			i++;

		}


		char x[100][100];
		int num = eflag;

		if (rflag){
			frecursive(path[q -1], eflag, x);
			num = 0;
			t = 0;
			while(x[t][0] != '\0'){
				t++;
			}
			q = t;
		}

		for (t = num; t < q; t++){

			int mcnt = 0;
			int mvcnt = 0;
			/*For -m */
			if (mnum == 0){
				break;
			}

			char *ptr = NULL;
			/*This stores the ocuurence of each pattern in a single line*/
			int scnt = 0;
			int position;
			/*j gives the current pattern which is processed*/
			int j = 0;
			/*i gives the number of patterns*/
			/*used for empty line*/
			int blank;
			int exist;

			if(rflag)
				fs = fopen(x[t], "r");
			else
				fs = fopen(path[t], "r");
			if (fs == NULL){
				printf("File not found");
				exit(EXIT_FAILURE);
			}
			int ccnt = 0;/*for wcf and cf*/
			int cvnt = 0; /*for wcvf and cvf*/

			while ((read = getline(&line, &len, fs)) != -1) {

				if (mflag && mcnt == mnum){
					break;
				}

				if (mflag && vflag && mvcnt == mnum){
					break;
				}
				exist = 0;
				/*For handling newline character*/		
				modify(line, read);
				j = 0;

				while(i > j){

					/*This while loop runs once for each pattern*/
					scnt = 0;
					if (iflag){
						if (!strcasecmp(p[j].pat, "")){
							p[j].pos = (int *)malloc(sizeof(int));
							p[j].pos[0] = -1;
							blank = 1;
							j++;
							continue;
						}
					}
					else{
						if (!strcmp(p[j].pat, "")){
							p[j].pos = (int *)malloc(sizeof(int));
							p[j].pos[0] = -1;
							blank = 1;
							j++;
							continue;
						}
					}
					if (iflag)
						ptr = strcasestr(line, p[j].pat);
					else
						ptr = strstr(line, p[j].pat);

					if (ptr){
						if (!wflag)
							exist = 1;
						p[j].pos = (int *)malloc(sizeof(int));
					}
					else if (!ptr){
						p[j].pos = (int *)malloc(sizeof(int));
						p[j].pos[0] = -1;
						j++;
						continue;
					}
					while(ptr){
						if (scnt > 0){
							size = size + 1;
							p[j].pos = (int *)realloc(p[j].pos, sizeof(int)* size);
						}
						/*position in line*/
						position = ptr - line;

						if (wflag){
							if (!valid(line[position + strlen(p[j].pat)]) && !valid(line[position - 1])){
								p[j].pos[scnt] = position;
								scnt++;
								exist = 1;
							}
						}
						else{

							p[j].pos[scnt] = position;
							scnt++;
						}
						if(iflag)
							ptr = strcasestr(ptr + 1, p[j].pat);
						else
							ptr = strstr(ptr + 1, p[j].pat);
					}
					/*position can't be -1 It is used to denote end*/
					p[j].pos[scnt] = -1;
					j++;
				}

				if (exist == 1){
					ccnt ++;
					if(!vflag)
						mcnt++;
				}
				if (exist == 0){
					cvnt++;
					if (vflag)
						mvcnt++;
				}

				if (vflag && !cflag && exist == 0){
					if(!qflag && nl++)
						printf("\n");
					if (fileflag){
						if(rflag)
							print_filename((char *)x[t]);
						else
							print_filename((char *)path[t]);
					}
					if (bflag){
						byte_offset(fs, line);
					}	
					if (!qflag)
						printf("%s", line);
				}

				if (!cflag && !vflag){
					if(rflag)
						printline(line, p, j, exist, blank, x[t], fs);
					else
						printline(line, p, j, exist, blank, path[t], fs);
				}

			}

			fclose(fs);

			if (cflag && !vflag){
				if (fileflag){
					if(rflag)
						print_filename((char *)x[t]);
					else
						print_filename((char *)path[t]);
				}
				printf("%d", ccnt);
				if (!qflag)
					printf("\n");
			}

			if (cflag && vflag ){
				if (fileflag){
					if(rflag)
						print_filename((char *)x[t]);
					else
						print_filename((char *)path[t]);
				}
				if (!qflag)
					printf("%d", cvnt);
				if (!qflag)
					printf("\n");
			}
		}
		if (p)
			free(p);
		if(!qflag && nl++)
			printf("\n");
		goto END;
	}

/*-----------------------------------------------------------------------------------------------------------*/
	END:

	if (line)
		free(line);
	
	return 0;
}