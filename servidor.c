#include <windows.h>
#include <stdio.h>
#include <string.h>

#define PIPE_NAME "\\\\.\\pipe\\MyPipe"
#define MATRIX_SIZE 10
#define MESSAGE_SIZE 100
#define BUFFER_SIZE (MATRIX_SIZE * MATRIX_SIZE * sizeof(char) + sizeof(int) * 2 + MESSAGE_SIZE)

typedef struct {
    char matrix[MATRIX_SIZE][MATRIX_SIZE];
    int row, col; // Coordenada para marcar na matriz do cliente
    char message[MESSAGE_SIZE];
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
        printf("%2d ", i + 1);  // Exibe os números das colunas
    }
    printf("\n");

    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf("%2d  ", i + 1);  // Exibe as letras das linhas
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%c  ", matriz[i][j]);  // Exibe o conteúdo de cada célula
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

        printf("Mensagem recebida do cliente: %s\n", data.message);

        if (strncmp(data.message, "exit", 4) == 0) {
            continueCommunication = 0;
        }
        
        // Solicitar coordenada do servidor para marcar no cliente
        printf("Digite a linha (1-10) para marcar no cliente: ");
        scanf("%d", &data.row);
        printf("Digite a coluna (1-10) para marcar no cliente: ");
        scanf("%d", &data.col);

        // Marcar coordenada
        data.matrix[data.row-1][data.col-1] = 'S';

        printf("Digite a mensagem para o cliente: ");
        getchar();
        fgets(data.message, MESSAGE_SIZE, stdin);

        // Enviar dados atualizados para o cliente
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

        printf("Resposta enviada para o cliente.\n");
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}
