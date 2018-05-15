// Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tp_socket.c"

int main(int argc, char * argv[]){

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

	// Buffer
	char buffer[tam_buffer];

	// From
	so_addr cliente;

	// Rebebe um buffer
	tp_recvfrom(udp_socket, buffer, tam_buffer, &cliente);

	// Exibe mensagem
	printf("Data received: %s\n", buffer);

	// Envia resposta 
	int count;
	char resp[] = "Obrigado!";
	count = tp_sendto(udp_socket, resp, sizeof(resp), &cliente);


	return 0;
}
