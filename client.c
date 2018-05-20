// Client 
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

void error(const char *msg){
	perror(msg);
	exit(1);
}

void create_packet(char* akc, char* dados, char* checksum, char* packet){
	strcpy(packet, akc);
	strcat(packet, checksum);	
	strcat(packet, dados);
}

void extract_packet(char* packet, char* ack, char* dados, int total_recebido ){
	if(total_recebido > 0){
	memmove(dados, packet+2, total_recebido-1);
}}

void extract_ack(char* packet, char* ack){
	strncpy(ack, packet, 1);  // Be aware that strncpy does NOT null terminate
	ack[1] = '\0';
}

void extract_checksum(char* packet, char *checksum){
	memcpy(checksum, packet+1, 1);
	checksum[1] = '\0';
}

void toggle_ack(char *ack){
	if (strcmp(ack, "0") == 0){
		strcpy(ack, "1");
	}
	else if (strcmp(ack, "1") == 0){
		strcpy(ack, "0");
	}
	else{
		error("Valor de ACK desconhecido");
	}
}

int bytes_to_write(int total_recebido, int tam_cabecalho){
	int write_n;
	if (total_recebido-tam_cabecalho > 0){
		write_n = total_recebido-tam_cabecalho;
	}
	else{
		write_n = 0;
	}
	return write_n;
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
	strncpy(nome_do_servidor, argv[1], host_len-1);
	if (!nome_do_arquivo) { 
		fprintf (stderr, "error: virtual memory exhausted allocating 'nome_do_arquivo'\n");
		return 1;
    	}
	//salva o nome do arquivo recebido pela linha de comando na variável nome_do_arquivo
    strncpy(nome_do_arquivo, argv[3], filename_len);

	printf("Nome do servidor: %s,\n", nome_do_servidor);
	printf("Porta do servidor: %d\n", porta_do_servidor);
	printf("Nome do arquivo: %s, %zu, %zu\n", nome_do_arquivo, sizeof(nome_do_arquivo), strlen(nome_do_arquivo));
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

	//Estabelecendo endereco de envio
	so_addr server;
	if (tp_build_addr(&server,nome_do_servidor,porta_do_servidor)< 0){
		error("Falha ao estabelecer endereco do servidor\n");
	}

	// Seta nome do arquivo
	char *nome_do_arquivo_pkg = calloc(filename_len+2, sizeof (*nome_do_arquivo_pkg));
	//strcpy(nome_do_arquivo_pkg, "0");
	//strcat(nome_do_arquivo_pkg, nome_do_arquivo);
	char ack[] = "0";
	char sum[2];
	char sum_recebido[2];
	sum[0] = checksum(nome_do_arquivo);
	sum[1] = '\0';
	create_packet(ack, nome_do_arquivo, sum, nome_do_arquivo_pkg);
	printf("Nome de nome_do_arquivo_pkg: %s\n, tamanho: %zu, strlen: %zu \n", nome_do_arquivo_pkg, sizeof(nome_do_arquivo_pkg), strlen(nome_do_arquivo_pkg));

	// Realiza temporizacao para 1s
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("Error setsockopt\n");}
	struct timeval t1;
	struct timeval t2;
	gettimeofday(&t1, NULL);
	// Envia nome do arquivo
	int total_recebido;
	char buffer[tam_buffer];
	do {
		printf("Envia nome_do_arquivo ......\n");
		tp_sendto(udp_socket, nome_do_arquivo_pkg, strlen(nome_do_arquivo_pkg)+1, &server);
		total_recebido = tp_recvfrom(udp_socket, buffer, tam_buffer, &server);  // Esperando ACK = 0
	}while ((total_recebido == -1) || buffer[0] != '0');
	printf("OK, server recebeu o nome do arquivo !\n");

	// Confirma inicio da conexão
	strcpy(ack, "1");
	tp_sendto(udp_socket, ack, sizeof(ack), &server); // Manda ACK = 1

	//abre um arquivo para salvar os dados recebidos
	printf("Nome do arquivo: %s \n", nome_do_arquivo);
	FILE *arq;
	arq = fopen(nome_do_arquivo, "w+");
	if (arq == NULL){
		printf("Problemas na criacao do arquivo");
		exit(1);	
	}
	fflush(arq);

	// Recebe os dados por Stop and Wait
	int tam_cabecalho = 2;
	int tam_dados = tam_buffer-tam_cabecalho;
	char dados[tam_dados];
	memset(dados, 0, tam_dados);
	int total_gravado; //recebe o numero em bytes do que foi gravado no arquivo em cada iteracao
	int tam_arquivo = 0; 
	char ack_esperado[] = "0";
	char ack_recebido[2];
	int max_timeouts = 10;
	int timeouts = 0;
	int write_n;

	do {
		if (strcmp(ack_esperado, "0") == 0){
			do{
				total_recebido = tp_recvfrom(udp_socket, buffer, tam_buffer, &server);  // Esperando ACK = 0
				printf("totalrecebido:%i\n", total_recebido);
				extract_ack(buffer, ack_recebido);
				extract_checksum(buffer, sum_recebido);
				extract_packet(buffer, ack_recebido, dados, total_recebido);
				sum[0] = checksum(dados);
				sum[1] = '\0';
				printf("sum:%s", sum);
				printf("Aguardando ACK=0, ack_recebido=%s \n", ack_recebido);
				if ((strcmp(ack_recebido, "0") == 0) && (strcmp(sum, sum_recebido) == 0)){
					tp_sendto(udp_socket, "0", sizeof("0"), &server); // Manda ACK = 0
					printf("Recebi corretamente : %s \n", buffer);
					timeouts = 0;
				}
				else{
					timeouts++;
					}
			}while(((total_recebido == -1) || (strcmp(ack_recebido, "0") != 0)) && timeouts<=max_timeouts);
		}

		else if(strcmp(ack_esperado, "1") == 0){
			do{
			total_recebido = tp_recvfrom(udp_socket, buffer, tam_buffer, &server);  // Esperando ACK = 1
			printf("totalrecebido:%i\n", total_recebido);
			extract_ack(buffer, ack_recebido);
			extract_checksum(buffer, sum_recebido);
			extract_packet(buffer, ack_recebido, dados, total_recebido);
			sum[0] = checksum(dados);
			sum[1] = '\0';
			printf("sum:%s\n", sum);
			printf("Aguardando ACK=1, ack_recebido=%s \n", ack_recebido);
			if ((strcmp(ack_recebido, "1") == 0) && (strcmp(sum, sum_recebido) == 0)){
				tp_sendto(udp_socket, "1", sizeof("1"), &server); // Manda ACK = 1
				printf("Recebi corretamente : %s \n", buffer);
			}
			else{
				timeouts++;
			}
			}while(((total_recebido == -1) || (strcmp(ack_recebido, "1") != 0)) &&  timeouts<=max_timeouts);
		}

		printf("DADOS:%s \n", dados);
		fflush(arq);
		write_n = bytes_to_write(total_recebido, tam_cabecalho);
		total_gravado = fwrite(dados, 1, write_n, arq);
		tam_arquivo += total_gravado;
		printf("total_gravado: %d, total_recebido-tam_cabecalho: %d \n", total_gravado, total_recebido-tam_cabecalho);

		if ((total_gravado != total_recebido-tam_cabecalho) && (total_recebido-tam_cabecalho > 0)){
			printf("Erro na escrita do arquivo.\n");
			exit(1);
		}
		memset(dados, 0, tam_dados);
		memset(ack_recebido, 0, 2);
		memset(sum_recebido, 0, 2);
		toggle_ack(ack_esperado);

	}while((total_recebido != 1) && timeouts<=max_timeouts);
	gettimeofday(&t2, NULL);
	long double tempoGasto = (t2.tv_sec - t1.tv_sec);
	printf("Foram recebidos, %d bytes, em %d segundos", tam_arquivo, tempoGasto);
	//fecha o arquivo
	fclose(arq);

	// Libera os ponteiros alocados
	free(nome_do_arquivo_pkg);
	free(nome_do_arquivo);
	free(nome_do_servidor);

	return 0;
}
