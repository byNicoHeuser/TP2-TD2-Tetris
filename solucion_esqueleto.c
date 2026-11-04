// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {
    // TODO: Contar los caracteres de src, reservar con malloc (no olvidar el '\0'),
    //       copiar todo y devolver el puntero nuevo.
    if (src == NULL) {
        return NULL;
    }

    // 1. Contar la cantidad de caracteres de src
    int largo = 0;
    while (src[largo] != '\0') {
        largo++;
    }

    // 2. Solicitar mediante malloc la memoria necesaria (largo + '\0')
    char* res = malloc(largo + 1);
    if (res == NULL) {
        return NULL;
    }

    // 3. Copiar todos los caracteres (incluyendo el terminador '\0')
    for (int i = 0; i <= largo; i++) {
        res[i] = src[i];
    }

    // 4. Retornar el puntero al nuevo string
    return res;
}

int strCompare(char* s1, char* s2) {
    // TODO: Devolver 0 si son iguales, 1 si s1 < s2, -1 si s1 > s2.
    //       Ojo: es la convencion INVERSA a strcmp.

    int i = 0;
    //si el char es no es el ultimo en ninguno de los dos, compara cual es mayor/menor.
    while (s1[i] != '\0' && s2[i] != '\0'){
        if (s1[i] < s2[i]) return 1;
        if (s1[i] > s2[i]) return -1;
        i++;
    }
    //Despues, si ambos ya estan en el ultimo char significa que son iguales, return 0.
    if (s1[i] == '\0' && s2[i] == '\0') return 0;
    // si solo s1 llego al ultimo char, return 1. Sino por descarte ya hace return -1 (s1 > s2).
    if (s1[i] == '\0') return 1;
    return -1;
}

char* strConcatenate(char* src1, char* src2) {
    size_t largo1 = 0;
    size_t largo2 = 0;

    while (src1[largo1] != '\0') largo1++;
    while (src2[largo2] != '\0') largo2++;

    // Se reserva un lugar extra para el terminador del string.
    char* resultado = malloc(largo1 + largo2 + 1);
    if (resultado == NULL) {
        // La funcion se hace cargo de ambos argumentos, aun si falla malloc.
        free(src1);
        free(src2);
        return NULL;
    }

    for (size_t i = 0; i < largo1; i++) resultado[i] = src1[i];
    for (size_t i = 0; i < largo2; i++) resultado[largo1 + i] = src2[i];
    resultado[largo1 + largo2] = '\0';

    // Se liberan despues de copiar para no leer memoria ya liberada.
    free(src1);
    free(src2);
    return resultado;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

void gameBoardDelete(GameBoard* board);

GameBoard* gameBoardNew() {
    // Reservamos memoria para la estructura principal del tablero.
    GameBoard* res = malloc(sizeof(GameBoard));
    if (res == NULL) {
        return NULL;
    }

    // Inicializamos las listas vacías y los valores iniciales del juego.
    res->first_row = NULL;
    res->queue_head = NULL;
    res->queue_tail = NULL;
    res->first_particle = NULL;
    res->current_piece.col = 0;
    res->current_piece.row = 0;
    res->current_piece.type = 0;
    res->current_piece.rotation = 0;
    res->current_piece.active = 0;
    res->score = 0;
    res->tick_timer = 0;

    // "ultimo" permite conectar cada fila nueva al final de la lista.
    RowNode* ultimo = NULL;
 
    // Creamos las filas del tablero una por una.
    for (int i = 0; i < GRID_ROWS; i++){
        RowNode* nuevaFila = malloc(sizeof(RowNode));
        if (nuevaFila == NULL){
            // Si falla una reserva, borramos lo creado hasta ahora.
            gameBoardDelete(res);
            return NULL;
        }
        // Las celdas empiezan vacías.
        for (int c = 0; c < GRID_COLS; c++) {
            nuevaFila->cells[c] = 0;
        } 
        nuevaFila->next = NULL;
        if (res->first_row == NULL){
            // La primera fila pasa a ser el inicio de la lista.
            res->first_row = nuevaFila;
        }
        else {
            // Las siguientes filas se enlazan después de la última.
            ultimo->next = nuevaFila;
        }
        ultimo = nuevaFila;
    }
    // Devolvemos el tablero ya inicializado.
    return res;
}

void gameBoardDelete(GameBoard* board) {
    // Si no hay tablero, no hay nada que liberar.
    if (board == NULL){
        return;
    }

    // Guardamos el siguiente nodo antes de liberar la fila actual.
    RowNode* siguiente = NULL;
    while (board->first_row != NULL){
        siguiente = board->first_row->next;
        free(board->first_row);
        board->first_row = siguiente;
    }

    // Liberamos los nodos de la cola desde el primero.
    PieceQueueNode* siguiente_pieza = NULL;
    while (board->queue_head != NULL) {
        siguiente_pieza = board->queue_head->next;
        free(board->queue_head);
        board->queue_head = siguiente_pieza;
    }
    // La cola ya quedó vacía.
    board->queue_tail = NULL;

    // Liberamos todas las partículas de la lista.
    ParticleNode* siguiente_particula = NULL;
    while (board->first_particle != NULL) {
        siguiente_particula = board->first_particle->next;
        free(board->first_particle);
        board->first_particle = siguiente_particula;
    }

    // Liberamos la estructura que contiene las listas.
    free(board);

}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    if (board == NULL) return;

    ParticleNode* nueva = malloc(sizeof(ParticleNode));
    if (nueva == NULL) return;

    nueva->x = x;
    nueva->y = y;
    nueva->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    nueva->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nueva->color = color;
    nueva->life_timer = 30 + (rand() % 30);

    // La nueva particula apunta a la cabeza anterior y pasa a ser la cabeza.
    nueva->next = board->first_particle;
    board->first_particle = nueva;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    if (board == NULL) return 0;

    RowNode* actual = board->first_row;
    RowNode* anterior = NULL;
    int fila = 0;
    int eliminadas = 0;

    while (actual != NULL) {
        int completa = 1;
        for (int c = 0; c < GRID_COLS; c++) {
            if (actual->cells[c] == 0) {
                completa = 0;
                break;
            }
        }

        if (completa) {
            // Reservo el reemplazo antes de borrar para conservar las 20 filas
            // incluso si no hay memoria disponible.
            RowNode* nueva = malloc(sizeof(RowNode));
            if (nueva == NULL) return eliminadas;
            for (int c = 0; c < GRID_COLS; c++) nueva->cells[c] = 0;

            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(board,
                    (float)(OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2),
                    (float)(OFFSET_Y + fila * CELL_SIZE + CELL_SIZE / 2),
                    actual->cells[c]);
            }

            RowNode* siguiente = actual->next;
            if (anterior == NULL) {
                board->first_row = siguiente;
            } else {
                anterior->next = siguiente;
            }
            free(actual);

            nueva->next = board->first_row;
            board->first_row = nueva;

            // Si borre la cabeza, la fila nueva es la anterior a siguiente.
            // Esto permite borrar tambien varias filas completas al inicio.
            if (anterior == NULL) anterior = nueva;
            actual = siguiente;
            eliminadas++;
        } else {
            anterior = actual;
            actual = actual->next;
        }
        fila++;
    }
    return eliminadas;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    // TODO: Crear un PieceQueueNode con el 'type'.
    // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.

    // Reservo la memoria para el nuevo nodo
    PieceQueueNode* node = malloc(sizeof(PieceQueueNode));
    if (node == NULL) return;
    node->type = type;
    node->next = NULL;
    if (board->queue_tail == NULL){
        board->queue_tail = node;
        board->queue_head = node;
    }else {
        board->queue_tail->next = node;
        board->queue_tail = node;
    }
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    // TODO: Si la cola está vacía, retornar 0.
    // TODO: Extraer el primer nodo (queue_head).
    // TODO: Guardar el type en la variable out_type, liberar el nodo y actualizar el head.
    if (board->queue_head == NULL) return 0;
    *out_type = board->queue_head->type;
    // Guardo un puntero al head actual para poder liberarlo despues de mover queue_head
    PieceQueueNode* node = board->queue_head;
    board->queue_head = board->queue_head->next;
    free(node);
    if (board->queue_head == NULL) board->queue_tail = NULL;
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

void correrTests() {
    int fallas = 0;

    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.
    char* copia1 = strDuplicate("");
    if (strCompare(copia1, "") == 0) {
        printf("strDuplicate \"\" OK \n");
    }else{
        printf("strDuplicate \"\" FALLA \n");
        fallas++;
    }
    free(copia1);

    char* copia2 = strDuplicate("x");
    if (strCompare(copia2, "x") == 0) {
        printf("strDuplicate \"x\" OK \n");
    }else{
        printf("strDuplicate \"x\" FALLA \n");
        fallas++;
    }
    free(copia2);

    char* copia3 = strDuplicate("ankara messi");
    if (strCompare(copia3, "ankara messi") == 0) {
        printf("strDuplicate \"ankara messi\" OK \n");
    }else{
        printf("strDuplicate \"ankara messi\" FALLA \n");
        fallas++;
    }
    free(copia3);
    
    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).

    if (strCompare("", "") == 0) {
        printf("strCompare \"\", \"\" OK \n");
    }else{
        printf("strCompare \"\", \"\" FALLA \n");
        fallas++;
    }

    if (strCompare("a", "a") == 0) {
        printf("strCompare \"a\", \"a\" OK \n");
    }else{
        printf("strCompare \"a\", \"a\" FALLA \n");
        fallas++;
    }

    if (strCompare("a", "b") == 1) {
        printf("strCompare \"a\", \"b\" OK \n");
    }else{
        printf("strCompare \"a\", \"b\" FALLA \n");
        fallas++;
    }

    if (strCompare("b", "a") == -1) {
        printf("strCompare \"b\", \"a\" OK \n");
    }else{
        printf("strCompare \"b\", \"a\" FALLA \n");
        fallas++;
    }

    if (strCompare("casa", "casas") == 1) {
        printf("strCompare \"casa\", \"casas\" OK \n");
    }else{
        printf("strCompare \"casa\", \"casas\" FALLA \n");
        fallas++;
    }

    if (strCompare("casas", "casa") == -1) {
        printf("strCompare \"casas\", \"casa\" OK \n");
    }else{
        printf("strCompare \"casas\", \"casa\" FALLA \n");
        fallas++;
    }

    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).
    char* izquierda[] = {"", "hola ", "abc", ""};
    char* derecha[] = {"abc", "mundo", "", ""};
    char* esperado[] = {"abc", "hola mundo", "abc", ""};
    for (int i = 0; i < 4; i++) {
        char* a = strDuplicate(izquierda[i]);
        char* b = strDuplicate(derecha[i]);
        char* resultado = NULL;
        if (a != NULL && b != NULL) {
            resultado = strConcatenate(a, b);
        }else{
            free(a);
            free(b);
        }
        if (resultado != NULL && strCompare(resultado, esperado[i]) == 0) {
            printf("strConcatenate \"%s\" + \"%s\" OK \n", izquierda[i], derecha[i]);
        }else{
            printf("strConcatenate \"%s\" + \"%s\" FALLA \n", izquierda[i], derecha[i]);
            fallas++;
        }
        free(resultado);
    }

    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.
    GameBoard* board = gameBoardNew();
    if (board == NULL) {
        printf("gameBoardNew: FALLA (no se pudo crear el tablero)\n");
        exit(EXIT_FAILURE);
    }
    if (board->queue_head == NULL && board->queue_tail == NULL && board->first_particle == NULL &&
        board->score == 0 && board->tick_timer == 0 && board->current_piece.active == 0) {
        printf("gameBoardNew - estado inicial: OK\n");
    }else{
        printf("gameBoardNew - estado inicial: FALLA\n");
        fallas++;
    }

    RowNode* fila = board->first_row;
    int cantidad = 0, vacias = 1;
    while (fila != NULL && cantidad < GRID_ROWS) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (fila->cells[c] != 0) vacias = 0;
        }
        cantidad++;
        fila = fila->next;
    }
    if (cantidad == GRID_ROWS && fila == NULL && vacias) {
        printf("gameBoardNew - 20 filas vacias: OK\n");
    }else{
        printf("gameBoardNew - 20 filas vacias: FALLA\n");
        fallas++;
    }

    int tipos[] = {4, 1, 6, 0, 3};
    for (int i = 0; i < 5; i++) gameBoardEnqueuePiece(board, tipos[i]);
    if (board->queue_head != NULL && board->queue_tail != NULL && board->queue_head->type == tipos[0] &&
        board->queue_tail->type == tipos[4] && board->queue_tail->next == NULL) {
        printf("gameBoardEnqueuePiece - cabeza y final: OK\n");
    }else{
        printf("gameBoardEnqueuePiece - cabeza y final: FALLA\n");
        fallas++;
    }

    for (int i = 0; i < 5; i++) {
        int tipo = -1;
        int saco = gameBoardDequeuePiece(board, &tipo);
        if (saco == 1 && tipo == tipos[i]) {
            printf("FIFO - extraccion %d: OK\n", i + 1);
        }else{
            printf("FIFO - extraccion %d: FALLA (esperaba %d, obtuvo %d)\n", i + 1, tipos[i], tipo);
            fallas++;
        }
    }
    if (board->queue_head == NULL && board->queue_tail == NULL) {
        printf("Cola vacia al sacar la quinta: OK\n");
    }else{
        printf("Cola vacia al sacar la quinta: FALLA\n");
        fallas++;
    }

    int tipo = 99;  // Valor conocido para detectar si la funcion lo pisa al fallar.
    int saco = gameBoardDequeuePiece(board, &tipo);
    if (saco == 0 && tipo == 99 && board->queue_head == NULL && board->queue_tail == NULL) {
        printf("Sexta extraccion - no cambia la salida: OK\n");
    }else{
        printf("Sexta extraccion - no cambia la salida: FALLA\n");
        fallas++;
    }

    gameBoardEnqueuePiece(board, 2);
    saco = gameBoardDequeuePiece(board, &tipo);
    if (saco == 1 && tipo == 2 && board->queue_head == NULL && board->queue_tail == NULL) {
        printf("Reutilizar la cola vacia: OK\n");
    }else{
        printf("Reutilizar la cola vacia: FALLA\n");
        fallas++;
    }

    // Agregamos dos particulas para ver cual queda en la cabeza.
    gameBoardAddParticle(board, 10.5f, 20.5f, 3);
    ParticleNode* primera = board->first_particle;
    if (primera != NULL && primera->x == 10.5f && primera->y == 20.5f &&
        primera->color == 3 && primera->next == NULL) {
        printf("Primera particula: OK\n");
    }else{
        printf("Primera particula: FALLA\n");
        fallas++;
    }

    gameBoardAddParticle(board, 35.5f, 7.5f, 6);
    ParticleNode* cabeza = board->first_particle;
    if (primera != NULL && cabeza != NULL && cabeza != primera && cabeza->next == primera &&
        cabeza->x == 35.5f && cabeza->y == 7.5f && cabeza->color == 6) {
        printf("Nueva particula en la cabeza: OK\n");
    }else{
        printf("Nueva particula en la cabeza: FALLA\n");
        fallas++;
    }

    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que first_row es la fila nueva vacia y que
    //    la lista sigue teniendo 20 filas.
    // Tambien probamos las filas 0 y 1 juntas, dos separadas y los extremos.
    // En filaA y filaB, -1 significa que no se eligio una fila.
    char* casos[] = {"ninguna completa", "ultima fila (19)", "filas 0 y 1", "filas 5 y 12", "todas las filas"};
    int filaA[]   = {-1, 19, 0,  5, -1};
    int filaB[]   = {-1, -1, 1, 12, -1};
    int aBorrar[] = { 0,  1, 2,  2, GRID_ROWS};
    for (int caso = 0; caso < 5; caso++) {
        GameBoard* tableroFilas = gameBoardNew();
        if (tableroFilas == NULL) {
            printf("Preparar tablero (%s): FALLA\n", casos[caso]);
            gameBoardDelete(board);
            exit(EXIT_FAILURE);
        }
        int colores[GRID_ROWS];  // Marcas de las filas que deben sobrevivir, en orden.
        int guardadas = 0;
        fila = tableroFilas->first_row;
        for (int r = 0; r < GRID_ROWS && fila != NULL; r++) {
            int completa = r == filaA[caso] || r == filaB[caso] || aBorrar[caso] == GRID_ROWS;
            int color = 1 + r % 7;
            for (int c = 0; c < GRID_COLS; c++) {
                if (completa) fila->cells[c] = 1 + c % 7;
                else fila->cells[c] = color;
            }
            if (!completa) {
                fila->cells[GRID_COLS - 1] = 0;  // Un solo hueco alcanza para no borrarla.
                colores[guardadas++] = color;
            }
            fila = fila->next;
        }

        int borradas = gameBoardCheckAndClearLines(tableroFilas);
        if (borradas == aBorrar[caso]) {
            printf("Lineas (%s) - cantidad borrada: OK\n", casos[caso]);
        }else{
            printf("Lineas (%s) - FALLA: esperaba %d, borro %d\n", casos[caso], aBorrar[caso], borradas);
            fallas++;
        }

        fila = tableroFilas->first_row;
        int r = 0, tableroOk = 1;
        while (r < GRID_ROWS && fila != NULL) {
            for (int c = 0; c < GRID_COLS; c++) {
                int valorEsperado = 0;  // Arriba deben aparecer las filas nuevas, vacias.
                if (r >= aBorrar[caso] && c < GRID_COLS - 1) {
                    int indice = r - aBorrar[caso];
                    if (indice < guardadas) valorEsperado = colores[indice];
                    else tableroOk = 0;
                }
                if (fila->cells[c] != valorEsperado) tableroOk = 0;
            }
            r++;
            fila = fila->next;
        }
        if (tableroOk && r == GRID_ROWS && fila == NULL) {
            printf("Lineas (%s) - 20 filas y contenido: OK\n", casos[caso]);
        }else{
            printf("Lineas (%s) - 20 filas y contenido: FALLA\n", casos[caso]);
            fallas++;
        }

        ParticleNode* p = tableroFilas->first_particle;
        int particulas = 0;
        while (p != NULL && particulas < GRID_ROWS * GRID_COLS) {
            particulas++;
            p = p->next;
        }
        if (particulas == aBorrar[caso] * GRID_COLS && p == NULL) {
            printf("Lineas (%s) - cantidad de particulas: OK\n", casos[caso]);
        }else{
            printf("Lineas (%s) - cantidad de particulas: FALLA\n", casos[caso]);
            fallas++;
        }

        // En la fila 19 verificamos ademas que haya una particula en cada celda original.
        if (caso == 1 && p == NULL) {
            int posicionesOk = 1;
            for (int c = 0; c < GRID_COLS; c++) {
                int x = OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2;
                int y = OFFSET_Y + 19 * CELL_SIZE + CELL_SIZE / 2;
                p = tableroFilas->first_particle;
                while (p != NULL && !(p->x == x && p->y == y && p->color == 1 + c % 7)) p = p->next;
                if (p == NULL) posicionesOk = 0;
            }
            if (posicionesOk) {
                printf("Fila 19 - posicion y color de cada particula: OK\n");
            }else{
                printf("Fila 19 - posicion y color de cada particula: FALLA\n");
                fallas++;
            }
        }
        gameBoardDelete(tableroFilas);
    }

    // Liberar toda la memoria pedida en los tests (strings y tableros).
    // Quedan piezas y particulas para comprobar su liberacion con Valgrind.
    gameBoardEnqueuePiece(board, 2);
    gameBoardEnqueuePiece(board, 4);
    gameBoardDelete(board);

    if (fallas > 0) {
        exit(EXIT_FAILURE);
    }

}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
