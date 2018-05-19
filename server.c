// Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "tp_socket.c"

void extract_packet(char* packet, char* ack, char* dados ){
	strncpy(ack, packet, 1);  // Be aware that strncpy does NOT null terminate
	ack[1] = '\0';
	memmove(dados, packet+1, strlen(packet));
}

int main(int argc, char * argv[]){
	// PROCESSANDO ARGUMENTOS DA LINHA DE COMANDO
	if(argc < 3){	
		fprintf(stderr , "Parametros faltando \n");
		printf("Formato: [porta_do_servidor], [tamanho_buffer] \n");
		exit(1);
	}
	int portno = atoi(argv[1]);
	int tam_buffer = atoi(argv[2]);

	printf("Porta do servidor: %d\n", portno);
	printf("Tamanho do buffer: %d\n", tam_buffer);

	// Inicializando TP Socket
	tp_init();

	// Criando socket udp
	int udp_socket;
	udp_socket = tp_socket(portno);
	if (udp_socket == -1){
		error("Falha ao criar o socket");
	}
	else if (udp_socket == -2){
		error("Falha ao estabelecer endereco (tp_build_addr)");
	}
	else if (udp_socket == -3){
		error("Falha de bind");
	}

	// From
	so_addr cliente; 
	char nome_do_arquivo[256];
	char nome_recebido[256];
	// Rebebe um buffer
	tp_recvfrom(udp_socket, nome_recebido, sizeof(nome_do_arquivo), &cliente);
	//extrai a substring com o nome do arquivo, sem o ACK
	memcpy(nome_do_arquivo, &nome_recebido[1], strlen(nome_recebido)-1);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("Error setsockopt");
	}

	// Aguardando ACK do nome do arquivo
	int count;
	char buffer[tam_buffer];
	char ack[1] = "0";
	do {
		tp_sendto(udp_socket, ack, sizeof(ack), &cliente); // Manda ACK = 0
		printf("Aguardando ACK = 1 ....... \n");
		count = tp_recvfrom(udp_socket, buffer, sizeof(buffer), &cliente);  // Esperando 1
	}while ((count == -1) || (buffer[0] != '1'));
	printf("count:%i\n",count);
	count = -1;
	// Exibe mensagem
	printf("Cliente confirmou inicio da conexao! \n");
	printf("Nome do arquivo: %s\n", nome_do_arquivo);
	FILE *arq;
	arq = fopen(nome_do_arquivo, "r");
	if(arq == NULL){
		printf("Erro na abertura do arquivo.\n");
		exit(1);
	}
	int total_lido;
	char ack_recebido[1];
	char aux[1]; //para manipular a concatenacao dos dados com o cabeçalho
	printf("ack%s",ack);
	//rotina stop-and-wait
	do{
		total_lido = fread(buffer, tam_buffer-1, 1, arq);
		strcpy(aux, ack); //aux = ack
		strcat(aux, buffer); //aux = ack+buffer
		strcpy(buffer, aux); //buffer = ack+buffer (cabeçalho completo)
		printf("buffer:%s\n",buffer);
		printf("ack:%s\n", ack);
		if(ack[0]=='0'){
			do {
				tp_sendto(udp_socket, buffer, sizeof(buffer), &cliente); // Manda pacote de dados 0
				printf("Aguardando ACK = %s ....... \n",ack);
				count = tp_recvfrom(udp_socket, ack_recebido, sizeof(ack_recebido), &cliente);  // Espera ACK = 0
			}while ((count == -1) || (ack_recebido[0] != '0'));
			memset(ack, 0, 1);
			strcpy(ack,"1");
			count = -1;
			}
		else if(ack[0]=='1'){
			do {
				tp_sendto(udp_socket, buffer, sizeof(buffer), &cliente); // Manda pacote de dados 1
				printf("Aguardando ACK = %s ....... \n",ack);
				count = tp_recvfrom(udp_socket, ack_recebido, sizeof(ack_recebido), &cliente);  // Espera ACK = 1
			}while ((count == -1) || (buffer[0] != '1'));
			memset(ack, 0, 1);
			strcpy(ack,"0");
			count = -1;
			}
		memset(buffer, 0, tam_buffer );
		memset(aux, 0, tam_buffer );
		memset(ack_recebido, 0, 1);
	}while(total_lido != 0);
	return 0;
	fclose(arq);
}
