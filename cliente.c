#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SIZE 10
#define PORT 8080

// Função para exibir o tabuleiro
void displayBoard(char board[SIZE][SIZE]) {
    printf("  ");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", i + 1);
    }
    printf("\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%c ", 'A' + i);
        for (int j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

// Função principal do cliente
int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char board[SIZE][SIZE];
    
    // Inicializa o tabuleiro do cliente
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = '.';
        }
    }

    // Cria o socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erro ao criar o socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Endereço inválido ou não suportado\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Conexão falhou\n");
        return -1;
    }

    printf("Conectado ao servidor!\n");

    while (1) {
        // Recebe o tabuleiro do servidor
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            break;
        }

        // Exibe o tabuleiro do cliente
        printf("\nSeu tabuleiro:\n");
        displayBoard(board);

        // Solicita ao cliente para enviar uma coordenada
        printf("Digite uma coordenada para atacar (ex: A1): ");
        scanf("%s", buffer);

        send(sock, buffer, strlen(buffer), 0);

        // Recebe a resposta do servidor
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            break;
        }
    }

    close(sock);
    return 0;
}
