#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]){

	// Comparing char 
	printf("Here we will begin comparing chars \n");
	char a = '0';
	char b = '1';
	char c = '0';
	printf("a == b: %d \n", a==b);
	printf("a == c: %d \n", a==c);
	printf("a == '0': %d \n", a=='0');
	printf("a == '1': %d \n", a=='1');
	printf("a == \"0\": %d \n", a=="0");
	printf("a == \"1\": %d \n", a=="1");
	printf("Here we see that comparing a char with a string does not work!! \n");

	// Comparing strings
	printf("\n \n \n");
	printf("Now we will see how to compare strings! \n");
	char string[] = "0";
	printf("We have done -> string[] = 0 \n");
	printf("string = %s \n", string);
	//printf("string[0] = %s", string[0]); //This here is very dangerous and cause "Segmentation fault"
	printf("string[0] = %c \n", string[0]);
	printf("string == \"0\": %d \n", string=="0");
	printf("string == \"1\": %d \n", string=="1");
	printf("string == '0': %d \n", string=='0');
	printf("string == '1': %d \n", string=='1');
	printf("Here we see that using '==' never works for strings \n");
	printf("strcmp(string, \"0\"): %d \n", strcmp(string, "0"));
	printf("strcmp(string, \"1\"): %d \n", strcmp(string, "1"));
	printf("Buut, using strcmp seens to be effective!!! \n");

	// Use case in TP
	printf("\n \n \n");
	printf("Lets check out the best way of handling this on TP \n");
	printf("We have done -> string[] = \"0\" \n");
	printf("string[0] == '0': %d \n", string[0]=='0');
	printf("string[0] == '1': %d \n", string[0]=='1');

	return 0;
}