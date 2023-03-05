#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>


/*
 *  hwnd: é um identificador de janela (Window Handle) que representa a janela que deve receber a mensagem. É usado para identificar uma janela em particular entre todas as outras janelas abertas no sistema.
    msg: é um código que identifica o tipo de mensagem que está sendo enviada. As mensagens podem ser de diferentes tipos, como WM_PAINT (para desenhar a janela), WM_CLOSE (para fechar a janela) e WM_KEYDOWN (para indicar que uma tecla foi pressionada).
    wParam: é um parâmetro genérico que pode ser usado para passar informações adicionais para a mensagem. O seu significado varia dependendo do tipo de mensagem.
    lParam: é outro parâmetro genérico que também pode ser usado para passar informações adicionais para a mensagem. O seu significado varia dependendo do tipo de mensagem.

 *  O case WM_CREATE é executado quando a janela é criada e é utilizado para inicializar e criar a caixa de lista que exibirá as informações dos processos.
    O case WM_PAINT é executado sempre que a janela precisa ser repintada, por exemplo, quando é exibida pela primeira vez ou quando é redimensionada. Nesse caso, o case WM_PAINT é responsável por obter as informações dos processos e adicionar as informações à caixa de lista.
    O case WM_DESTROY é executado quando a janela é fechada, e é utilizado para encerrar o programa.


 *  Define o ID da lista de processos
    GetSysColorBrush(COLOR_3DFACE): Possui uma paleta de cores para usar
    LoadCursor(0, IDC_ARROW): Possui cursores para a janela, IDC_ARROW no nosso caso.
    RegisterClass(&wc): registra a classe da janela com o sistema operacional.
    GetUserName(userName, &userNameSize): obtém o nome do usuário atualmente logado no sistema operacional.
    CreateWindow(wc.lpszClassName, userName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 400, 400, 1280, 800, 0, 0, hInstance, 0): cria uma janela com o nome do usuário
    GetMessage(&msg, NULL, 0, 0): obtém a próxima mensagem da fila de mensagens para a janela.
    TranslateMessage(&msg):  é utilizada para traduzir uma mensagem de evento de entrada de baixo nível, capturada pela função GetMessage
    DispatchMessage(&msg): Pega o conteudo da syscall TranslateMessage() e permite o uso.
    CreateWindow(TEXT("LISTBOX"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 10, 10, 1240, 920, hwnd, (HMENU)ID_LIST, NULL, NULL): Cria uma lista de informações com um tamanho especifico
    SendMessage(hList, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE): Pegamos a lista de informações dos processos, adicionamos um cast WPARAM colocamos a font dentro de GetStockObject como DEFAULT_GUI_FONT
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"ID NOME THREADS"): primeiro indice da lista como uma string de cabeçalho.
    CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0): cria um snapshot (captura instantânea do estado atual de um recurso, como por exemplo a lista de processos em execução) dos processos em execução no sistema operacional.
    Process32First(snapshot, &process): obtém informações sobre o primeiro processo no snapshot de processos.
    Process32Next(snapshot, &process): obtém informações sobre o próximo processo no snapshot de processos.
    CloseHandle(snapshot):  é usado para liberar o identificador de um objeto de kernel previamente criado. Isso significa que ele libera o acesso a esse objeto pelo processo que o criou.
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)processInfo): adiciona uma string com as informações do processo à lista.
    PostQuitMessage(0): adiciona uma mensagem de saída à fila de mensagens, fazendo com que o loop principal de mensagens pare e o programa seja encerrado.

 * */
#define ID_LIST 101

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    HWND hwnd;                      // Handle da janela
    MSG  msg;                       // Estrutura de mensagem da janela
    WNDCLASS wc = {0};              // Estrutura da classe da janela

    wc.lpszClassName = TEXT("Gerenciador de processos"); // Nome da classe da janela
    wc.hInstance     = hInstance;   // Handle da instância do programa
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);   // Cor de fundo da janela
    wc.lpfnWndProc   = WndProc;     // Função de callback para processamento de mensagens
    wc.hCursor       = LoadCursor(0, IDC_ARROW);        // Cursor padrão da janela

    RegisterClass(&wc);             // Registra a classe da janela

    // Obter o nome do usuário
    TCHAR userName[246 + 1];        // Nome do usuário
    DWORD userNameSize = 256 + 1;   // Tamanho máximo do nome do usuário
    GetUserName(userName, &userNameSize); // Obtém o nome do usuário

    // Criar a janela com o nome do usuário como título
    hwnd = CreateWindow(wc.lpszClassName, userName,
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Estilo da janela
                        400, 400, 1280, 800, 0, 0, hInstance, 0); // Posição e tamanho da janela

    // Loop principal de processamento de mensagens
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);     // Traduz a mensagem
        DispatchMessage(&msg);      // coloca a mensagem
    }

    return (int) msg.wParam;        // Retorna o código de saída do programa
}

// Implementa a função de callback para processamento de mensagens da janela
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hList;              // Handle da lista de processos

    switch(msg)
    {
        // Mensagem de criação da janela
        case WM_CREATE:
        {
            // cria a lista de processos
            hList = CreateWindow(TEXT("LISTBOX"), TEXT(""), // Cria uma caixa de lista sem texto
                                 WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, // Estilo da caixa de lista
                                 10, 10, 1240, 920,         // Posição e tamanho da caixa de lista
                                 hwnd, (HMENU)ID_LIST, NULL, NULL);
            SendMessage(hList, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"ID                                            NOME                                            THREADS");
            break;
        }
        case WM_PAINT:
        {
            // Inicia o processo de pintura da janela
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // obtém a lista de processos
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            PROCESSENTRY32 process;
            process.dwSize = sizeof(PROCESSENTRY32);

            // verifica se foi possível obter informações do primeiro processo
            if (Process32First(snapshot, &process) == TRUE)
            {
                // itera sobre todos os processos
                do
                {
                    // cria uma string com as informações do processo
                    char processInfo[200];
                    sprintf(processInfo, "%-45d %-45s %-65d", process.th32ProcessID, process.szExeFile, process.cntThreads);

                    // adiciona a string à lista
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)processInfo);
                } while (Process32Next(snapshot, &process));
            }

            // fecha o snapshot
            CloseHandle(snapshot);

            // Quebra o paint case em execução
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
        {
            // Quebra a tarefa em execução
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}