//File transfer server
#include "../secure_networking/secure_header.h"
#define FOLDER "PATHTODIR"

//Functions
void presentation(void);
void* thread_manager(void*);
char* get_filename(int);

int main() {

    //presenting
    presentation();

    //declaring struct info
    serverinfo s_info;
    clientinfo c_info;

    //declaring variables
    pthread_t t_id;
    int port = 55000;
    struct hostent* host;
    char ip_buffer[INET_ADDRSTRLEN];
    
    //installing EPIPE signal handler
    signal(EPIPE, SIG_IGN);

    //assigning attributes to addr
    s_info.__serveraddr.sin_family = AF_INET;
    s_info.__serveraddr.sin_port = htons(port);
    s_info.__serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_info.__serverlen = sizeof(s_info.__serveraddr);

    //creating socket
    s_info.__serverfd = secure_socket(AF_INET, SOCK_STREAM, 0);

    //binding socket
    secure_bind(s_info.__serverfd, (struct sockaddr*)&s_info.__serveraddr, s_info.__serverlen);

    //listening
    secure_listen(s_info.__serverfd, BACKLOG);

    //defining client addr size
    c_info.__clientlen = sizeof(c_info.__clientaddr);

    puts("File Transfer Server started.\n");
    puts("");
    while(ONLINE) {
        
        //accept connection
        c_info.__clientfd = secure_accept(s_info.__serverfd, (struct sockaddr*)&c_info.__clientaddr, &c_info.__clientlen);

        //get to know client:
        //clean ip buffer and fetch ip
        memset(ip_buffer, 0, INET_ADDRSTRLEN);
        secure_ntop(AF_INET, &c_info.__clientaddr.sin_addr, ip_buffer, INET_ADDRSTRLEN);
	    
        //print info
        printf("Client connected IP: %s\n", ip_buffer);
        puts("");

        //create a thread to manage the just connected client
        secure_pthread_create(&t_id, NULL, thread_manager, &c_info.__clientfd);

    }


}

//Interface
void presentation(void) {

    system("clear");
    printf("***************************\n");
    printf("*        Welcome to       *\n");
    printf("*  File Transfer Server!  *\n");
    printf("***************************\n");
    puts("");
}

//Thread Managing
void* thread_manager(void* arg) {

	//declaring variables
	int client_fd = *(int*)arg;
	int filedes, sizelen, byte_sent, remain_data;
	char* greeting = "Connected to File Transfer Server";
	char* filename;
	char opening_folder[MAXBUFSIZE*2];
	struct stat filestat;

	//cleaning
	memset(opening_folder, 0, sizeof(opening_folder));

	//send greeting
	sizelen = strlen(greeting);
	secure_send(client_fd, &sizelen, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
	secure_send(client_fd, greeting, sizelen, MSG_NOSIGNAL, SERVER, NONFILE, 0);

	//getting file descriptor
	filename = get_filename(client_fd);

	//if error occurred
	int response = SUCCESS;
	if(strcmp(filename, "OOR") == 0) {
		response = FAIL;
		//notify the client 
		secure_send(client_fd, &response, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
		//exit
		pthread_exit(NULL);
	} 
		
	//notify the client of the successful research
	secure_send(client_fd, &response, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
		
	//send filename
	sizelen = strlen(filename);
	secure_send(client_fd, &sizelen, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
	secure_send(client_fd, filename, sizelen, MSG_NOSIGNAL, SERVER, NONFILE, 0);

	//strcat folder to file
	sprintf(opening_folder, "%s%s", FOLDER, filename);

	//opening the returned file
	if((filedes = open(opening_folder, O_RDONLY, 0666)) == -1) {
		pthread_exit(NULL);
	}

	//send file size
	if(fstat(filedes, &filestat) == -1) {
		perror("stat");
		pthread_exit(NULL);
	}
		
	//getting the size
 	remain_data = filestat.st_size;
 
	//sending the file size
 	secure_send(client_fd, &remain_data, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0); 

	//sending the actual file in a loop: until all the bytes of the file has not been transfered, keep sending the bytes
	//and remove the byte sent from the remain_data (the total size of the file).
	while(((byte_sent = sendfile(client_fd, filedes, NULL, filestat.st_size)) > 0) && (remain_data > 0)) {
		remain_data -= byte_sent;
 	}
 	
 	//we're done: close the file and client descriptor and exit.
 	close(client_fd);
 	close(filedes);
 	pthread_exit(NULL);
 		
}

char* get_filename(int client_fd) {

    //declaring variables
    int dir_count = 0, sizelen = 0, response;
    char** directory_files;
    char sending_buffer[MAXBUFSIZE*5];
    DIR* directory;
    struct dirent* countdirentdir, *direntdir;

    //cleaning
    memset(sending_buffer, 0, sizeof(sending_buffer));

    //openign directory
    if((directory = opendir(FOLDER)) == NULL) {
        perror("opendir");
        close(client_fd);
        pthread_exit(NULL);
    }

    //reading directory for defining file count
    while((countdirentdir = readdir(directory)) != NULL) {
        if(!strcmp(countdirentdir->d_name, ".") || !strcmp(countdirentdir->d_name, "..")) {
		    //do nothing, we don't mind . and ..
	    } else {
            //increase the file count
            dir_count++;
        }
    }

    //alloc necessary space to memorize all the file name
    directory_files = (char**)malloc(dir_count*sizeof(char*));

    //reset counter
    dir_count = 0;

    //reopening directory
    if((directory = opendir(FOLDER)) == NULL) {
        perror("opendir");
        close(client_fd);
        pthread_exit(NULL);
    }

    //reading again 
    int file_counter = 1;
    while((direntdir = readdir(directory)) != NULL) {
        if(!strcmp(direntdir->d_name, ".") || !strcmp(direntdir->d_name, "..")) {
		    //do nothing, we don't mind . and ..
	    } else {
            //alloc space for a new string into the array at actual dir count
            directory_files[dir_count] = (char*)malloc(strlen(direntdir->d_name));

            //copy it 
            strncpy(directory_files[dir_count], direntdir->d_name, strlen(direntdir->d_name));

            //build formatted name into the buffer (+4 because we'll add 4 character more.)
            sprintf(sending_buffer, "%d. \033[32m%s\033[0m\n", file_counter, direntdir->d_name);

            //sending buffer and size
            sizelen = strlen(sending_buffer);
            secure_send(client_fd, &sizelen, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
            secure_send(client_fd, sending_buffer, sizelen, MSG_NOSIGNAL, SERVER, NONFILE, 0);

            //increase dir counter
            dir_count++;
            file_counter++;
        }
    }

    //send end of list
    memset(sending_buffer, 0, strlen(sending_buffer));
    sprintf(sending_buffer, "EOL");
    sizelen = strlen(sending_buffer);
    secure_send(client_fd, &sizelen, sizeof(int), MSG_NOSIGNAL, SERVER, NONFILE, 0);
    secure_send(client_fd, sending_buffer, sizelen, MSG_NOSIGNAL, SERVER, NONFILE, 0);

    //wait for a response 
    secure_recv(client_fd, &response, sizeof(int), 0, SERVER, NONFILE, 0);

    //return name if valid, -1 otherwise
    if(response > file_counter || response <= 0) {
    	return "OOR";
    } 
    
    return directory_files[response-1];

}

