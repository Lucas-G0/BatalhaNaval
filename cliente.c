#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SIZE 10
#define PORT 8080

// Função para exibir o tabuleiro
void displayBoard(char board[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);  // Exibe os números das colunas
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%c  ", 'A' + i);  // Exibe as letras das linhas
        for (int j = 0; j < SIZE; j++) {
            printf("%c  ", board[i][j]);  // Exibe o conteúdo de cada célula
        }
        printf("\n");
    }
}

// Função principal do cliente
int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char board[SIZE][SIZE];  // Tabuleiro do cliente
    
    // Inicializa o tabuleiro do cliente com '.' (água)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = '.';  // Marca todas as células com água
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
        // Recebe o tabuleiro do servidor (do oponente)
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, board, sizeof(board));

        if (valread <= 0) {
            break;
        }

        // Exibe o tabuleiro do cliente
        printf("\nSeu tabuleiro:\n");
        displayBoard(board);

        // Exibe o tabuleiro do oponente (sem mostrar navios)
        char opponentBoard[SIZE][SIZE];  // Tabuleiro do oponente
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                // Exibe apenas marcas de ataques e não os navios
                if (board[i][j] == 'X') {
                    opponentBoard[i][j] = '.';  // Não mostrar navios
                } else {
                    opponentBoard[i][j] = board[i][j];  // Mostra os ataques
                }
            }
        }

        printf("\nTabuleiro do Oponente (sem navios):\n");
        displayBoard(opponentBoard);

        // Solicita ao cliente para enviar uma coordenada
        printf("Digite uma coordenada para atacar (ex: A1): ");
        scanf("%s", buffer);

        // Envia a coordenada para o servidor
        send(sock, buffer, strlen(buffer), 0);

        // Recebe a resposta do servidor (se foi acerto ou erro)
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            break;
        }

        // Exibe a resposta do servidor (acerto/erro)
        printf("%s\n", buffer);
    }

    close(sock);
    return 0;
}
