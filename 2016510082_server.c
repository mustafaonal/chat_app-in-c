#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#define MAX 80 
#define PORT 3205

int clientCount = 0;//this hold number of client gor Client struck
int roomCount = 0;//this hold number of rooms for Room struck

struct client{

	int index;
	int sockID;
	struct sockaddr_in clientAddr;
	int len;
	char nickname[20];
	char roomname[20];
	int isClientinRoom;//this is extra for look is client in room or not, because some functions don't work if client is in room

};

struct room
{
	char name[1024];
	int isPrivate;
	char password[20];
	int owner;//hold sokID who create this room, benim programımda en son odayı yaratan çıkabiliyor ve oda o zaman kapanıyor
};

struct room Room[30]; //30 room can create most in this program
struct client Client[1024]; //at most 1024 client in this program

pthread_t thread[1024];//used pthread library in this program



void * doNetworking(void * ClientDetail)//this is function in create thread, everything almost happen there
{										//works for each client
										//bu kod atıyorum client -create yazıp enter a basınca çalışıyor ve process işliyor
										//yada -enter yazıp enter a bastıktan sonra server room ismi istiyor ve o şekilde odaya girebiliyor
										//bazı başlatmalarda kod çalışmayabiliyor, sebebini anlamadım
										//ctrl+c yapıp client.out dosyasını çalıştırırsanın tekrar çalışabilir
										//genelded düzgün çalışıyor, nickname: göründüünde yazılanı kabül ediyor




	struct client* clientDetail = (struct client*) ClientDetail; //create struck, şuan üzerinde çalışılan client bilgileri burada
	int index = clientDetail -> index;//assigned client index
	int clientSocket = clientDetail -> sockID;//çok kullanıldığı için sockID yi int bir değişkenin içine atıyoruz

	printf("Client %d connected.\n",index + 1);//servera kimin connect olduğunu yazıyor
	send(clientSocket, "Please enter your nickname:", 30, 0);//nickname için clienta mesaj atıyor
    recv(clientSocket, &clientDetail -> nickname[0], 20, 0);//nickname bilgisini clienttan alıyor
	int isClientCreatedRoom = 0;//eğer bir client oda oluşturdu ise tekrar oluşturmamasını sağlıyor
        



	while(1){

		char data[1024];//to hold data that client send or when server need to write something on server page 
		recv(clientSocket,data,1024,0);//first of all wait for recive to data from client
		

		char output[1024];//verilen dataları yazmak için list de create de kullandım, input ile aynı gibi amaçları

		if(strcmp(data,"-list") == 0){//list işlemi
			if (clientDetail ->isClientinRoom == 1)//eğer room içindeyse list yapamaz
			{
				send(clientSocket, "you can't list!!!", 20, 0);
			}else{
			

				int l = 0;

				for(int i = 0 ; i < roomCount ; i ++){

					l += snprintf(output + l,1024,"\nRoomname: %s",Room[i].name);//list yapılacak şeyleri arka arkaya output içine ekleyip yazdırıyor
					for (int j = 0; j < clientCount; j++)//room içindeki clientları bulup yazdırıyor, özel oda ise içerisindeki clientlar yazılmıyor
					{
						if (strcmp(Client[j].roomname, Room[i].name) == 0 && Room[i].isPrivate == 0)
							l += snprintf(output + l,1024,"\n    Client in: %s", Client[j].nickname);
					
					}
					if (Room[i].isPrivate == 1)
					{
						l += snprintf(output + l,1024,"     private room\n");
					}
				
				

				}

				send(clientSocket,output,1024,0);
				continue;
			}

		}else if (strcmp(data,"-create") == 0)
		{
			send(clientSocket, "Please enter room name: ", 30, 0);
			recv(clientSocket, output, 1024, 0);
			for (int i = 0; i < roomCount; i++)//aynı isimde oda var mı diye bakıyor
			{
				if(strcmp(Room[i].name, output) == 0)
				{
					send(clientSocket, "There is already room with same name!!!\n", 50, 0);
					isClientCreatedRoom = 1;//alttaki if bloğuna girmemek için
					break;
				}	

			}
			if (isClientCreatedRoom == 0)
			{
				strcpy(Room[roomCount].name, output);//client dan gelen oda ismini alıyor ve room un name değişkenine atıyor
				Room[roomCount].isPrivate = 0;//private olmadığı için 0
				Room[roomCount].owner = clientDetail ->sockID;
				printf("Room %s created by %s\n", Room[roomCount].name, clientDetail -> nickname);
				strcpy(clientDetail -> roomname, output);//clientında bu oda da olduğunu anlamak için, -msg yaparken buraya göre odadaki insanlara mesaj atılıyor
				clientDetail ->isClientinRoom = 1;
				send(clientSocket, "room created\n", 20, 0);
				
				roomCount++;
				isClientCreatedRoom = 2;//client oda oluşturdu durumu bir sonraki oda oluşturmasını engellemek için
			}else if (isClientCreatedRoom == 2)//oda oluşturmuş ve silmemiş bir client tekrar oluşturamaz
			{
				send(clientSocket, "same client can't create more than one room!!!\n", 55, 0);
			}else
				break;
			
			
	
		}else if (strcmp(data,"-exit") == 0)//hangi clienttın programdan ayrıldığını yazmak için
		{
			printf("Client %d(%s) disconnected.\n",index + 1, clientDetail -> nickname);
			
		}else if(strcmp(data,"-pcreate") == 0)//create ile aynı çalışıyor neredeyse sadece fazladan password alıyor ve isprivate değişkenini 1 yapıyor
		{
			send(clientSocket, "Please enter room name: ", 30, 0);
			recv(clientSocket, output, 1024, 0);
			for (int i = 0; i < roomCount; i++)
			{
				if(strcmp(Room[i].name, output) == 0)
				{
					send(clientSocket, "There is already room with same name!!!\n", 50, 0);
					isClientCreatedRoom = 1;
					break;
				}	

			}
			if (isClientCreatedRoom == 0)
			{
				send(clientSocket, "Please enter room password: ", 40, 0);
				recv(clientSocket, Room[roomCount].password, 15, 0);
				Room[roomCount].isPrivate = 1;
				strcpy(Room[roomCount].name, output);
				printf("Room %s created by %s\n", Room[roomCount].name, clientDetail -> nickname);
				send(clientSocket, "room created\n", 20, 0);
				strcpy(clientDetail -> roomname, output);
				clientDetail ->isClientinRoom = 1;
				roomCount++;
				isClientCreatedRoom = 2;
			}else if (isClientCreatedRoom == 2)
			{
				send(clientSocket, "same client can't create more than one room!!!\n", 55, 0);
			}else
				break;
	
		}else if(strcmp(data,"-whoami") == 0)//clientDetail değişkeni clientın her bilgisini tutduğu için nickname i oradan alıyor
		{
			send(clientSocket,clientDetail ->nickname, 20, 0);

		}else if(strcmp(data,"-enter") == 0)//eğer client bir odaya girmek istiyorsa ilk baş client dan girmek istediği odanın ismi alınıyor
		{
			send(clientSocket, "please enter room name that you want to join: ", 50, 0);
			memset(data, 0, sizeof(data));
			recv(clientSocket, data, sizeof(data), 0);
			for (int i = 0; i < roomCount; i++)//Room struct ı içerisinde istenilen odanın adı aranıyor, private oda ise clientdan password isteniyor
			{									//client ın room name ini yazıyor -msg kodunda diğerlerine mesaj atması ve alması için kontrol edilen yer
												//ve odada olma durumunu 1 yapıyor
				if(strcmp(Room[i].name, data) == 0)
				{
					if(Room[i].isPrivate == 1){
						send(clientSocket, "please enter password", 30, 0);
						memset(data, 0, sizeof(data));
						recv(clientSocket, data, sizeof(data), 0);
						if (strcmp(Room[i].password, data) == 0)
						{
							strcpy(clientDetail -> roomname, Room[i].name);
							printf("Client %s joined room %s\n", clientDetail ->nickname, Room[i].name);
							clientDetail ->isClientinRoom = 1;
							send(clientSocket,"joined", 10, 0);
						}else
							send(clientSocket,"wrong password", 20, 0);
					
					}else{
						strcpy(clientDetail -> roomname, Room[i].name);
						printf("Client %s joined room %s\n", clientDetail ->nickname, Room[i].name);
						clientDetail ->isClientinRoom = 1;
						send(clientSocket,"joined", 10, 0);
					}
				}
			}
		}else if(strcmp(data,"-quit") == 0) //clientDetail daki clientın içerisinde bulunduğu odanın ismi ile Room struct tındaki odaları karşılaştırıyor
											//ve ona göre odadan çıkıyor,eğer client odanın sahibi ise odayı kapatıyor
		{
			for(int i = 0 ; i < roomCount ; i ++){
				if (strcmp(clientDetail ->roomname, Room[i].name) == 0){
					strcpy(clientDetail ->roomname, "00");
					printf("Client %s left room %s\n", clientDetail ->nickname, Room[i].name);
					clientDetail ->isClientinRoom = 0;
					send(clientSocket, "room lefted...", 20, 0);
					if (clientDetail ->sockID == Room[i].owner){//sahibi olup olmadığınına bakıyor
						printf("Room %s closed...\n", Room[i].name);
						Room[i].isPrivate = 0;
						Room[i].owner = 0;
						for(int j = i ; j < roomCount - 1 ; j ++){//oda silinmesi gerakiyorsa Room structı içerisinde ondan sonra gelen odaları bir geri kaydırıyor
							Room[j].isPrivate = Room[j+1].isPrivate;//ve roomCount u bir azaltıyor
							Room[j].owner = Room[i+1].owner;
							strcpy(Room[j].name, Room[j+1].name);
							strcpy(Room[j].password, Room[j+1].password);
						}
						roomCount--;
						break;
						
					}
					break;
				}
			}
	

		}else if(strcmp(data,"-msg") == 0)
		{
			int l = 0;
			l += snprintf(output + l,1024,"%s: ",clientDetail ->nickname);//nickname i ve mesajı outputa yazıp gönderiyor
			send(clientSocket, "please enter your message: ", 30, 0);
			memset(data, 0, sizeof(data));
			recv(clientSocket, data, sizeof(data), 0);
			l += snprintf(output + l,1024," %s",data);
			for (int i = 0; i < clientCount; i++)
			{
				if(strcmp(clientDetail ->roomname, Client[i].roomname) == 0){//aynı odada olduğu kişilere mesaj gidiyor sadece
					send(Client[i].sockID, output, sizeof(data), 0);
				}			
			}	
		}
	}
	return NULL;
}




int main() //main parts almost samein every code, just diffarences are in infity while loops
{
	int socket_desc, len; 
	struct sockaddr_in server, client; 


	// socket create and verification 
	socket_desc = socket(AF_INET, SOCK_STREAM, 0); 
	if (socket_desc == -1) { 
		printf("socket creation failed...\n"); 
		return 1; 
	} 
	else
		printf("Socket successfully created..\n"); 

	// assign IP, PORT 
    memset(&server,0,sizeof(server));
	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = INADDR_ANY; 
	server.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(socket_desc, (struct sockaddr *)&server, sizeof(server))) != 0) { 
		printf("socket bind failed...\n"); 
		return 1; 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(socket_desc, 10)) != 0) { 
		printf("Listen failed...\n"); 
		return 1; 
	} 
	else
		printf("Server listening..\n"); 


	len = sizeof(client); 

		while(1)//there is accepted client socket and create thread and go to doNetworking function
		{		//doNetworking run infinit time till server closed 	
				//assigned client information to hold their data

			Client[clientCount].sockID = accept(socket_desc, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
			Client[clientCount].index = clientCount;

			pthread_create(&thread[clientCount], NULL, doNetworking, (void *) &Client[clientCount]);

			clientCount ++;
 
		}

	for(int i = 0 ; i < clientCount ; i ++)//to close threads which we done to work
	pthread_join(thread[i],NULL);
        
	
} 
