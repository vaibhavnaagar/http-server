#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <time.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <errno.h>

#define MAX_BYTES 4096
#define MAX_CLIENTS 1000

int port = 5100;									// Default Port
int socketId;										// Server Socket ID
char *base_directory;								// Base directory of server

pid_t client_PID[MAX_CLIENTS];						// PID of connected clients


int sendErrorMessage(int socket, int status_code)
{
	char str[1024];
	char currentTime[50];
	time_t now = time(0);

	struct tm data = *gmtime(&now);
	strftime(currentTime,sizeof(currentTime),"%a, %d %b %Y %H:%M:%S %Z", &data);

	switch(status_code)
	{
		case 400: snprintf(str, sizeof(str), "HTTP/1.1 400 Bad Request\r\nContent-Length: 95\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\n<BODY><H1>400 Bad Rqeuest</H1>\n</BODY></HTML>", currentTime); 
				  printf("400 Bad Request\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 403: snprintf(str, sizeof(str), "HTTP/1.1 403 Forbidden\r\nContent-Length: 112\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\n<BODY><H1>403 Forbidden</H1><br>Permission Denied\n</BODY></HTML>", currentTime);
				  printf("403 Forbidden\n"); 
				  send(socket, str, strlen(str), 0);
				  break;

		case 404: snprintf(str, sizeof(str), "HTTP/1.1 404 Not Found\r\nContent-Length: 91\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n<BODY><H1>404 Not Found</H1>\n</BODY></HTML>", currentTime);
				  printf("404 Not Found\n"); 
				  send(socket, str, strlen(str), 0);
				  break;

		case 500: snprintf(str, sizeof(str), "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 115\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n<BODY><H1>500 Internal Server Error</H1>\n</BODY></HTML>", currentTime); 
				  printf("500 Internal Server Error\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 501: snprintf(str, sizeof(str), "HTTP/1.1 501 Not Implemented\r\nContent-Length: 103\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Implemented</TITLE></HEAD>\n<BODY><H1>501 Not Implemented</H1>\n</BODY></HTML>", currentTime); 
				  printf("501 Not Implemented\n");
				  send(socket, str, strlen(str), 0);
				  break;

		case 505: snprintf(str, sizeof(str), "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 125\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>505 HTTP Version Not Supported</TITLE></HEAD>\n<BODY><H1>505 HTTP Version Not Supported</H1>\n</BODY></HTML>", currentTime); 
				  printf("505 HTTP Version Not Supported\n");
				  send(socket, str, strlen(str), 0);
				  break;

		default:  return -1;
				  
	}

	return 1;
}


char* getContentType(char *path)
{
	char *dot = strrchr(path, '.');						// return the address of last '.' found in string
	char * extension;

	if(!dot || dot == path)
		extension = "";
	else
		extension = dot + 1; 

	if(strncmp(extension, "html", 4) == 0 || strncmp(extension, "htm", 3) == 0)
		return "text/html";
	else if(strncmp(extension, "txt", 3) == 0)
		return "text/plain";
	else if(strncmp(extension, "jpeg", 4) == 0 || strncmp(extension, "jpg", 3) == 0)
		return "image/jpeg";
	else if(strncmp(extension, "gif", 3) == 0)
		return "image/gif";
	else if(strncmp(extension, "pdf", 3) == 0)
		return "Application/pdf";
	else
		return "application/octet-stream";

} 


int sendHeaderMessage(int socket, char *head, char *media, int file_size)
{
	char keep_alive[] 	  = "\r\nConnection: keep-alive";
	char content_type[]   = "\r\nContent-Type: ";
	char content_length[] = "\r\nContent-Length: ";
	char date[] 		  = "\r\nDate: ";
	char server_name[]	  = "\r\nServer: VaibhavN/14785";
	char new_line[] 	  = "\r\n\r\n";

	char cLength[20];		
	snprintf(cLength,sizeof(cLength), "%d",file_size);		// Content Length: convert int to string 

	char currentTime[50];
	time_t now = time(0);

	struct tm data = *gmtime(&now);
	strftime(currentTime,sizeof(currentTime),"%a, %d %b %Y %H:%M:%S %Z", &data);	// Get current time

	char *header = (char*)calloc(strlen(head) + strlen(keep_alive) + strlen(content_type) + strlen(media) + strlen(content_length) + strlen(cLength) + strlen(date) + strlen(currentTime) + strlen(server_name) + strlen(new_line) + 20, sizeof(char));


	strcpy(header, head);
	strcat(header, content_type);
	strcat(header, media);	
	strcat(header, content_length);
	strcat(header, cLength);
	strcat(header, keep_alive);
	strcat(header, date);
	strcat(header, currentTime);
	strcat(header, server_name);
	strcat(header, new_line);
	
	int bytes_send = send(socket, header, strlen(header), 0);

	free(header);

	return bytes_send;
}


int sendFile(int socket, int fd, char *path)
{
	
	struct stat st;
	fstat(fd, &st);
	int file_size = st.st_size;									// Get file size

	char *mediaType = getContentType(path);						// Get media type of content

	int bytes_send = sendHeaderMessage(socket, "HTTP/1.1 200 OK", mediaType, file_size);


	if(bytes_send > 0)											// Header Message sent successfully
	{	
		bytes_send = sendfile(socket, fd, NULL, file_size);		// send file data
		
		while(bytes_send < file_size)							// If sent data less tham file size 
		{
			bytes_send = sendfile(socket, fd, NULL, file_size);	// Send again

			printf("\n\nSending File Again\n\n");
			if(bytes_send <= 0)									// Connection break;
			{
				bytes_send = sendErrorMessage(socket, 500);		// Unexpected server error
				return bytes_send;
			}	
		}
	}
	else
	{
		bytes_send = sendErrorMessage(socket, 500);				// Unexpected server error
		return bytes_send;
	}

	printf("Total bytes Sent : %d\n", bytes_send);

	return bytes_send;
}


int sendDirectory(int socket, char *path, char *dir_path)
{
	DIR *dir;
	struct dirent *entry;

	char buffer[MAX_BYTES];

	dir = opendir(path);											// Open directory

	int bytes_send;

	int contentLength = 0;

	if(strncmp(&dir_path[strlen(dir_path) -1], "/", 1) == 0)		// Removes Last forward slash
				strcpy(&dir_path[strlen(dir_path) -1], "\0");

	if(dir != NULL)
	{
		//----------------------------Calulate length of message to be send ---------------------------------
		while((entry = readdir(dir)) != NULL)
		{
			if(strcmp(entry->d_name, ".") == 0) continue;
			contentLength += strlen(dir_path) + 2*strlen(entry->d_name) + 25;	// Calculated
		}
		contentLength += 110 + strlen(dir_path);
		closedir(dir);
		//---------------------------------------------------------------------------------------------------

		dir = opendir(path);
		bytes_send = sendHeaderMessage(socket, "HTTP/1.1 200 OK", "text/html", contentLength);		

		if(bytes_send > 0)													// Header message sent successfully
		{

			snprintf(buffer,sizeof(buffer),"<HTML><HEAD><TITLE>Directory Links</TITLE></HEAD><BODY><H1>Files in the directory %s</H1><ul>", dir_path);
			
			bytes_send = send(socket, buffer, strlen(buffer), 0);

			if(bytes_send > 0)
			{
				while((entry = readdir(dir)) != NULL)
				{
					if(strcmp(entry->d_name, ".") == 0) continue;

					bzero(buffer,MAX_BYTES);

					snprintf(buffer,sizeof(buffer), "<li><a href=\"%s/%s\">%s</a></li>",dir_path, entry->d_name, entry->d_name);

					bytes_send = send(socket, buffer, strlen(buffer), 0);		// Send files one by one

					if(bytes_send <= 0)											// Connection is broken
						break;
				}
			}
			else
			{
				bytes_send = sendErrorMessage(socket, 500);						// Unexpected Error
				return bytes_send;
			}

			bzero(buffer,MAX_BYTES);
			
			snprintf(buffer,sizeof(buffer), "</ul></BODY></HTML>");			
			bytes_send = send(socket, buffer, strlen(buffer), 0);

			closedir(dir);														// Close dir

			return bytes_send;
		}
		else
		{
			closedir(dir);
			bytes_send = sendErrorMessage(socket, 500);							// Unexpected server error
			return bytes_send;
		}
	}
	else
	{
		if( errno == EACCES)												//  Check errno value
		{
			perror("Permission Denied\n");
			bytes_send = sendErrorMessage(socket, 403);
			return bytes_send;
		}
		else
		{
			perror("Directory Not Found\n");
			bytes_send = sendErrorMessage(socket, 404);						// Directory Not Found
			return bytes_send;
		}
	}
	
}


int checkHTTPversion(char *msg)
{
	int version = -1;

	if(strncmp(msg, "HTTP/1.1", 8) == 0)
	{
		version = 1;
	}
	else if(strncmp(msg, "HTTP/1.0", 8) == 0)				// Server can also handle 1.0 requests in the same way as it does to handle 1.1 requests  
	{
		version = 1;										// Hence setting same version as 1.1
	}
	else
		version = -1;

	return version;
}


int requestType(char *msg)
{	
	int type = -1;

	if(strncmp(msg, "GET\0",4) == 0)
		type = 1;
	else if(strncmp(msg, "POST\0",5) == 0)
		type = 2;
	else if(strncmp(msg, "HEAD\0",5) == 0)
		type = 3;
	else
		type = -1;

	return type;
}


int handleGETrequest(int socket, char *msg)
{
	char file_path[500];
	char dir_path[500];
	bzero(dir_path,sizeof(dir_path));
	bzero(file_path,sizeof(file_path));

	int fd;												// File descriptor

	int bytes_send;
	
	if(strlen(msg) == 0 || strncmp(msg, "/", 1) !=0) 	// Error
	{
		printf("message Error!");
		sendErrorMessage(socket, 400);					// 400 Bad Request
		return 1;
	}

	if(strlen(msg) == 1)								// Default file open index.html
	{
		strcpy(file_path, base_directory);
		strcat(file_path, "/index.html");
	}
	else
	{
		strcpy(file_path, base_directory);				// concatenate requested file name in base_directory
		strcat(file_path, msg);
		strcpy(dir_path, msg);
	}

	struct stat s;
	if( (stat(file_path, &s) == 0 && S_ISDIR(s.st_mode)) )		// Given File Path is a directory
	{
		printf("Send directory links\n");
		bytes_send = sendDirectory(socket, file_path, dir_path);		// Send directory links

		return bytes_send;
	}

	fd = open(file_path, O_RDONLY);							// Otherwise open requested file

	if(fd == -1)
	{
		if( errno == EACCES)
		{
			perror("Permission Denied\n");
			sendErrorMessage(socket, 403);					// Permission Denied
			return 1;
		}
		else
		{
			perror("File does not exist\n");
			sendErrorMessage(socket, 404);					// File not found
			return 1;
		}
	}

	bytes_send = sendFile(socket, fd, file_path);			// Send file content
	
	close(fd);												// Close file

	return bytes_send;
	
}



void respondClient(int socket)
{

	int bytes_send;													// Bytes Transferred

	char buffer[MAX_BYTES];											// Creating buffer of 4kb for a client
	
	bzero(buffer, MAX_BYTES);										// Make buffer zero

	bytes_send = recv(socket, buffer, MAX_BYTES, 0);				// Receive File Name

	while(bytes_send > 0)
	{
		//printf("%s\n",buffer);
		char *message[3];

		if(strlen(buffer) > 0)
		{
			message[0] = strtok(buffer, " \t\n");					// stores Request Method

			int type = requestType(message[0]);
			if(type == 1)											// GET Request
			{	
				
				message[1] = strtok(NULL, " \t\n");					// stores request file path
				message[2] = strtok(NULL, " \t\n");					// stores HTTP version

				if(strlen(message[2]) && checkHTTPversion(message[2]) == 1)	
					bytes_send = handleGETrequest(socket, message[1]);		// Handle GET request
				
				else
					sendErrorMessage(socket, 505);					// Incorrect HTTP version

			}
			else if(type == 2)										// POST Request
			{
				printf("POST: Not implemented");
				sendErrorMessage(socket, 501);
			}
			else if(type == 3)										// HEAD Request
			{
				printf("HEAD: Not implemented");
				sendErrorMessage(socket, 501);
			}
			else													// Unknown Method Request
			{
				printf("Unknown Method: Not implemented");
				sendErrorMessage(socket, 501);
			}
		}
		else
		{
			printf("ERROR\n");
			sendErrorMessage(socket, 400);							// 400 Bad Request
		}
		
		bzero(buffer, MAX_BYTES);
		bytes_send = recv(socket, buffer, sizeof(buffer), 0);		// Recieve Next Request from Cliemt

	}

	if( bytes_send < 0)
	{	
		perror("Error in receiving from client.\n");
	}
	else if(bytes_send == 0)
	{
		printf("Client disconnected!\n");
	}

	close(socket);													// Close socket

	return;
}


int findAvailableChild(int i)
{
	int j = i;
	pid_t ret_pid;
	int child_state; 

	do
	{
		if(client_PID[j] == 0)
			return j;
		else
		{
			ret_pid = waitpid(client_PID[j], &child_state, WNOHANG);		// Finds status change of pid 

			if(ret_pid == client_PID[j])									// Child exited
			{
				client_PID[j] = 0;
				return j;
			}
			else if(ret_pid == 0)											// Child is still running 
			{
				;	
			}
			else
				perror("Error in waitpid call\n");
		}
		j = (j+1)%MAX_CLIENTS;
	}
	while(j != i);

	return -1;
}


int main(int argc, char *argv[])
{
	int newSocket, client_len;		

	struct sockaddr_in server_addr, client_addr;

	base_directory = (char*)malloc(45*sizeof(char));
	char *temp_directory;

	strcpy(base_directory, "webfiles");	// Need to be changed accordingly

	bzero(client_PID, MAX_CLIENTS);

	// Fetching Arguments----------------------------------------------------------------------------------
	int params = 1;

	for(; params < argc; params++) 
	{
		if(strcmp(argv[params], "-p") == 0)
		{
			params++;

			if(params < argc)
			{
				port = atoi(argv[params]);
				continue;	
			}
			else
			{
				printf("Wrong Arguments! Usage: %s [-p PortNumber] [-b BaseDirectory]\n", argv[0]);
				exit(1);
			}	
		}
		else if(strcmp(argv[params], "-b") == 0)
		{
			params++;

			if(params < argc)
			{
				struct stat s;
				if( !(stat(argv[params], &s) == 0 && S_ISDIR(s.st_mode)))
				{
					printf("Error: No such directory exist!\n");
					exit(1);
				}

				temp_directory = argv[params];

				int k = strlen(temp_directory) - 1;

				if(strncmp(&temp_directory[k], "/", 1) == 0)					// Removing / from the last
					strcpy(&temp_directory[k], "\0");

				char *temp = (char*)realloc(base_directory, sizeof(char)*strlen(temp_directory));
				base_directory = temp;
				strcpy(base_directory, temp_directory);

				continue;	
			}
			else
			{
				printf("Wrong Arguments! Usage: %s [-p PortNumber] [-b BaseDirectory]\n", argv[0]);
				exit(1);
			}
		}
		else
		{
			printf("Wrong Arguments! Usage: %s [-p PortNumber] [-b BaseDirectory]\n", argv[0]);
			exit(1);
		}
	}

	printf("Setting Server Port : %d and Base Directory: %s\n", port, base_directory);


	// Creating socket------------------------------------------------------------------------------------- 

	socketId = socket(AF_INET, SOCK_STREAM, 0);	

	if( socketId < 0)
	{
		perror("Error in Creating Socket.\n");
		exit(1);
	}	

	int reuse =1;
	if (setsockopt(socketId, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");

	//----------------------------------------------------------------------------------------------------

	// Binding socket with given port number and server is set to connect with any ip address-------------
 
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	
	if( bind(socketId, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 )
	{
		perror("Binding Error : Port may not be free. Try Using diffrent port number.\n");
		exit(1);
	}

	printf("Binding successful on port: %d\n",port);

	//-----------------------------------------------------------------------------------------------------
 	
	// Listening for connections and accept upto MAX_CLIENTS in queue--------------------------------------

	int status = listen(socketId, MAX_CLIENTS);

	if(status < 0 )
	{
		perror("Error in Listening !\n");
		exit(1);	
	}

	//-----------------------------------------------------------------------------------------------------

	// Infinite Loop for accepting connections-------------------------------------------------------------

	int i=0;
	int ret;

	while(1)
	{	
		printf("Listening for a client to connect!\n");
		bzero((char*)&client_addr, sizeof(client_addr));							// Clears struct client_addr
		client_len = sizeof(client_addr);

		newSocket = accept(socketId, (struct sockaddr*)&client_addr, &client_len);	// Accepts connection
		if(newSocket < 0)
		{
			fprintf(stderr, "Error in Accepting connection !\n");
			exit(1);	
		}
		
				
		// Getting IP address and port number of client

		struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
		struct in_addr ip_addr = client_pt->sin_addr;
		char str[INET_ADDRSTRLEN];										// INET_ADDRSTRLEN: Deafult ip address size 
		inet_ntop( AF_INET, &ip_addr, str, INET_ADDRSTRLEN );	
		printf("New Client connected with port no.: %d and ip address: %s \n",ntohs(client_addr.sin_port), str);
	
		
		//------------------------------------------------------------------------------------------------
		// Forks new client 

		i = findAvailableChild(i);									

		if(i>= 0 && i < MAX_CLIENTS)
		{
			ret = fork();

			if(ret == 0)									// Create child process									
			{
				respondClient(newSocket);
				printf("Child %d closed\n", i);
				exit(0);									// Child exits
			}
			else
			{
				printf("----------------------------\nChild %d Created with PID = %d\n--------------------------\n", i,ret);
				client_PID[i] = ret;

			}
		}
		else
		{
			i = 0;
			close(newSocket);
			printf("No more Client can connect!\n");
		}

		
		// And goes back to listen again for another client
	}

	close(socketId);									// Close socket
    return 0;
}
