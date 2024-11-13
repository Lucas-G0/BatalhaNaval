#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define SIZE 10
#define BUFFER_SIZE (2 * SIZE * SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char serverBoard[SIZE][SIZE];
    char clientBoard[SIZE][SIZE];
    int row, col; // Para marcar a jogada
} DataPackage;


//função para mostrar o tabuleiro completo
void displayBoard (char matriz[SIZE][SIZE]) {
    printf("   ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d ", i + 1);
    }
    printf("\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%2d  ", i + 1);
        for (int j = 0; j < SIZE; j++) {
            printf("%c  ", matriz[i][j]);
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

    setlocale(LC_ALL, "pt_BR.UTF-8");

    while (1) {
        hPipe = CreateFile(
            PIPE_NAME, 
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            0, 
            NULL
        );

        if (hPipe != INVALID_HANDLE_VALUE) {
            printf("INICIANDO O JOGO: BATALHA NAVAL!\n\n");
            break;
        }

        if (GetLastError() == ERROR_PIPE_BUSY) {
            printf("Servidor ocupado, aguardando...\n");
            Sleep(1000);
        } else {
            printf("Erro ao conectar. Código de erro: %d\n", GetLastError());
            return 1;
        }
    }

    // Receber os tabuleiros iniciais enviados pelo servidor
    BOOL result = ReadFile(
        hPipe, 
        &data, 
        sizeof(data), 
        &bytesRead, 
        NULL
    );

    if (!result || bytesRead == 0) {
        printf("Erro ao iniciar o jogo.\n");
        CloseHandle(hPipe);
        return 1;
    }

    printf("\n\nSeu tabuleiro\n");
    displayBoard(data.clientBoard);
    printf("\n\nTabuleiro do oponente:\n");
    displayBoardWithoutShip(data.serverBoard);
    printf("\n\n");

    int continueCommunication = 1;
    while (continueCommunication) {
        printf("Digite a linha que deseja atacar: ");
        scanf("%d", &data.row);
        printf("Digite a coluna que deseja atacar: ");
        scanf("%d", &data.col);

        attack(data.serverBoard, data.row-1, data.col-1);

        if (isGameEnded(data.serverBoard)) {
            printf("Você venceu! Jogo fechando..\n\n");
            Sleep(5000);
            CloseHandle(hPipe);
            return 1;
        }

        BOOL result = WriteFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesWritten, 
            NULL
        );

        if (!result) {
            printf("Erro ao enviar dados. Código de erro: %d\n", GetLastError());
            break;
        }

        result = ReadFile(
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
        displayBoard(data.clientBoard);
        printf("\n\n");
        printf("Tabuleiro do oponente:\n");
        displayBoardWithoutShip(data.serverBoard);
        printf("\n\n");
    }
    printf("\nVocê perdeu! Jogo fechando..\n");
    Sleep(5000);
    CloseHandle(hPipe);
    return 0;
}
