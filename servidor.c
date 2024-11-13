#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define SIZE 10
#define BUFFER_SIZE (2 * SIZE * SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char serverBoard[SIZE][SIZE];
    char clientBoard[SIZE][SIZE];
    int row, col; // Coordenada para marcar na board do cliente
} DataPackage;

void initBoard(char board[SIZE][SIZE], char valor) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = valor;
        }
    }
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

int main() {
    HANDLE hPipe;
    DataPackage data;
    DWORD bytesRead, bytesWritten;

    // Inicializar os tabuleiros cliente e servidor com .
    initBoard(data.serverBoard, '.');
    initBoard(data.clientBoard, '.');

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

        printf("Tabuleiro do cliente:\n");
        displayBoard(data.clientBoard);
        printf("\n\n");
        printf("Tabuleiro do servidor:\n");
        displayBoard(data.serverBoard);

        // Solicitar coordenada do servidor para marcar no cliente
        printf("Digite a linha (1-10) para marcar no cliente: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar no cliente: ");
        scanf("%d", &data.col);

        // Marcar coordenada
        data.clientBoard[data.row-1][data.col-1] = 'X';

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
