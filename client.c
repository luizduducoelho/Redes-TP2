
// Client 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "tp_socket.c"

void error(const char *msg){
	perror(msg);
	exit(1);
}

int main(int argc, char **argv){
	// PROCESSANDO ARGUMENTOS DA LINHA DE COMANDO
	if(argc < 5){	
		fprintf(stderr , "Parametros faltando \n");
		printf("Formato: [host_do_servidor], [porta_do_servidor], [nome_do_arquivo], [tamanho_buffer] \n");
		exit(1);
	}

	size_t host_len = strlen(argv[1]) + 1;  
	//converte o numero de porta de string para inteiro
	int porta_do_servidor;
	porta_do_servidor = atoi(argv[2]);
	size_t filename_len = strlen(argv[3]) + 1;
	char *nome_do_servidor = calloc(host_len, sizeof (*nome_do_servidor));
	char *nome_do_arquivo = calloc(filename_len, sizeof (*nome_do_arquivo));
	int tam_buffer = atoi(argv[4]);
	//verifica se o espaço de memória foi alocado com sucesso
	if (!nome_do_servidor) {   
		fprintf (stderr, "error: virtual memory exhausted allocating 'nome_do_servidor'\n");
		return 1;
    	}
	//salva o nome do servidor (na forma de endereço IP) recbido pela linha de comando na variável nome_do_servidor
	strncpy (nome_do_servidor, argv[1], host_len-1);
	if (!nome_do_arquivo) { 
		fprintf (stderr, "error: virtual memory exhausted allocating 'nome_do_arquivo'\n");
		return 1;
    	}
	//salva o nome do arquivo recebido pela linha de comando na variável nome_do_arquivo
	strncpy (nome_do_arquivo, argv[3], filename_len-1);

	printf("Nome do servidor: %s\n", nome_do_servidor);
	printf("Porta do servidor: %d\n", porta_do_servidor);
	printf("Nome do arquivo: %s\n", nome_do_arquivo);
	printf("Tamanho do buffer: %d\n", tam_buffer);

	// Inicializando TP Socket
	tp_init();

	// Cria um socket udp
	int udp_socket;
	unsigned short porta_cliente = 9000;  // numro arbitrario para porta do cliente
	udp_socket = tp_socket(porta_cliente);
	if (udp_socket == -1){
		error("Falha ao criar o socket\n");
	}
	else if (udp_socket == -2){
		error("Falha ao estabelecer endereco (tp_build_addr)\n");
	}
	else if (udp_socket == -3){
		error("Falha de bind\n");
	}
	printf("antes de so addr\n");

	//Estabelecendo endereco de envio
	so_addr server;
	if (tp_build_addr(&server,nome_do_servidor,porta_do_servidor)< 0){
		error("Falha ao estabelecer endereco do servidor\n");
	}

	// Seta nome do arquivo
	char *nome_do_arquivo_pkg = calloc(filename_len+1, sizeof (*nome_do_arquivo_pkg));
	//nome_do_arquivo_pkg[0] = '0';
	strcpy(nome_do_arquivo_pkg, "0");
	strcat(nome_do_arquivo_pkg, nome_do_arquivo);
	printf("Nome de nome_do_arquivo_pkg: %s\n", nome_do_arquivo_pkg);

	// Realiza temporizacao para 1s
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("Error setsockopt\n");}

	// Envia nome do arquivo
	int count;
	char buffer[tam_buffer];
	do {
		printf("Envia nome_do_arquivo ......\n");
		tp_sendto(udp_socket, nome_do_arquivo_pkg, filename_len, &server);
		count = tp_recvfrom(udp_socket, buffer, tam_buffer, &server);  // Esperando ACK = 0
	}while ((count == -1) || buffer[0] != '0');
	printf("OK, server recebeu o nome do arquivo !\n");

	char ack[] = "1";
	tp_sendto(udp_socket, ack, sizeof(ack), &server); // Manda ACK = 1

	free(nome_do_arquivo);
	free(nome_do_servidor);
	free(nome_do_arquivo_pkg);

	return 0;
}
