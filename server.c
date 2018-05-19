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
	// Rebebe um buffer
	tp_recvfrom(udp_socket, nome_do_arquivo, sizeof(nome_do_arquivo), &cliente);
	char recv_ack[2];
	char arquivo_correto[15];
	extract_packet(nome_do_arquivo, recv_ack, arquivo_correto);
	printf("EXTRACTED: arquivo_correto = %s, recv_ack = %s \n", arquivo_correto, recv_ack);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("Error setsockopt");
	}

	// Aguardando ACK do nome do arquivo
	int count;
	char buffer[tam_buffer];
	char ack[] = "0";
	do {
		tp_sendto(udp_socket, ack, sizeof(ack), &cliente); // Manda ACK = 0
		printf("Aguardando ACK = 1 ....... \n");
		count = tp_recvfrom(udp_socket, buffer, sizeof(buffer), &cliente);  // Esperando ACK = 1
	}while ((count == -1) || (buffer[0] != '1'));

	// Exibe mensagem
	printf("Cliente confirmou inicio da conexao! \n");
	printf("Nome do arquivo: %s\n", nome_do_arquivo);

	return 0;
}
