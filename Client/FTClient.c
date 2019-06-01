//File transfer Client

//Include and defines
#include "../secure_networking/secure_header.h"
#define FOLDER "PATHTODIR"
#define FILEBUF 2048

//Functions
void presentation(void);
void scan_int(char*, int*);
void doProgress(int, int); 
void menu(void);

//variable that indicate the progress of a download
int progress = 0;

int main(int argc, char* argv[]) {
    
    //error check
    if(argc != 2) {
        fprintf(stderr, "Usage: ./FTClient <IP (0.0.0.0)>\n");
        exit(EXIT_FAILURE);
    }

    char* ip = argv[1];

    //declaring variables
    int sizelen, choice, filedes, response, c;
    char recv_buffer[MAXBUFSIZE];
    char opening_folder[MAXBUFSIZE*2];
    char* file_buffer;
    connectioninfo info;
    serverinfo s_info;

    //generating the attributes
    strncpy(info.__ip, ip, INET_ADDRSTRLEN);
    info.__port = 55000;

    //clean the folder buffer
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(opening_folder, 0, sizeof(opening_folder));

    
    while(1) {
    
    	//init the server address and creating the socket
    	secure_server_init(info, &s_info);
    	
    	//connecting
    	printf("Connecting to the server..\n");
    	secure_connect(s_info.__serverfd, (struct sockaddr*)&s_info.__serveraddr, s_info.__serverlen);
    
    	//connected, recv greeting
    	secure_recv(s_info.__serverfd, &sizelen, sizeof(int), 0, CLIENT, NONFILE, 0);
    	secure_recv(s_info.__serverfd, recv_buffer, sizelen, 0, CLIENT, NONFILE, 0);
    	printf("%s\n", recv_buffer);
    	puts("");
    	puts("");
    
    	presentation();
    	printf("Available file into the server:\n");
    
    	while(secure_recv(s_info.__serverfd, &sizelen, sizeof(int), 0, CLIENT, NONFILE, 0)) {

			//recv buffer
			memset(recv_buffer, 0, sizeof(recv_buffer));
			secure_recv(s_info.__serverfd, &recv_buffer, sizelen, 0, CLIENT, NONFILE, 0);

			//check if not EOL
			if(strcmp(recv_buffer, "EOL") == 0) {
				//end of list reached, break
				break;
			}

			//if not, print the content
			printf("%s", recv_buffer);
		   
		}

		//if here, make the file choice
		puts("");
		scan_int("Choose the file (NUM)", &choice);

		//send the choice to the server
		secure_send(s_info.__serverfd, &choice, sizeof(int), 0, CLIENT, NONFILE, 0);
		
		//receive the response
		secure_recv(s_info.__serverfd, &response, sizeof(int), 0, CLIENT, NONFILE, 0);
		
		//if the file was invalid
		if(response == FAIL) {
			//notify and skip the remaining cycle
			printf("Choice was not recognized by the server. Try again.\n");
			getchar();	
			continue;
		} 
		
		//reset the buffer
		memset(recv_buffer, 0, sizeof(recv_buffer));

		//receive the filename
		secure_recv(s_info.__serverfd, &sizelen, sizeof(int), 0, CLIENT, NONFILE, 0);
		secure_recv(s_info.__serverfd, recv_buffer, sizelen, 0, CLIENT, NONFILE, 0);

		//strcat the name to the folder
		sprintf(opening_folder, "%s%s", FOLDER, recv_buffer);

		//open a file with that name
		if((filedes = open(opening_folder, O_CREAT | O_RDWR, 0666)) == -1) {
		    perror("open");
		    exit(EXIT_FAILURE);
		}

		int total_byte, byte_recv, byte_wrote;
		//recv the file size
		secure_recv(s_info.__serverfd, &total_byte, sizeof(int), 0, CLIENT, NONFILE, 0);
		
		file_buffer = malloc(total_byte*sizeof(char));
		
		 //while all the byte are not received, take the file
		 //the clients wait forever because the server doesnt send the "connection close" error.
 		while((byte_recv = recv(s_info.__serverfd, file_buffer, total_byte, 0)) > 0) {
			if((byte_wrote = write(filedes, file_buffer, byte_recv)) <= 0) {
			perror("write");
			exit(EXIT_FAILURE);
			}

			doProgress(byte_recv, total_byte);
 			 
		} puts("");
		
		//reset the progress indicator
		progress = 0;

		//notify
		scan_int("Transfer Done. Continue? [1-Y | 0-N]", &c);
		
		if(!c) {
			printf("Exiting..\n");
			close(filedes);
			close(s_info.__serverfd);
			exit(EXIT_SUCCESS);		
		}
		
		//close the connection with that thread: we're going to inti a connection again if the client wants another file.
		close(s_info.__serverfd);
		

	}
}

//Interface
void presentation(void) {

    system("clear");
    printf("***************************\n");
    printf("*        Welcome to       *\n");
    printf("*  File Transfer Client!  *\n");
    printf("***************************\n");
    puts("");
}

void doProgress(int step, int total) {
 
 progress += step;
 printf("\rDownloaded: %dMB - Remaining: %dMB", (progress/1000)/1000, ((total-progress)/1000)/1000);
 fflush(stdout);

}

//Secure Scan
void scan_int(char* msg, int* var) {

    //declaring variables
    bool correct = false;
    int c;

	//while the int is not correct
    printf("%s: ", msg);
	    while(!correct) {
		    if(scanf("%d", var) != 1) {
			    printf("Value not correct, please try again.\n");
			    printf("%s: ", msg);
			    while((c = getchar()) != '\n' && c != EOF);
		    } else {
			    correct = true;
		    }	
    	}
}
