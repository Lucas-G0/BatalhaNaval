#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define MATRIX_SIZE 10
#define BUFFER_SIZE (MATRIX_SIZE * MATRIX_SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char matrix[MATRIX_SIZE][MATRIX_SIZE];
    int row, col; // Coordenada para marcar na matriz do cliente
} DataPackage;

void inicializarMatriz(char matriz[MATRIX_SIZE][MATRIX_SIZE], char valor) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matriz[i][j] = valor;
        }
    }
}

void displayMatriz (char matriz[MATRIX_SIZE][MATRIX_SIZE]) {
    printf("   ");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf("%2d ", i + 1);  // Exibe os n�meros das colunas
    }
    printf("\n");

    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf("%2d  ", i + 1);  // Exibe as letras das linhas
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%c  ", matriz[i][j]);  // Exibe o conte�do de cada c�lula
        }
        printf("\n");
    }   
}

int main() {
    HANDLE hPipe;
    DataPackage data;
    DWORD bytesRead, bytesWritten;

    // Inicializar a matriz do servidor com '.'
    inicializarMatriz(data.matrix, '.');

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

        printf("Matriz recebida do cliente:\n");
        displayMatriz(data.matrix);

        // Solicitar coordenada do servidor para marcar no cliente
        printf("Digite a linha (1-10) para marcar no cliente: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar no cliente: ");
        scanf("%d", &data.col);

        // Marcar coordenada
        data.matrix[data.row-1][data.col-1] = 'S';

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
