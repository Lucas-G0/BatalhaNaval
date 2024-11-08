#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define SIZE 10
#define PORT 8080

// Função para inicializar o tabuleiro
void initBoard(char board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = '.';
        }
    }
}

// Função para gerar a posição aleatória do navio
void placeShip(char board[SIZE][SIZE]) {
    srand(time(NULL));
    int x = rand() % SIZE;
    int y = rand() % SIZE;
    board[x][y] = 'X'; // Coloca um navio em uma posição aleatória
}

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

// Função para processar o ataque do servidor no tabuleiro do cliente
void serverAttack(char board[SIZE][SIZE], char opponentBoard[SIZE][SIZE]) {
    int x = rand() % SIZE;
    int y = rand() % SIZE;

    // Verifica se o servidor acerta um navio no tabuleiro do cliente
    if (opponentBoard[x][y] == 'X') {
        opponentBoard[x][y] = 'O';  // Marca um acerto
        printf("Servidor acertou em [%c%d]!\n", 'A' + x, y + 1);
    } else {
        opponentBoard[x][y] = 'M';  // Marca um erro
        printf("Servidor errou em [%c%d].\n", 'A' + x, y + 1);
    }
}

// Função para exibir o tabuleiro sem os navios
void displayBoardWithoutShips(char board[SIZE][SIZE], char displayBoard[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            // Se for um navio, exibe '.', caso contrário exibe o estado do ataque
            if (board[i][j] == 'X') {
                displayBoard[i][j] = '.'; // Não mostra os navios
            } else {
                displayBoard[i][j] = board[i][j]; // Exibe ataques ou marcações
            }
        }
    }
}

// Função principal do servidor
int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    char board[SIZE][SIZE];
    char opponentBoard[SIZE][SIZE];
    char displayBoardOpponent[SIZE][SIZE]; // Tabuleiro para exibição sem os navios
    
    // Inicializa tabuleiros
    initBoard(board);
    initBoard(opponentBoard);

    // Coloca navios aleatórios
    placeShip(board);
    placeShip(opponentBoard);

    // Criação do socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexão do cliente...\n");

    // Definindo uma variável socklen_t para armazenar o tamanho da estrutura
socklen_t addr_len = sizeof(address);

// A linha de accept deve ser alterada para:
if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addr_len)) < 0) {
    perror("accept failed");
    exit(EXIT_FAILURE);
}


    printf("Cliente conectado.\n");

    while (1) {
        // Envia o tabuleiro do servidor para o cliente (sem mostrar o navio)
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Tabuleiro do servidor:\n");
        send(new_socket, buffer, strlen(buffer), 0);

        // Envia o tabuleiro para o cliente
        displayBoardWithoutShips(opponentBoard, displayBoardOpponent);
        send(new_socket, displayBoardOpponent, sizeof(displayBoardOpponent), 0);

        // Recebe a coordenada do cliente
        memset(buffer, 0, sizeof(buffer));
        valread = read(new_socket, buffer, 1024);
        if (valread <= 0) {
            break;
        }

        printf("Coordenada recebida: %s\n", buffer);

        // Processa a coordenada e verifica se acertou
        int x = buffer[0] - 'A';
        int y = buffer[1] - '1';

        if (board[x][y] == 'X') {
            board[x][y] = 'O';  // Marca um acerto
            snprintf(buffer, sizeof(buffer), "Acertou!\n");
        } else {
            snprintf(buffer, sizeof(buffer), "Errou.\n");
        }

        // Envia a resposta ao cliente
        send(new_socket, buffer, strlen(buffer), 0);

        // Verifica se o jogo acabou
        int gameOver = 1;
        for (int i = 0; i < SIZE && gameOver; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == 'X') {
                    gameOver = 0;
                    break;
                }
            }
        }

        if (gameOver) {
            snprintf(buffer, sizeof(buffer), "Você venceu!\n");
            send(new_socket, buffer, strlen(buffer), 0);
            break;
        }

        // O servidor faz o seu ataque no tabuleiro do cliente
        serverAttack(board, opponentBoard);

        // Verifica se o jogo acabou após o ataque do servidor
        gameOver = 1;
        for (int i = 0; i < SIZE && gameOver; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (opponentBoard[i][j] == 'X') {
                    gameOver = 0;
                    break;
                }
            }
        }

        if (gameOver) {
            snprintf(buffer, sizeof(buffer), "Você perdeu! O servidor venceu.\n");
            send(new_socket, buffer, strlen(buffer), 0);
            break;
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
