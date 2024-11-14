#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define SIZE 10
#define BUFFER_SIZE (2 * SIZE * SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char serverBoard[SIZE][SIZE];
    char clientBoard[SIZE][SIZE];
    int row, col; // Para marcar a jogada
} DataPackage;


//função para inserir os navios aleatoriamente no tabuleiro
void placeShip(char board[SIZE][SIZE], int size, char direction) {
    int row, col;
    if (direction == 'V') {
        row = rand() % (SIZE - size + 1);
        col = rand() % SIZE;
        for (int i = 0; i < size; i++) {
            board[row + i][col] = 'S';
        }
    } else {
        row = rand() % SIZE;
        col = rand() % (SIZE - size + 1);
        for (int i = 0; i < size; i++) {
            board[row][col + i] = 'S';
        }
    }
}


//função para iniciar todo o tabuleiro com um valor e inserir os navios
void initBoard(char board[SIZE][SIZE], char valor, int qtShips) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = valor;
        }
    }
    for (int i = 0; i < qtShips; i++) {
        char direction = rand() % 2 == 0 ? 'V' : 'H';
        int size = rand() % 3 + 2;
        placeShip(board, size, direction);
    }
}


//função para mostrar o tabuleiro completo
void displayBoard (char board[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%2d  ", i + 1);
        for (int j = 0; j < SIZE; j++) {
            printf("%c  ", board[i][j]);
        }
        printf("\n");
    }   
}


//função para mostrar o tabuleiro sem os navios
void displayBoardWithoutShip(char matriz[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%2d  ", i + 1);
        for (int j = 0; j < SIZE; j++) {
            if (matriz[i][j] == 'S') {
                printf(".  ");
            } else {
                printf("%c  ", matriz[i][j]);
            }
        }
        printf("\n");
    }   
}


//função para marcar uma posição do tabuleiro, O para acertos e X para erros
void attack(char board[SIZE][SIZE], int row, int col) {
    if (board[row][col] == 'O' || board[row][col] == 'X') {

        do {
            printf("Posicao ja marcada! Tente novamente\n");
            printf("\nDigite a linha (1-10) para marcar no cliente: ");
            scanf("%d", &row);
            printf("\nDigite a coluna (1-10) para marcar no cliente: ");
            scanf("%d", &col);
        } while (board[row-1][col-1] == 'O' || board[row-1][col-1] == 'X');

        if (board[row-1][col-1] == 'S') {
        board[row-1][col-1] = 'O';
        } else {
        board[row-1][col-1] = 'X';
        }

        return;
    }

    if (board[row][col] == 'S') {
        board[row][col] = 'O';
    } else {
        board[row][col] = 'X';
    }
}


// função para verificar se o jogo acabou, se não houver mais navios (S) no tabuleiro
bool isGameEnded(char board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 'S') {
                return false;
            }
        }
    }
    return true;
}

int main() {
    HANDLE hPipe;
    DataPackage data;
    DWORD bytesRead, bytesWritten;

    srand(time(NULL));

    setlocale(LC_ALL, "Portuguese");

    // Inicializar os tabuleiros cliente e servidor com . e insere 5 navios aleatoriamente
    initBoard(data.serverBoard, '.', 6);
    initBoard(data.clientBoard, '.', 6);


    hPipe = CreateNamedPipe(
        PIPE_NAME, 
        PIPE_ACCESS_DUPLEX, 
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 
        BUFFER_SIZE, 
        BUFFER_SIZE, 
        0, 
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("Erro ao criar o pipe. Código de erro: %d\n", GetLastError());
        return 1;
    }

    printf("Aguardando conexão do cliente...\n");

    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        printf("Falha ao conectar ao cliente. Código de erro: %d\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    printf("INICIANDO O JOGO: BATALHA NAVAL!\n\n");

     // Enviar os tabuleiros inicializados para o cliente
    BOOL result = WriteFile(
        hPipe, 
        &data, 
        sizeof(data), 
        &bytesWritten, 
        NULL
    );

    if (!result) {
        printf("Erro ao enviar os tabuleiros. Código de erro: %d\n", GetLastError());
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        return 1;
    }

    int continueCommunication = 1;
    while (continueCommunication) {
        BOOL result = ReadFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesRead, 
            NULL
        );

        if (!result || bytesRead == 0) {
            break;
        }


        printf("\n\n");
        printf("Seu tabuleiro:\n");
        displayBoard(data.serverBoard);
        printf("\n\n");
        printf("Tabuleiro do oponente:\n");
        displayBoardWithoutShip(data.clientBoard);
        printf("\n\n");

        printf("Digite a linha (1-10) para marcar no cliente: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar no cliente: ");
        scanf("%d", &data.col);

        attack(data.clientBoard, data.row-1, data.col-1);

        if (isGameEnded(data.clientBoard)) {
            printf("Você venceu! Jogo fechando..\n");
            Sleep(5000);
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
            return 1;
        }

        result = WriteFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesWritten, 
            NULL
        );

        if (!result) {
            printf("Erro ao enviar dados para o cliente. Código de erro: %d\n", GetLastError());
            break;
        }
    }
    printf("Você perdeu! Jogo fechando..\n");
    Sleep(5000);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}
