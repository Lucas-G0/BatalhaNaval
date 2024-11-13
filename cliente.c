#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define MATRIX_SIZE 10
#define BUFFER_SIZE (MATRIX_SIZE * MATRIX_SIZE * sizeof(char) + sizeof(int) * 2)

typedef struct {
    char matrix[MATRIX_SIZE][MATRIX_SIZE];
    int row, col; // Coordenada para marcar na matriz do servidor
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

    // Inicializar a matriz do cliente com '.'
    inicializarMatriz(data.matrix, '.');

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
            printf("Conectado ao servidor!\n");
            break;
        }

        if (GetLastError() == ERROR_PIPE_BUSY) {
            printf("Pipe ocupado, aguardando...\n");
            Sleep(1000);
        } else {
            printf("Erro ao conectar ao pipe. C�digo de erro: %d\n", GetLastError());
            return 1;
        }
    }

    int continueCommunication = 1;
    while (continueCommunication) {
        printf("Digite a linha (1-10) para marcar: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar: ");
        scanf("%d", &data.col);

        data.matrix[data.row-1][data.col-1] = 'C';

        BOOL result = WriteFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesWritten, 
            NULL
        );

        if (!result) {
            printf("Erro ao enviar dados para o servidor. C�digo de erro: %d\n", GetLastError());
            break;
        }

        printf("Dados enviados com sucesso.\n");

        result = ReadFile(
            hPipe, 
            &data, 
            sizeof(data), 
            &bytesRead, 
            NULL
        );

        if (!result || bytesRead == 0) {
            printf("Servidor desconectado.\n");
            break;
        }

        printf("Matriz recebida do servidor:\n");
        displayMatriz(data.matrix);
    }

    CloseHandle(hPipe);
    return 0;
}
