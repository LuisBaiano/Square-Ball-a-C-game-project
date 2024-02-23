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

volatile sig_atomic_t sair;
void catchSIGINT(int signum) { sair = 1; }


#define video_BLACK 0x00
#define LARGURA_TELA 319
#define ALTURA_TELA 239
#define LINHA 9
#define COLUNA 9

typedef struct {
  int tamanho;
  int pos_x, pos_y;
  short cor;
} Projetil;

typedef struct {
  int tamanho_x, tamanho_y;
  int pos_x;
  short cor;
} Raquete;

typedef struct {
  int tam_x, tam_y; // Tamanho do bloco (o quanto se deve incrementar o espaço
                    // na video_box pra criar o bloco)
  int pos_x, pos_y; // Posição em eixo X e Y em que será criado o bloco
  short cor;        // Cor do bloco
  bool quebrado;    // Se o bloco está já quebrado ou não
} Bloco;

void interpreta_botoes(int *botoes, bool *pause, bool *reset);
void gera_blocos(Bloco blocos[COLUNA][LINHA], int *quant_blocos);
void exibe_blocos(Bloco blocos[COLUNA][LINHA]);
void detecta_colisao(Bloco blocos[COLUNA][LINHA], int *quant_blocos, bool *vitoria, bool *derrota, 
                    int bolaX, int bolaY, int *move_bolaX, int *move_bolaY, int *score);

int main(int argc, char *argv[]) {
    signal(SIGINT, catchSIGINT);
    if (KEY_open() == 0 || video_open() == 0 || accel_open() == 0) {
    printf("Erro na inicialização de periféricos.\n");
    return -1;
    }
    printf("Periféricos inicializados.\n");

    Bloco blocos[COLUNA][LINHA]; // 9x9 blocos

    video_erase();
    accel_init();
    accel_format(1, 2);
    accel_calibrate();

    int botoes, quant_blocos;
    bool pause = false, reset = false, fim_de_jogo = false, vitoria = false, derrota = false;
    int acel_rdy, acel_tap, acel_dtap, acel_x, acel_y, acel_z, acel_mg;
    int raquete_xi = (319 / 2) - 25;
    int raquete_xf = (319 / 2) + 25;
    int raquete_yi = 210 - 1;
    int raquete_yf = 210 + 1;
    int bolaX = (319 / 2);
    int bolaY = (200);
    int move_bolaX = 1;
    int move_bolaY = -1;
    int score = 0;
    char str[15];
    char comeco_str[] = "---PLAY---!";
    char vitoria_str[] = "YOU WIN!";
    char derrota_str[] = "YOU LOSE!";
    char pause_str[] = "PAUSE!";
    char fim_str[] = "RESET?";

    while (!sair) {
        printf("Tela inicial do jogo.\n");
        video_clear();
        video_erase();
        video_text(36, 45, comeco_str);
        video_show();

        printf("Aguardando início do jogo...\n");
        pause = true;
        while (pause && !sair) interpreta_botoes(&botoes, &pause, &reset);
        printf("Inicializando elementos de jogo...\n");
        
        video_erase();
        fim_de_jogo = false;
        vitoria = false;
        derrota = false;
        reset = false;

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
        while (!reset && !fim_de_jogo && !sair){
            interpreta_botoes(&botoes, &pause, &reset);
            if (pause) {
            printf("Programa pausado.\n");
            video_text(36, 45, pause_str);
            video_show();
            while (pause && !sair && !reset) interpreta_botoes(&botoes, &pause, &reset);
            video_erase();
            }
            
            video_clear();
            // Leitura do acelerômetro para movimentação da raquete (barra)
            accel_read(&acel_rdy, &acel_tap, &acel_dtap, &acel_x, &acel_y, &acel_z,
                    &acel_mg);

            video_box(0, 0, LARGURA_TELA, ALTURA_TELA,
                    video_BLACK); // Desenha o fundo da tela

            video_box(bolaX, bolaY, bolaX + 2, bolaY + 2, video_WHITE);
            // video_box(ALTURA_TELA - 2, LARGURA_TELA - 2, 2, 2, video_BLACK); //
            // Desenha a área útil e visível do jogo, criando bordas com o fundo da tela
            if (acel_x > -10 && acel_x < 10) {
            } else if (acel_x < 0 && raquete_xi > 7) {

            raquete_xi -= 3;
            raquete_xf -= 3;
            // Desenha o fundo da tela
            } else if (acel_x > 0 && raquete_xf < (319 - 13)) {

            raquete_xi += 3;
            raquete_xf += 3;
            } else {
            }

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

            video_box(raquete_xi, raquete_yi, raquete_xf, raquete_yf, video_WHITE);

            detecta_colisao(blocos, &quant_blocos, &vitoria, &derrota, bolaX, bolaY,
                            &move_bolaX, &move_bolaY, &score);
            sprintf(str, "Score: %d", score);
            video_text(4, 54, str);
            exibe_blocos(blocos); // Desenha os blocos

            video_show();

            // Fim movimentação da bola e detecção de colisão

       
            if (vitoria || derrota) fim_de_jogo = true;
        }
        //Fim rodada de jogo

        printf("Evento detectado. Fim de rodada de jogo.\n");
        video_clear();
        video_erase();
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

        printf("Aguardando ação do jogador...\n");
        reset = false;
        while (!reset && !sair) interpreta_botoes(&botoes, &pause, &reset);
        video_erase();
        if (reset) printf("Reiniciando jogo...\n");
    }

    printf("Encerrando...\n");
    KEY_close();
    video_close();
    accel_close();
    return 0;
}

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

void detecta_colisao(Bloco blocos[COLUNA][LINHA], int *quant_blocos,
                     bool *vitoria, bool *derrota, int bolaX, int bolaY,
                     int *move_bolaX, int *move_bolaY, int *score) {
  int i = 0;
  int j = 0;

  // Início colisão com blocos
  for (i = 0; i < LINHA; i++) {
    for (j = 0; j < COLUNA; j++) {
      if (!blocos[i][j].quebrado) {
        // Colisão com parte inferior ou superior do bloco
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
        // Colisão com parte lateral do bloco
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
