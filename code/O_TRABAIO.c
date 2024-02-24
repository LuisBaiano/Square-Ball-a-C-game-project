/* Criação de um jogo inspirado em Breakout para ser executado em uma DE1-SoC
 - A raquete/barra será movimentada pelo jogador por meio do acelerômetro
 - Todos os itens visuais serão exibidos por meio da interface VGA
 - Botões serão utilizados para: reiniciar, pausar e continuar o jogo
 - O jogo é encerrado de vez pela utilização de ^C
*/

#include <intelfpgaup/KEY.h>
#include <intelfpgaup/accel.h>
#include <intelfpgaup/video.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>

//Variável global para encerrar o programa
volatile sig_atomic_t sair;
void catchSIGINT(int signum) { sair = 1; }


#define video_BLACK 0x00
#define LARGURA_TELA 319 // Tamanho da tela VGA
#define ALTURA_TELA 239 // Tamanho da tela VGA
#define LINHA 9 // Quantidade de linhas de blocos
#define COLUNA 9 // Quantidade de colunas de blocos

// Estrutura de dados para os blocos
typedef struct {
  int tam_x, tam_y; // Tamanho do bloco (o quanto se deve incrementar o espaço
                    // na video_box pra criar o bloco)
  int pos_x, pos_y; // Posição em eixo X e Y em que será criado o bloco
  short cor;        // Cor do bloco
  bool quebrado;    // Se o bloco está já quebrado ou não
} Bloco;

void interpreta_botoes(int *botoes, bool *pause, bool *reset); // Função para interpretar os botões e gerar sinal de pause ou reset
void gera_blocos(Bloco blocos[COLUNA][LINHA], int *quant_blocos); // Função para gerar os blocos
void exibe_blocos(Bloco blocos[COLUNA][LINHA]); // Função para exibir os blocos
void detecta_colisao(Bloco blocos[COLUNA][LINHA], int *quant_blocos, bool *vitoria, bool *derrota, 
                    int bolaX, int bolaY, int *move_bolaX, int *move_bolaY, int *score); // Função para detectar colisão com os blocos

int main(int argc, char *argv[]) {
    signal(SIGINT, catchSIGINT);
    if (KEY_open() == 0 || video_open() == 0 || accel_open() == 0) {
    printf("Erro na inicialização de periféricos.\n");
    return -1;
    }
    printf("Periféricos inicializados.\n");

    Bloco blocos[COLUNA][LINHA]; // 9x9 blocos

    video_erase();

    // Inicialização e calibração do acelerômetro
    accel_init();
    accel_format(1, 2);
    accel_calibrate();

    int botoes, quant_blocos; 
    bool pause = false, reset = false, fim_de_jogo = false, vitoria = false, derrota = false; // Variáveis de controle
    int acel_rdy, acel_tap, acel_dtap, acel_x, acel_y, acel_z, acel_mg;
    int raquete_xi = (319 / 2) - 25; // Posição do início X da barra
    int raquete_xf = (319 / 2) + 25; // Posição do final X da barra
    int raquete_yi = 210 - 1; // Posição do início Y da barra
    int raquete_yf = 210 + 1; // Posição do final Y da barra
    int bolaX = (319 / 2); // Posição X inicial da bola
    int bolaY = (200); // Posição Y inicial da bola
    int move_bolaX = 1; // Sentido de movimentação da bola em X
    int move_bolaY = -1; // Sentido de movimentação da bola em Y
    int score = 0; // Pontuação do jogador
    char str[15]; // String para exibição da pontuação
    char comeco_str[] = "---PLAY---!"; // String para exibição de início de jogo
    char vitoria_str[] = "YOU WIN!"; // String para exibição de vitória
    char derrota_str[] = "YOU LOSE!"; // String para exibição de derrota
    char pause_str[] = "PAUSE!"; // String para exibição de pausa
    char fim_str[] = "RESET?"; // String para exibição de possível reset

    // Início do jogo
    while (!sair) {
        printf("Tela inicial do jogo.\n");
        video_clear();
        video_erase();
        video_text(36, 45, comeco_str);
        video_show();

        printf("Aguardando início do jogo...\n");
        pause = true;
        while (pause && !sair) interpreta_botoes(&botoes, &pause, &reset); // Aguarda início do jogo
        printf("Inicializando elementos de jogo...\n");
        
        video_erase();
        fim_de_jogo = false;
        vitoria = false;
        derrota = false;
        reset = false;

        // Inicialização de elementos de jogo
        gera_blocos(blocos, &quant_blocos);
        printf("%d blocos gerados...\n", quant_blocos);
        raquete_xi = (319 / 2) - 25;
        raquete_xf = (319 / 2) + 25;
        raquete_yi = 210 - 1;
        raquete_yf = 210 + 1;
        printf("Barra inicializada...\n");
        bolaX = (319 / 2);
        bolaY = (200);
        move_bolaX = 1;
        move_bolaY = -1;
        printf("Bola inicializada...\n");
        score = 0;
        printf("Score inicializado...\n");


        printf("Iniciando jogo...\n");

        // Início rodada de jogo
        while (!reset && !fim_de_jogo && !sair){
            // Início verificação de pause e/ou reset
            interpreta_botoes(&botoes, &pause, &reset);
            if (pause) {
            printf("Programa pausado.\n");
            video_text(36, 45, pause_str);
            video_show();
            while (pause && !sair && !reset) interpreta_botoes(&botoes, &pause, &reset);
            video_erase();
            }
            // Fim verificação de pause e/ou reset
            
            video_clear();
            // Leitura do acelerômetro para movimentação da raquete (barra)
            accel_read(&acel_rdy, &acel_tap, &acel_dtap, &acel_x, &acel_y, &acel_z,
                    &acel_mg);

            video_box(0, 0, LARGURA_TELA, ALTURA_TELA,
                    video_BLACK); // Desenha o fundo da tela

            video_box(bolaX, bolaY, bolaX + 2, bolaY + 2, video_WHITE); // Desenha a bola

            // Início movimentação da barra
            if (acel_x > -10 && acel_x < 10) { // Validação para evitar trepidação da barra
            } else if (acel_x < 0 && raquete_xi > 7) {

            raquete_xi -= 3;
            raquete_xf -= 3;
            } else if (acel_x > 0 && raquete_xf < (319 - 13)) {

            raquete_xi += 3;
            raquete_xf += 3;
            } else {
            }
            // Fim movimentação da barra

            // Início movimentação da bola e detecção de colisão
            if ((bolaX) >= 307 || (bolaX) <= 0) {
            move_bolaX *= -1;
            }

            if ((bolaY) >= 230 || (bolaY) <= 0) {
            move_bolaY *= -1;
       
            }

            if (((bolaY + 2) == raquete_yi) &&
                (bolaX + 2 >= raquete_xi && bolaX <= raquete_xf)) {
            move_bolaY *= -1;
            } else if (((bolaY + 2) >= raquete_yf) &&
                    (bolaX + 2 == raquete_xi || bolaX == raquete_xf)) {
            move_bolaY *= -1;
            move_bolaX *= -1;
            }

            if (raquete_yf <= bolaY + 2) {
            	if (bolaY >= 230) derrota = true;
            }

            bolaX += move_bolaX;
            bolaY += move_bolaY;
            // Fim movimentação da bola e detecção de colisão

            video_box(raquete_xi, raquete_yi, raquete_xf, raquete_yf, video_WHITE); // Desenha a barra  

            detecta_colisao(blocos, &quant_blocos, &vitoria, &derrota, bolaX, bolaY,
                            &move_bolaX, &move_bolaY, &score); // Detecta colisão da bola com os blocos
            sprintf(str, "Score: %d", score); // Atualiza a pontuação e exibe em terminal
            video_text(4, 54, str); // Exibe a pontuação
            exibe_blocos(blocos); // Desenha os blocos

            video_show();

            // Fim movimentação da bola e detecção de colisão

       
            if (vitoria || derrota) fim_de_jogo = true; // Verifica uma vitória ou derrota
        }
        //Fim rodada de jogo

        printf("Evento detectado. Fim de rodada de jogo.\n");
        video_clear();
        video_erase();

        // Início exibição de telas finais do jogo
        if (derrota){
            printf("Derrota! Colisão com o chão!\n");
            video_text(36, 20, derrota_str);
        } else if (vitoria){
            printf("Parabéns! Você venceu!\n");
            video_text(36, 20, vitoria_str);
        } else {
            printf("Fim de jogo.\n");
            video_text(36, 20, fim_str);
        }
        video_show();
        // Fim exibição de telas finais do jogo

        printf("Aguardando ação do jogador...\n");
        reset = false;
        while (!reset && !sair) interpreta_botoes(&botoes, &pause, &reset); // Aguarda ação do jogador
        video_erase();
        if (reset) printf("Reiniciando jogo...\n");
    }
    // Fim do jogo

    printf("Encerrando...\n");
    KEY_close();
    video_close();
    accel_close();
    return 0;
}

/* Função para leitura de entrada dos botões 
Pause funciona com lógica de alternância */
void interpreta_botoes(int *botoes, bool *pause, bool *reset) {
  KEY_read(botoes);
  if (*botoes > 7) {
    if (*pause == false)
      *pause = true;
    else
      *pause = false;
  }
  if (*botoes % 2 != 0) *reset = true;
  else *reset = false;
}

/* Função para geração dos blocos e contabilização dos blocos gerados */
void gera_blocos(Bloco blocos[COLUNA][LINHA], int *quant_blocos) {
  int linha, coluna;
  *quant_blocos = 0;
  for (linha = 0; linha < LINHA; linha++) {
    for (coluna = 0; coluna < COLUNA; coluna++) {
      blocos[linha][coluna].tam_x = 30;
      blocos[linha][coluna].tam_y = 10;
      // blocos[linha * 10 + coluna].cor = 0xFFFF;
      blocos[linha][coluna].cor = video_WHITE;
      blocos[linha][coluna].pos_x = coluna * 35;
      blocos[linha][coluna].pos_y = linha * 15;
      blocos[linha][coluna].quebrado = false;
      *quant_blocos += 1;
    }
  }
}

/* Função para exibição dos blocos */
void exibe_blocos(Bloco blocos[COLUNA][LINHA]) {
  int i;
  int j;
  for (i = 0; i < LINHA; i++) {
    for (j = 0; j < COLUNA; j++) {
      if (!blocos[i][j].quebrado) {
        video_box(blocos[i][j].pos_x, blocos[i][j].pos_y,
                  (blocos[i][j].pos_x + blocos[i][j].tam_x),
                  (blocos[i][j].pos_y + blocos[i][j].tam_y), blocos[i][j].cor);
      }
    }
  }
}

/* Função para detecção de colisão com os blocos */
void detecta_colisao(Bloco blocos[COLUNA][LINHA], int *quant_blocos,
                     bool *vitoria, bool *derrota, int bolaX, int bolaY,
                     int *move_bolaX, int *move_bolaY, int *score) {
  int i = 0;
  int j = 0;

  // Início colisão com blocos
  for (i = 0; i < LINHA; i++) {
    for (j = 0; j < COLUNA; j++) {
      if (!blocos[i][j].quebrado) {
        // Início colisão com parte inferior ou superior do bloco
        if ((bolaX >= blocos[i][j].pos_x) &&
            (bolaX + 2 <= blocos[i][j].pos_x + blocos[i][j].tam_x)) {
          if ((bolaY <= blocos[i][j].pos_y + blocos[i][j].tam_y) &&
              (bolaY + 2 >= blocos[i][j].pos_y)) {
            *move_bolaY *= -1;
            blocos[i][j].quebrado = true;
            *quant_blocos -= 1;
            *score += 10;
            printf("Colisão com o bloco[%d][%d]\n", i, j);
            printf("Blocos restantes: %d\n", *quant_blocos);
            printf("Score: %d\n", *score);
          }
        }
        // Fim colisão com parte inferior ou superior do bloco

        // Início colisão com parte lateral do bloco
        else if ((bolaY + 2 >= blocos[i][j].pos_y) &&
                 (bolaY <= blocos[i][j].pos_y + blocos[i][j].tam_y)) {
          if ((bolaX <= blocos[i][j].pos_x + blocos[i][j].tam_x) &&
              (bolaX + 2 >= blocos[i][j].pos_x)) {
            *move_bolaX *= -1;
            blocos[i][j].quebrado = true;
            *quant_blocos -= 1;
            score += 10;
            printf("Colisão lateral com o bloco[%d][%d]\n", i, j);
            printf("Blocos restantes: %d\n", *quant_blocos);
            printf("Score: %d\n", *score);
          }
        }
        // Fim colisão com parte lateral do bloco

        // Verifica quantidade de blocos existentes
        if (*quant_blocos == 0) {
            printf("Todos os blocos quebrados!\n");
            printf("Score: %d\n", *score);
          *vitoria = true;
          return;
        }
      }
    }
  }
}
