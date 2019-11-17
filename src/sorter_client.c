#include <stdio.h>
#include <stdlib.h>
#include <sorter_client.h>
#include "string.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <arpa/inet.h>

//global variables for socket connection
char * hostname;
int port;
int sock = 0, valread;
struct sockaddr_in address;
struct sockaddr_in serv_addr;
struct hostent *server;

int category;//category num retrieved from sortType
pthread_t pthreads[2000]; //array of pthreads of type pthread_t
int pnum=0; //num of pthreads in pthreads array of type pthreads_t
pthread_mutex_t plock;//lock for pthread array
pthread_mutex_t flock;//lock for csv data sending

int lines = 0;//num of lines total found in dir traversal
int csvnum = 0;
char recvd[2000];//buffer for incoming messages from server
int read_size;//size of message received

//turns user input for column sort into an int
int sortType(char * input){
    if(strcmp("color", input) == 0) return 0;
    if(strcmp("director_name", input) == 0) return 1;
    else if(strcmp("num_critic_for_reviews", input) == 0) return 2;
    else if(strcmp("duration", input) == 0) return 3;
    else if(strcmp("director_facebook_likes", input) == 0) return 4;
    else if(strcmp("actor_3_facebook_likes", input) == 0) return 5;
    else if(strcmp("actor_2_name", input) == 0) return 6;
    else if(strcmp("actor_1_facebook_likes", input) == 0) return 7;
    else if(strcmp("gross", input) == 0) return 8;
    else if(strcmp("genres", input) == 0) return 9;
    else if(strcmp("actor_1_name", input) == 0) return 10;
    else if(strcmp("movie_title", input) == 0) return 11;
    else if(strcmp("num_voted_users", input) == 0) return 12;
    else if(strcmp("cast_total_facebook_likes", input) == 0) return 13;
    else if(strcmp("actor_3_name", input) == 0) return 14;
    else if(strcmp("facenumber_in_poster", input) == 0) return 15;
    else if(strcmp("plot_keywords", input) == 0) return 16;
    else if(strcmp("movie_imdb_link", input) == 0) return 17;
    else if(strcmp("num_user_for_reviews", input) == 0) return 18;
    else if(strcmp("language", input) == 0) return 19;
    else if(strcmp("country", input) == 0) return 20;
    else if(strcmp("content_rating", input) == 0) return 21;
    else if(strcmp("budget", input) == 0) return 22;
    else if(strcmp("title_year", input) == 0) return 23;
    else if(strcmp("actor_2_facebook_likes", input) == 0) return 24;
    else if(strcmp("imdb_score", input) == 0) return 25;
    else if(strcmp("aspect_ratio", input) == 0) return 26;
    else if(strcmp("movie_facebook_likes", input) == 0) return 27;
    else
        return -1;
}

//connects two strings via '/' to build a path
char* pathConnect(char * str1, char * str2){ 
    char* result;  
    result=(char*)malloc(strlen(str1)+strlen(str2)+ 2);
    strcpy(result,str1);
    strcat(result,"/");   
    strcat(result,str2);  
    return result;  
}

//counts number of lines in file
int countlines(char * filename, char * path){
    char * inpath;
    inpath = pathConnect(path, filename);
    FILE *fp = fopen(inpath, "r");
    int ch=0;
    int lines=0;

    if (fp == NULL)
        return 0;

    while ((ch = (int)fgetc(fp)) != EOF){
        if (ch == '\n')
            lines++;
    }
    fclose(fp);
    return lines;
}

//modifies absolute path to relative path from current directory
char * moddir(char* output,char *input){
    int inputlen=strlen(input);
    char * dummy = (char *)malloc((inputlen+1)*sizeof(char));
    strcpy(dummy,input);
    char * token = (char *)malloc((inputlen+1)*sizeof(char));
    DIR *dir;
    struct dirent *entry;
    int i;
    int index=-1;
    int numtokens=0; 
    int breakinner=0;
    int count=0;
    char dirpath[64]={'.','/','\0','\0'};
    for (i=0;i<inputlen;i++){                               
        if (input[i]=='/' && i!=0 && i!=(inputlen-1)){
            numtokens++;
        }
    }
    numtokens++;
    char **list=(char **)malloc(numtokens*sizeof(char*));
    for (i=0;i<numtokens;i++){
        list[i]=(char *)malloc(20*sizeof(char));
    }
    
    if (input[0]=='/'){
        
        if (numtokens<=2){
            printf("should not access root directory: two or less tokens\n");
            //free stuff
            free(dummy);
            free(token);
            return NULL;
        }
        
        i=0;
        //while loop initializes list array
        while ((token=strsep(&dummy,"/"))!=NULL){
            if (token[0]!='\0'){
                list[i]=token;
                i++;
            }
        }
        
        dir=opendir(dirpath);
        while(1){   
            while((entry=readdir(dir))!=NULL){
                for (i=0;i<numtokens;i++){
                    if (strcmp(entry->d_name,list[i])==0){
                        if (i==0){//if root
                            printf("trying to access root during loop\n");
                            free(dummy);
                            free(token);
                            
                            return NULL;
                        }else{
                            index=i;
                            breakinner=1;
                            break;
                        }
                    }
                }
            }//current dir finished being scanned
            if (breakinner==1){
                break;
            }
            count++;
            if (strcmp(dirpath,"./")==0){//if dirpath is dot
                dirpath[1]='.';
                dirpath[2]='/';
            }else{
                strcat(dirpath,"../");
            }
            closedir(dir);
            dir=opendir(dirpath);
        }
        //outside both while loops
        closedir(dir);
        for (i=index;i<numtokens;i++){
            strcat(dirpath,list[i]);
            strcat(dirpath,"/");
        }
            free(dummy);
            free(token);
            for (i=0;i<strlen(dirpath)+1;i++){
                output[i]=dirpath[i];
            }
        dir=opendir(output);
        if (dir==NULL){
            closedir(dir);
            return NULL;
        }
        
        closedir(dir);
        return output;
        
        
    }else{
        
        if (input[0]!='.'){
            dirpath[0]='.';
            dirpath[1]='/';
            for (i=2;i<64;i++){
                dirpath[i]='\0';
            }
            strcat(dirpath,input);
            for (i=0;i<strlen(dirpath)+1;i++){
                output[i]=dirpath[i];
            }
        }else{
            for (i=0;i<strlen(input)+1;i++){
                output[i]=input[i];
            }
        }
        dir=opendir(output);
        if (dir==NULL){
            free(dummy);
            free(token);
            return NULL;
        }
        free(dummy);
        free(token);
        return output;//do nothing
    }
}

//checks if file is a sortable csv
int isValidCsv(char * filename, char * path){
    FILE *file;
    char cur; //current char read from first line of file
    char * ptr = strrchr(filename, '.');
    int numcommas=0;
    char inpath[1000];
    
    if(strstr(filename, "AllFiles-sorted-") != NULL) {
            return 0;
    }
    if(!ptr)
        return 0;
    else {
        ptr++;
        if (*ptr == 'c' && *(ptr+1) == 's' && *(ptr+2) == 'v' && *(ptr+3) == '\0'){
            strcpy(inpath, pathConnect(path, filename));
            file = fopen(inpath, "r");
            if(file == NULL){
                printf("Could not read file\n");
            }
            while((cur=getc(file))!='\n'&&cur!=EOF){
                if (cur==','){
                    numcommas++;
                }
            }
            fclose(file);
            if(numcommas==27){
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

char * str_replace(char *orig, char *rep, char *with) {
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL;
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}
 
void * encode(Parameters * data){
    pthread_mutex_lock(&flock);                         //lock so that csvs send data serially
    char * output_row = malloc(2000);                   //variable for encoded row
    int i;                                              //loops
    int j;                                              //loops

    char * inpath = malloc(1000*sizeof(char));          //path of file to encode
    if(inpath == NULL){
        perror("Unable to allocate inpath");
        exit(1);
    }

    //set path of file
    strcpy(inpath, pathConnect((char *)data->path, (char *)data->filename));

    //open csv file
    FILE* fp = fopen(inpath, "r");
    if(fp == NULL){
        printf("Could not open file %s.", inpath);
        return NULL;
    }
    free(inpath);

    char * buffer;            
    size_t bufsize = 2000;      



    for(i = 1; i <= (int)data->rows; i++){ 
	
        buffer = (char *)malloc(bufsize * sizeof(char));
        getline(&buffer,&bufsize,fp);
        if(buffer == NULL){
            perror("Unable to allocate buffer");
            exit(1);
        }
		
        char firstline[100];
        sprintf(firstline, ">>>%d>@>", data->rows);

		//clear output_row variable
		for(j = 0; j <= 2000; j++){
			output_row[j] = '\0';
		}
		//code for first line of file
		if(i == 1){
			strcat(output_row, firstline);
		}

		//append the buffer into output_row and replace '\n' with "@^@"
		strcat(output_row, buffer);
		int rowlength = strlen(output_row);
		output_row[rowlength-1] = '@';
		output_row[rowlength] = '^';
		output_row[rowlength+1] = '@';

		//code for last line of file
		rowlength = strlen(output_row);
		if(i == data->rows){
			output_row[rowlength] = '<';
			output_row[rowlength+1] = '<';
			output_row[rowlength+2] = '<';
		}
		
		write(sock, output_row, strlen(output_row));

		
		read_size = recv(sock, recvd, 2000, 0);
		//printf("[S??]: %s]%d\n", recvd, i);
		recvd[read_size] = '\0';
		memset(recvd, 0, 2000);	
		free(buffer);
   	} 
    pthread_mutex_unlock(&flock);

    pthread_exit(NULL);
    //return NULL;
    return 0;
}

int linestosend(char * directory){
    char * name = malloc(1000*sizeof(char));
    DIR *dir;
    struct dirent *entry;
    int i;
    char path[1000];
    int pindex;

    //set path
    strcpy(name, (char *)directory);
    strcpy(path, name);

    //open directory
    if(!(dir = opendir(name))){
        return -1;
    }

    //read entries and act accordingly based on type of entry
    while((entry = readdir(dir)) != NULL){
        //ignore '.' and '..' entries
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        //check if entry is a directory
        if(entry->d_type == DT_DIR){   
            //recursive thread to traverse found directory
            linestosend(pathConnect(path, entry->d_name));
        } 
        //check if entry is valid csv
        else if(isValidCsv(entry->d_name, path)==1){
            lines += countlines(entry->d_name, path);
            csvnum++;
        }
        else{
            continue;
        }
    }
    
    closedir(dir);
    free(name);
    return lines;
}

void * traverse(void * directory){  
    char * name = malloc(1000*sizeof(char));        //name of directory to copy into path
    DIR *dir;                                       //DIR to open for traversal
    struct dirent *entry;                           //the variable to use for testing if dir/csv
    int i;                                          //variable for loops
    char path[1000];                                //path of directory from starting directory
    int pindex;                                     //index to keep track of threads

    //set path
    strcpy(name, (char *)directory);
    strcpy(path, name);

    //open directory
    if(!(dir = opendir(name))){
        return NULL;
    }
    //printf("path is open \n");
    //read entries and act accordingly based on type of entry
    while((entry = readdir(dir)) != NULL){
        //ignore '.' and '..' entries
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        //check if entry is a directory
        if(entry->d_type == DT_DIR){       
            //printf("Found Directory: [%s]\n", entry->d_name); 

            //increment pnum
            pthread_mutex_lock(&plock);
            pindex=pnum;
            pnum++;
            pthread_mutex_unlock(&plock);

            //recursive thread to traverse found directory
            pthread_create(&pthreads[pindex], NULL, traverse, pathConnect(path, entry->d_name));
        } 
        //check if entry is valid csv
        else if(isValidCsv(entry->d_name, path)==1){
            //printf("Found CSV: [%s]\n", entry->d_name);

            //initialize parameters for the encode function
            Parameters *metadata = malloc(sizeof(Parameters));
            metadata->filename = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
            metadata->path = malloc(sizeof(char) * (strlen(path) + 1));

            //set parameters for the encode function
            strcpy((char *)(metadata->filename),entry->d_name);
            metadata->rows = countlines(entry->d_name, path);
            strcpy((char *)metadata->path,path);
            
            //increment pnum
            pthread_mutex_lock(&plock);
            pindex=pnum;
            pnum++;
            pthread_mutex_unlock(&plock);

            //thread for encoding
            pthread_create(&pthreads[pindex], NULL, (void *)encode, (void *)metadata);
            //printf("EXITTTTTTT\n");
        }
        else{
            continue;
        }
    }
    
    closedir(dir);
    free(name);
    return NULL;
}

int main(int argc, char** argv){
    char * name = malloc(200*sizeof(char));             
    char * output_name = malloc(200*sizeof(char));      //path to print file destination                                //item in current directory
 
    int i;                                              
    char * path = malloc(1000*sizeof(char));            
    
    //check num of inputs
    if (argc<7||argc>11||argc==8||argc==10){
        printf("invalid number of inputs\n");
        return -1;
    }
    switch (argc){
        case 7:
            if (strcmp(argv[1],"-c") == 0 &&strcmp(argv[3],"-h") == 0 && strcmp(argv[5],"-p") == 0){
                if ((category=sortType(argv[2]))==-1){
                    return -1;
                }
                hostname=argv[4];
                port=atoi(argv[6]);
                strcpy(path,".");
                strcpy(output_name,".");
            }else{
                printf("invalid flags\n");
                return -1;
            }
            break;
        case 9:
            if (strcmp(argv[1],"-c") == 0 &&strcmp(argv[3],"-h") == 0 && strcmp(argv[5],"-p") == 0){
                if ((category=sortType(argv[2]))==-1){
                    return -1;
                }
                hostname=argv[4];
                port=atoi(argv[6]);
                if (strcmp(argv[7],"-d")==0){//thisdir exists,no thatdir
                    if((name=moddir(name,argv[8]))!=NULL){
                        strcpy(path,name);
                        strcpy(output_name,".");
                    }else{
                        printf("invalid thisdir\n");
                        return -1;
                    }
                }else if (strcmp(argv[7],"-o")==0){//thatdir exists,no thisdir
                    if((output_name=moddir(output_name,argv[8]))==NULL){
                        printf("invalid thatdir\n");
                        return -1;
                    }
                    strcpy(path,".");
                }else{
                    printf("invalid -d or -o flags\n");
                    return -1;
                }
            }else{
                printf("bad input\n");
                return -1;
            }
            break;
        case 11:
            if (strcmp(argv[1],"-c") == 0 &&strcmp(argv[3],"-h") == 0 && strcmp(argv[5],"-p") == 0){
                if ((category=sortType(argv[2]))==-1){
                    return -1;
                }
                hostname=argv[4];
                port=atoi(argv[6]);
                if (strcmp(argv[7],"-d")==0 && strcmp(argv[9],"-o")==0){//thisdir exists,thatdir next
                    if((name=moddir(name,argv[8]))!=NULL){
                        strcpy(path,name);
                        if ((output_name=moddir(output_name,argv[10]))==NULL){
                            printf("good thisdir but bad thatdir\n");
                            return -1;
                        }
                    }else{
                        printf("invalid thisdir\n");
                        return -1;
                    }
                }else if (strcmp(argv[7],"-o")==0){//thatdir exists,thisdir next
                    if((output_name=moddir(output_name,argv[8]))!=NULL){
                        if ((name=moddir(name,argv[10]))==NULL){
                            printf("good thatdir but bad thisdir\n");
                            return -1;
                        }
                        strcpy(path,name);
                    }else{
                        printf("invalid thatdir\n");
                        return -1;
                    }
                }else{
                    printf("invalid -d or -o flags\n");
                    return -1;
                }
            }else{
                printf("bad input\n");
                return -1;
            }
            break;
            
        default:
            printf("Wrong input\n");
            return -1;
            break;
    }

    //lock initialization
    if(pthread_mutex_init(&plock, NULL)!= 0||pthread_mutex_init(&flock, NULL)!= 0){
        printf("lock init didn't work\n");
        return 1;
    }

    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    server = gethostbyname(hostname);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    char total_lines[13];
    int total_lines_int = linestosend(path);
    sprintf(total_lines, "<@>%d", total_lines_int);

    write(sock, total_lines, strlen(total_lines));



    
    read_size = recv(sock, recvd, 2000,0);
	recvd[read_size] = '\0';
	memset(recvd, 0, 2000);


    traverse(path); 
    
    //printf("Traverse Complete! \n");
    //wait for threads
    for (i=0;i<pnum;i++){
        pthread_join(pthreads[i], NULL);
    }

    //variable for column index
    char col_index[6];
    sprintf(col_index, ">@>%d", category);
  
    write(sock, col_index, strlen(col_index));
    read_size = recv(sock, recvd, 2000,0);
	//printf("[S]: %s]\n", recvd);
	recvd[read_size] = '\0';
	//clear recvd
	memset(recvd, 0, 2000);
    
    char out_filename[50];
    sprintf(out_filename, "AllFiles-sorted-%s.csv", argv[2]);
    char * outpath;
    outpath = pathConnect(output_name, out_filename);
    //printf("%s\n", outpath);
    FILE* fp = fopen(outpath, "w");
    if(fp == NULL){
        perror("Failed to write files");
    }
    fprintf(fp, "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes\n");

    for(i = 0; i < (total_lines_int-csvnum); i++){
        read_size = recv(sock, recvd, 2000,0);
        recvd[read_size] = '\0';
        fprintf(fp, "%s\n", recvd);
        //clear recvd
        memset(recvd, 0, 2000);

        write(sock, "Line received!", 15);
    }
    
    
    //printf("CSV Recieved!! #Success\n");
    fflush(fp);
    fclose(fp);


    free(name);
    free(output_name);
    free(path);
    return 0;
}
