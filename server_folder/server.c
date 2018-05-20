// Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "tp_socket.c"

char checksum(char* s)
{
	 char sum = 1;
	while (*s != 0)
	{
		sum += *s;
		s++;
	}
	return sum;
}

void create_packet(char* akc, char* dados, char* checksum, char* packet, int total_lido){
	strcpy(packet, akc);
	strcat(packet, checksum);
	memmove(packet+2, dados, total_lido);
}

void extract_packet(char* packet, char* ack, char* checksum, char* dados ){
	strncpy(ack, packet, 1);  // Be aware that strncpy does NOT null terminate
	ack[1] = '\0';
	checksum[1] = '\0';
	memmove(dados, packet+1, strlen(packet));
	memmove(dados, packet+2, strlen(packet)-1);
}

void extract_ack(char* packet, char* ack){
	strncpy(ack, packet, 1);  // Be aware that strncpy does NOT null terminate
	ack[1] = '\0';
}

void extract_checksum(char* packet, char *checksum){
	memcpy(checksum, packet+1, 1);
	checksum[1] = '\0';
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
		error("Falha ao criar o socket \n");
	}
	else if (udp_socket == -2){
		error("Falha ao estabelecer endereco (tp_build_addr) \n");
	}
	else if (udp_socket == -3){
		error("Falha de bind \n");
	}

	// From
	so_addr cliente; 
	char nome_do_arquivo[256] = { 0 };
	char pacote_com_nome[256] = { 0 };
	char ack_recebido[2] = { 0 };
	char sum_recebido[2] = { 0 };
	char sum[2] = { 0 };
	int count;

	// Inicializa temporizacao
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("Error setsockopt \n");
	}
	// Rebebe um buffer
	do{
		count = tp_recvfrom(udp_socket, pacote_com_nome, sizeof(nome_do_arquivo), &cliente);
		if (count > -1){
			extract_checksum(pacote_com_nome, sum_recebido);
			extract_packet(pacote_com_nome, ack_recebido, sum_recebido, nome_do_arquivo);
			sum[0] = checksum(nome_do_arquivo);
			sum[1] = '\0';
		}
	}while((sum[0] != sum_recebido[0]) || (count == -1));	
	//extrai a substring com o nome do arquivo, sem o ACK
	printf("Nome recebido: %s \n", nome_do_arquivo);

	// Aguardando ACK do nome do arquivo
	int tam_cabecalho = 2;
	char buffer[tam_buffer];
	char ack[] = "0";
	do {
		tp_sendto(udp_socket, ack, sizeof(ack), &cliente); // Manda ACK = 0
		printf("Aguardando ACK = 1 ....... \n");
		count = tp_recvfrom(udp_socket, buffer, sizeof(buffer), &cliente);  // Esperando 1
		extract_ack(buffer, ack_recebido);

	}while ((count == -1) || (strcmp(ack_recebido, "1") != 0) ); 
	count = -1;
	// Exibe mensagem
	printf("Cliente confirmou inicio da conexao! Comecaremos a enviar dados\n");

	// Abre o arquivo
	FILE *arq;
	arq = fopen(nome_do_arquivo, "r");
	if(arq == NULL){
		printf("Erro na abertura do arquivo.\n");
		exit(1);
	}
	int total_lido;
	int tam_dados = tam_buffer-tam_cabecalho;
	char dados[tam_dados];
	printf("ack%s \n",ack);

	//rotina stop-and-wait
	do{
		total_lido = fread(dados, 1, tam_dados, arq);
		printf("tamanho dados:%zu\n", sizeof(dados));
		printf("dados:%s\n", dados);
		sum[0] = checksum(dados);
		sum[1] = '\0';
		create_packet(ack, dados, sum, buffer, total_lido);
		printf("buffer:%s\n",buffer);
		printf("ack:%s\n", ack);
		printf("sum:%s\n", sum);

		//if (total_lido == 0){  // Descomentar essa parte para testar a temporizacao do cliente
		//	exit(1);
		//}

		if(ack[0]=='0'){
			do {
				tp_sendto(udp_socket, buffer, total_lido + tam_cabecalho, &cliente); // Manda pacote de dados 0
				printf("Aguardando ACK = %s ....... \n",ack);
				count = tp_recvfrom(udp_socket, ack_recebido, sizeof(ack_recebido), &cliente);  // Espera ACK = 0
				// Do something better here 
			}while ((count == -1) || (strcmp(ack_recebido, "0") != 0));
			//memset(ack, 0, 1);
			strcpy(ack,"1");
			count = -1;
			}
		else if(ack[0]=='1'){
			do {
				tp_sendto(udp_socket, buffer, total_lido + tam_cabecalho, &cliente); // Manda pacote de dados 1
				printf("Aguardando ACK = %s ....... \n",ack);
				count = tp_recvfrom(udp_socket, ack_recebido, sizeof(ack_recebido), &cliente);  // Espera ACK = 1
			}while ((count == -1) || (strcmp(ack_recebido, "1") != 0));
			memset(ack, 0, 1);
			strcpy(ack,"0");
			count = -1;
			}
		memset(buffer, 0, tam_buffer);
		memset(dados, 0, tam_dados);
		memset(ack_recebido, 0, 1);
	}while(total_lido != 0);

	fclose(arq);
	return 0;
}
