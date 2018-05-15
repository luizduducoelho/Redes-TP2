// Client 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	udp_socket = tp_socket(2000);
	if (udp_socket == -1){
		error("Falha ao criar o socket");
	}
	else if (udp_socket == -2){
		error("Falha ao estabelecer endereco (tp_build_addr)");
	}
	else if (udp_socket == -3){
		error("Falha de bind");
	}

	//Estabelecendo endereco de envio
	so_addr server;
	if (tp_build_addr(&server,nome_do_servidor,porta_do_servidor)< 0){
		error("Falha ao estabelecer endereco do servidor");
	}

	// Envia um buffer 
	int count;
	count = tp_sendto(udp_socket, nome_do_arquivo, filename_len, &server);

	// Buffer
	char buffer[tam_buffer];
	// Recebe a resposta
	tp_recvfrom(udp_socket, buffer, tam_buffer, &server);

	// Exibe mensagem
	printf("Data received: %s\n", buffer);



	return 0;
}