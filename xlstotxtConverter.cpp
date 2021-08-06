#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<time.h>
#include<vector>

#define FILENAMESIZE 256
#define MEMOSIZE 262144

using std::vector;

//Global Variables
char *userprofile=getenv("UserProfile");
char desktop[FILENAMESIZE];
vector<char *> fileNames;
vector<char *> firstdates, firsttimes, lastdates, lasttimes;

//Add GUI if you can.

void create_directory(){		//Creates a folder in user's desktop.
	sprintf(desktop, "%s\\Desktop", userprofile);
	DIR *d;
	char dirname[FILENAMESIZE];	//char dirname[]="C:\\Users\\Username\\Desktop\\TM-947SD_Data";
	sprintf(dirname, "%s\\TM-947SD_Data", desktop);
    d = opendir(dirname);
	if(d==NULL){
		if(!mkdir(dirname)) printf("Directory created\n");
		else{
			printf("Unable to create directory\n");
			exit(1);
		}
	}
	else{
		printf("Directory exists\n");
	}
}

void swap_char(char s[], char c, char k){	//Swaps any character you want in a string.
    for(size_t i = 0; s[i] != '\0'; ++i){
		if(s[i] == c){
			s[i] = k;
        }
    }
}

void delete_char(char s[], char c){			//Deletes any character you want in a string.
    size_t j = 0;
    for(size_t i = 0; s[i] != '\0'; ++i){
        if(s[i] != c){
            s[j] = s[i];
            ++j;
        }
    }
    s[j] = '\0';
}

char* rename_file(char oldname[], char *firstdate, char *firsttime, char *lastdate, char *lasttime, bool append_to_vectors){	//Renames the output file name as following: Test [date and time of first data (yyyy.mm.dd hh.mm.ss)] - [date and time of last data (yyyy.mm.dd hh.mm.ss)].
	sprintf(desktop, "%s\\Desktop", userprofile);
	char *newname=(char*) malloc(FILENAMESIZE*sizeof(char));
	swap_char(firsttime, ':', '.');
	swap_char(firstdate, '/', '.');
	swap_char(lasttime, ':', '.');
	swap_char(lastdate, '/', '.');
	sprintf(newname, "%s\\TM-947SD_Data\\Test %s %s - %s %s.txt", desktop, firstdate, firsttime, lastdate, lasttime);
	if(rename(oldname, newname) == 0){
		printf("File renamed successfully. New name of the file is %s\n", newname);
	}
	else{
		printf("Unable to rename file: %s. Please change their names manually.\n", oldname);
	}
	if(append_to_vectors){
		fileNames.push_back(newname);
		firsttimes.push_back(strdup(firsttime));
		firstdates.push_back(strdup(firstdate));
		lasttimes.push_back(strdup(lasttime));
		lastdates.push_back(strdup(lastdate));
	}
	/*else{
		free(newname);
	}*/
	return newname;
}

int compare_dates(char *date01, char *time01, char *date02, char *time02){	//Checks if the last data of a file and first data of another file are from the same test.
	struct tm datetime1, datetime2;

	datetime1.tm_year = (date01[0]-'0')*1000 + (date01[1]-'0')*100 + (date01[2]-'0')*10 + (date01[3]-'0');	//Date Registration
	datetime2.tm_year = (date02[0]-'0')*1000 + (date02[1]-'0')*100 + (date02[2]-'0')*10 + (date02[3]-'0');
	datetime1.tm_mon = (date01[5]-'0')*10 + (date01[6]-'0');
	datetime2.tm_mon = (date02[5]-'0')*10 + (date02[6]-'0');
	datetime1.tm_mday = (date01[8]-'0')*10 + (date01[9]-'0');
	datetime2.tm_mday = (date02[8]-'0')*10 + (date02[9]-'0');
	datetime1.tm_hour = (time01[0]-'0')*10 + (time01[1]-'0');	//Time Registration
	datetime2.tm_hour = (time02[0]-'0')*10 + (time02[1]-'0');
	datetime1.tm_min = (time01[3]-'0')*10 + (time01[4]-'0');
	datetime2.tm_min = (time02[3]-'0')*10 + (time02[4]-'0');
	datetime1.tm_sec = (time01[6]-'0')*10 + (time01[7]-'0');
	datetime2.tm_sec = (time02[6]-'0')*10 + (time02[7]-'0');

	//Adjusting dates
	datetime1.tm_mon--;
	datetime2.tm_mon--;
	datetime1.tm_year-=1900;
	datetime2.tm_year-=1900;

	/*char buffer[4000];
	strftime(buffer, 4000, "%Y-%m-%d %H:%M:%S", &datetime1);
	printf("## %s   ", buffer);
	strftime(buffer, 4000, "%Y-%m-%d %H:%M:%S", &datetime2);
	printf("## %s    ", buffer);*/

	time_t seconds1 = mktime(&datetime1); //seconds since 1970
	time_t seconds2 = mktime(&datetime2);

	int diff = (int) difftime(seconds1, seconds2);
	//printf("%d dakika fark var\n", diff/60);
	if(diff<5 && diff>=0){
		return 1;
	}
	else return 0;
}

void combine_files(char destination[], char source[]){	//Combine files if the data is continuous.
    FILE *fp1, *fp2;
    fp1 = fopen(destination, "ab");
    fp2 = fopen(source, "rb");

    if (!fp1 && !fp2) {
        printf("Couldn't open file\n");
				if(fp1) fclose(fp1);
				if(fp2) fclose(fp2);
        return;
    }

	fseek(fp2, 0, SEEK_END);
	int length = ftell(fp2);
	fseek(fp2, 0, SEEK_SET);
	char* buffer = (char*) malloc(length+5);
	if (buffer){
		//printf("#10 %d\n", length);
		fread(buffer, 1, length, fp2);
		fwrite(buffer, 1, length, fp1);
		free(buffer);
	}
	fclose(fp1);
	fclose(fp2);
}

int main(){
	sprintf(desktop, "%s\\Desktop", userprofile);
	FILE *infp = NULL, *outfp = NULL;
	char *buffer = NULL;
	char firstdate[FILENAMESIZE], firsttime[FILENAMESIZE], lastdate[FILENAMESIZE], lasttime[FILENAMESIZE];
	char inputFilename[FILENAMESIZE], tempFilename[FILENAMESIZE], outFilename[FILENAMESIZE];
	create_directory();
	int y=1;
	
	for(int a=1; a<=10; a++){
		for(int b=1; b<=99; b++){
			//sprintf(inputFilename, "C:\\Users\\Username\\Desktop\\TM-947SD\\TMA%02d\\TMA%02d%03d.XLS", a,a,b);
			sprintf(inputFilename, "%s\\TM-947SD\\TMA%02d\\TMA%02d%03d.XLS", desktop, a, a, b);
			infp=fopen(inputFilename, "r");
			if(infp==NULL){
			continue;
			}
			else{
				printf("Input file is found.\nFile name: %s\n", inputFilename);

				fseek(infp, 0, SEEK_END);
				int length = ftell(infp);
				fseek(infp, 0, SEEK_SET);
				
				float filesize = (float)length/(float)(1024*1024);
				printf("File size: %0.2f MB\n", filesize);

				buffer = (char*) malloc(length+5);
				if (buffer){
					fread (buffer, 1, length, infp);
					for(int i=0; i<length; i++){
					if(buffer[i]=='\0') buffer[i]=' ';
					}
				buffer[length]='\0';
				}
				
				if(infp) fclose(infp);

				char *line=buffer;
				char *nextLine;
				int lineCount=1;
				for(; ; line=nextLine+1){
					nextLine=strchr(line, '\r');
					if(nextLine==0){ 									//last line of the file
						int lineLength=strlen(line);
						if(lineLength>0) nextLine=line+lineLength-1;	//last line contains data
						else break;										//last line is empty
					}
					else{
						nextLine[0]='\0';
					}

					if(line[0]=='P'){	//Searches for the line starts with "Place"
						if(outfp) {
							fclose(outfp);
							rename_file(tempFilename, firstdate, firsttime, lastdate, lasttime, true);	//rename outfp
						}
						//sprintf(tempFilename, "C:\\Users\\Username\\Desktop\\TM-947SD_Data\\out%02d.txt", y);
						sprintf(tempFilename, "%s\\TM-947SD_Data\\out%02d.txt", desktop, y);
						outfp=fopen(tempFilename, "w");
						if( outfp == NULL){
				 			printf("Unable to open output file '%s'\n", tempFilename);
						}
						else {
				 			printf("Output file name: %s\n", tempFilename);
						}
						lineCount=1;
						y++;
					}
					else{
						char *(cells[11]);			//Array of (char pointer)s
						cells[0]=strtok(line,"\t");
						if(cells[0]==NULL) continue;
						int k;
						for(k=1; k<11; k++ ){
							cells[k]=strtok(NULL,"\t");
							if(cells[k]==NULL) break;
							cells[k][-1]='\0';
						}
						if(k<11){
							printf("Corrupted Line: %s\n", line);
							continue;
						}

						if(lineCount==1){			//Saves the first and last date and time to its memory.
							//firstdate=cells[1];
							//firsttime=cells[2];
							strcpy(firstdate, cells[1]);
							strcpy(firsttime, cells[2]);
						}
						//lastdate=cells[1];
						//lasttime=cells[2];
						strcpy(lastdate, cells[1]);
						strcpy(lasttime, cells[2]);

						//for(int k=0; k<11; k++ ) printf("%s, ", cells[k]);
						//printf("\n");
						
						if(cells[3][0]==' ') cells[3]++;	//Removes the space characters
						if(cells[5][0]==' ') cells[5]++;
						if(cells[7][0]==' ') cells[7]++;
						if(cells[9][0]==' ') cells[9]++;
						
						if(outfp){
							fprintf(outfp, "%s\t%s\t%s\t%s\t%s\t%s\n", cells[1], cells[2], cells[3], cells[5], cells[7], cells[9]);
							lineCount++;
						}
					}
				}
				free(buffer);
  			}
 		}
	}

	if(outfp){
		fclose(outfp);
		rename_file(tempFilename, firstdate, firsttime, lastdate, lasttime, true);	//rename outfp
	}

	int j=0;
	for(int i=1; i<fileNames.size() ; i++){
		if(	compare_dates(firstdates[i], firsttimes[i], lastdates[i-1], lasttimes[i-1]) == 1){
			printf("Combining the files...\n");
			combine_files(fileNames[j], fileNames[i]);
			remove(fileNames[i]);
			char *newname = rename_file(fileNames[j], firstdates[j],firsttimes[j], lastdates[i], lasttimes[i], false);
			free(fileNames[j]);
			fileNames[j] = newname;
		}
		else{
			j=i;
		}
		printf("filename: %s, %s %s %s %s\n",fileNames[i], firstdates[i], firsttimes[i], lastdates[i], lasttimes[i]);
	}
	
	printf("Press ENTER to exit the application...\n");
	getchar();
	
	return 0;
}
