#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define SIZE 10
#define BUFFER_SIZE (2 * SIZE * SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char serverBoard[SIZE][SIZE];
    char clientBoard[SIZE][SIZE];
    int row, col; // Coordenada para marcar na board do cliente
} DataPackage;

void placeShip(char board[SIZE][SIZE], int qt) {
    for (int i = 0; i < qt; i++) {
        int row = rand() % SIZE;
        int col = rand() % SIZE;
        board[row][col] = 'S';
    }
}

void initBoard(char board[SIZE][SIZE], char valor, int qtShips) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = valor;
        }
    }

    placeShip(board, qtShips);
}

void displayBoard (char board[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);  // Exibe os n�meros das colunas
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%2d  ", i + 1);  // Exibe as letras das linhas
        for (int j = 0; j < SIZE; j++) {
            printf("%c  ", board[i][j]);  // Exibe o conte�do de cada c�lula
        }
        printf("\n");
    }   
}

void displayBoardWithoutShip(char matriz[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);  // Exibe os n�meros das colunas
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%2d  ", i + 1);  // Exibe as letras das linhas
        for (int j = 0; j < SIZE; j++) {
            if (matriz[i][j] == 'S') {
                printf(".  ");
            } else {
                printf("%c  ", matriz[i][j]);  // Exibe o conte�do de cada c�lula
            }
        }
        printf("\n");
    }   
}

void attack(char board[SIZE][SIZE], int row, int col) {
    if (board[row][col] == 'S') {
        board[row][col] = 'O';
    } else {
        board[row][col] = 'X';
    }
}

int main() {
    HANDLE hPipe;
    DataPackage data;
    DWORD bytesRead, bytesWritten;

    srand(time(NULL));

    // Inicializar os tabuleiros cliente e servidor com .
    initBoard(data.serverBoard, '.', 5);
    initBoard(data.clientBoard, '.', 5);


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
        printf("Erro ao criar o pipe. C�digo de erro: %d\n", GetLastError());
        return 1;
    }

    printf("Aguardando conex�o do cliente...\n");

    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        printf("Falha ao conectar ao cliente. C�digo de erro: %d\n", GetLastError());
        CloseHandle(hPipe);
        return 1;
    }

    printf("Cliente conectado!\n");

     // Enviar os tabuleiros iniciais para o cliente
    BOOL result = WriteFile(
        hPipe, 
        &data, 
        sizeof(data), 
        &bytesWritten, 
        NULL
    );

    if (!result) {
        printf("Erro ao enviar os tabuleiros iniciais para o cliente. Código de erro: %d\n", GetLastError());
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        return 1;
    }

    printf("Tabuleiros iniciais enviados para o cliente.\n");
    
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
            printf("Cliente desconectado.\n");
            break;
        }

        printf("\n\n");
        printf("Seu tabuleiro:\n");
        displayBoard(data.serverBoard);
        printf("\n\n");
        printf("Tabuleiro do oponente:\n");
        displayBoardWithoutShip(data.clientBoard);
        printf("\n\n");

        // Solicitar coordenada do servidor para marcar no cliente
        printf("Digite a linha (1-10) para marcar no cliente: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar no cliente: ");
        scanf("%d", &data.col);

        attack(data.clientBoard, data.row-1, data.col-1);

        // Enviar dados atualizados para o cliente
        result = WriteFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesWritten, 
            NULL
        );

        if (!result) {
            printf("Erro ao enviar dados para o cliente. C�digo de erro: %d\n", GetLastError());
            break;
        }

        printf("Resposta enviada para o cliente.\n");
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}
