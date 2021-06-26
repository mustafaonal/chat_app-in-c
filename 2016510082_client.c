#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 3205 

void ClientRecieving(void * sockID){//thread içerisindeki fonksiyon sürekli çalışıyor, recive işlevi bu kod sayesinde çalışıyor
    int clientSocket = *((int *) sockID);//serverın gönderdiği mesajlar bu sayesde beliriyor

    while(1){

		char data[1024];
        recv(clientSocket,data,1024,0);
		printf("%s\n",data);
	}
}

int main() 
{ 
	int clientSocket;
	struct sockaddr_in server; 
    char buffer[1024];
    char nickname[20];//client nickname i burada, kendi ekranında bir şeyler yazarken hep başta nickname yazıyor, yazmadığı durumlarda enter a basarsanız çalışır
  

	// socket create and varification 
	clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if (clientSocket == -1) { 
		printf("Could not create socket"); 
	    return 1; 
	} 
	else
		printf("Socket successfully created..\n"); 


	// assign IP, PORT 
    memset(&server,0,sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	server.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

    pthread_t thread;
    pthread_create(&thread, NULL, ClientRecieving, (void *) &clientSocket );


    recv(clientSocket, buffer, 1024, 0);//nickname i yazmak için mesaj burada geliyor scanf ile tarıyor ve send ile nickname i server a gönderiyor
    printf("%s ", buffer);
    scanf("%s", &nickname[0]);
    send(clientSocket, nickname, strlen(nickname), 0);
    
    //bu kod -msg, -pcreate,-create,-list vs. yazıp enter a bastıktan sonra çalışıyor ve process işliyor
    //dökümandaki formatta -create room_name yazıyor ama benim kodun -create yazıp entera bastıktan sonra
    //room_name i yazınca çalışıyor
    //bazı çalışmalarda hata veriyor ve client kodu çalışmıyor, client.out dosyasını tekrar çalıştırısam-nız çalışablir
    //nickname istediği ekranda eğer imleç aynı satırda değil ise ve alt satırdaysa kapatıp tekrar çalıştırabilirsiniz

    while(1){


		char input[1024];
        printf("%s: \t", nickname);//her seferinde beliriyor
		gets(input);//veri girişini bekliyor


        if(strcmp(input, "-exit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
            send(clientSocket,input,1024,0);
			exit(1);
		}else if (strcmp(input, "-whoami") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);
            printf("your nickname is: ");
        }else if (strcmp(input, "-list") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);
        }else if (strcmp(input, "-create") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);
            gets(input);
            send(clientSocket, input, sizeof(input), 0); 
      
        }else if (strcmp(input, "-pcreate") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);
            gets(input);//oda ismini gönderiyor
            send(clientSocket, input, sizeof(input), 0); 
            memset(input, 0, sizeof(input));
            gets(input);//password u gönderiyor
            send(clientSocket, input, 15, 0);
      

        }else if (strcmp(input, "-enter") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);
            memset(input, 0, sizeof(input));
            gets(input);//girilmek istenilen odanın adını gönderiyor
            send(clientSocket,input , sizeof(input), 0);
            memset(input, 0, sizeof(input));
            gets(input);//password varsa gönderiyor
            send(clientSocket,input , sizeof(input), 0);

        }else if (strcmp(input, "-quit") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);

        }else if (strcmp(input, "-msg") == 0)
        {
            send(clientSocket,input , sizeof(input), 0);//-msg yazısını gönderiyor
            memset(input, 0, sizeof(input));
            gets(input);//yazılacak mesajı yazıyor
            send(clientSocket,input , sizeof(input), 0);//mesajı göderiyor
        }
       
	} 
	close(clientSocket); 
} 

