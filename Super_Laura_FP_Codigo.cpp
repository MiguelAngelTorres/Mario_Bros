/*******************************************/
        // M A R I O   B R O S //
/*******************************************/

#include <iostream>
#include <fstream>
#include <list>
#include <allegro.h>

using namespace std;

BITMAP *buffer;

// MARIO BROS PERSONAJE
BITMAP *mario;
BITMAP *mario_saltabmp;
BITMAP *mario_andabmp;
BITMAP *mario_paradobmp;

// Enemigo
BITMAP *ghos;
BITMAP *ghosbmp;

// Escenario
BITMAP *fondo, *ladrillo, *escena, *cielo, *ladrillobmp;
BITMAP *cesped1, *cesped2, *tierra1, *tierra2;
BITMAP *fondo_negro, *disco, *lad_azul, *cuadrado;
BITMAP *tubo, *tubo_ibmp, *tubo_dbmp, *canion_bmp;

BITMAP *bala_bmp, *_bala, *explo;
BITMAP *mario_init;
BITMAP *saltador;
BITMAP *puerta_bmp, *puerta;


// Sonidos
MIDI *musica;
SAMPLE *saltar_music, *sound_explo, *morir;
SAMPLE *pacman_sound, *sirena, *jumping;

enum Direccion{IZDA, DCHA};

// Dirección de inicio del personaje
int inicio = 4;//818;

// Var. globales

// Posición y dirección de Mario
Direccion dir = DCHA;
int x = inicio, y = 0;
int x_ant = x;

// Estado de Mario
bool subir = false, caer = false, parado = true;
bool game_over = false;
bool paralizado = false;

int x_a, y_a;
int n = 0;
int coord_map = 0;
int saltar = 0, bajar = 3, andar = 0, parar = 0;
int parar_aux = 0;
int tiempo_salto = 0;

class Bala;
list<Bala> lista_balas;
list<Bala>::iterator it;

char mapa[200][1024];


// Clase Enemigo
struct Enemigo{
  int x_inicial;
  double x;
  int y;
  int n;
  Direccion d;

  Enemigo(int _x_i, double _x, int _y, int _n, Direccion _d):
          x_inicial(int(_x)), x(_x), y(_y), n(_n), d(_d){}
};

list<Enemigo> lista_enemigos;
list<Enemigo>::iterator it_eneg;

// Función leer doc. texto y almacenarlo en una cadena
// Está función será usada para leer el fichero donde está almacenado el mapa
void _READ_FILE(const char *file, char cadena[][1024]){

     for(int i = 0; i < 200; i++)
         for(int j = 0; j < 1024; j++)
             cadena[i][j] = ' ';

     ifstream f(file);
     istream& is = f;

     int i = 0;

     while(!is.eof()){
           is.getline(cadena[i], 1024);
           strcat(cadena[i], "\n");
           i++;
     }
     strcat(cadena[i], "\0");
     f.close();
}

// Función para inicializar allegro
int init(){
     allegro_init();
     install_keyboard();

     set_color_depth(32);
     set_gfx_mode(GFX_AUTODETECT_WINDOWED, 600, 330, 0, 0);

     if(install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0){
       allegro_message("Error: inicializando sistema de sonido\n%s\n", allegro_error); return 1;
    }

    set_volume(70, 70);
    return 0;
}

// Cargar imagenes a usar en el juego y los buffers para mostrar dichas imag
void _LOAD_IMAGE(){
     cielo = load_bitmap("img/fondo_cielo.bmp", NULL);
     mario_init = load_bitmap("img/mario_init.bmp", NULL);
     bala_bmp = load_bitmap("img/bala.bmp", NULL);
     disco = load_bitmap("img/disco.bmp", NULL);
     lad_azul = load_bitmap("img/roca4.bmp", NULL);
     canion_bmp = load_bitmap("img/canion.bmp", NULL);
     saltador = load_bitmap("img/saltador.bmp", NULL);
     puerta_bmp = load_bitmap("img/puerta.bmp", NULL);
     puerta = create_bitmap(42,42);
     buffer = create_bitmap(20*30, 11*30);
     escena = create_bitmap(20*30, 11*30);
     ladrillo = create_bitmap(30,30);
     _bala = create_bitmap(34,34);
     cuadrado = create_bitmap(30,30);
     explo = create_bitmap(34,34);
     ladrillobmp = load_bitmap("img/roca.bmp", NULL);
     ghosbmp = load_bitmap("img/ghos.bmp", NULL);
     ghos = create_bitmap(33,34);
     fondo = load_bitmap("img/fondo_mario.bmp", NULL);
     cesped1 = load_bitmap("img/cesped1.bmp", NULL);
     cesped2 = load_bitmap("img/cesped2.bmp", NULL);
     tierra1 = load_bitmap("img/tierra1.bmp", NULL);
     tierra2 = load_bitmap("img/tierra2.bmp", NULL);
     mario = create_bitmap(34, 34);
     mario_saltabmp = load_bitmap("img/Mario_saltando.bmp", NULL);
     mario_andabmp = load_bitmap("img/Mario_andando.bmp", NULL);
     mario_paradobmp = load_bitmap("img/Mario_parado.bmp", NULL);
     fondo_negro = load_bitmap("img/fondo_negro.bmp", NULL);
     tubo = create_bitmap(44,33);
     tubo_ibmp = load_bitmap("img/tubo_izq.bmp", NULL);
     tubo_dbmp = load_bitmap("img/tubo_dcha.bmp", NULL);
}

// Cargar sonidos del juego
void _LOAD_SOUND(){
     saltar_music = load_wav("Sound/jump.wav");
     sound_explo = load_wav("Sound/explosion.wav");
     morir = load_wav("Sound/Muerte.wav");
     musica = load_midi("Sound/mario.mid");
     pacman_sound = load_wav("sound/pacman.wav");
     sirena = load_wav("sound/siren.wav");
     jumping = load_wav("sound/victory.wav");
}

bool Puede_pasar(int _x, int _y){
     if(mapa[_y][_x] != 'X' && mapa[_y][_x] != 'T' && mapa[_y][_x] != 'L' && mapa[_y][_x] != 'x'
        && mapa[_y][_x] != 'c' && mapa[_y][_x] != 'd' && mapa[_y][_x] != 'A')
        return true;
     else
        return false;
}

int cont_puerta = 0;
void Inicializar_enemigos();
void Inicializar_canion();

void pintar_puerta(int j, int i, bool encima){

     if(encima){
         paralizado = true;
         cont_puerta++;

         if(cont_puerta < 12)
             blit(puerta_bmp, puerta, 0, 42*(cont_puerta/3), 0, 0, 42, 42);
         else{
            if(mapa[j][i] == '1')
            _READ_FILE("mapa1.txt", mapa);
            else if(mapa[j][i] == '2')
            _READ_FILE("mapa2.txt", mapa);
            paralizado = false;
            cont_puerta = 0;
            Inicializar_enemigos();
            Inicializar_canion();
            lista_balas.clear();
         }
     }
     else
         blit(puerta_bmp, puerta, 0, 0, 0, 0, 42, 42);

     draw_sprite(buffer, puerta, (i-x+10)*30, j*30-12);

}

// Busca la posición de los enegmigos y la inicializa
void Inicializar_enemigos(){
     for(int j = 0; j < 200; j++)
         for(int i = 0; i < 1024; i++)
             if(mapa[j][i] == 'p'){
                Enemigo eneg(i, i, j, 0, DCHA);
                lista_enemigos.push_back(eneg);
             }
}


void Dibujar_eneg(int ex, int ey, int k, Direccion d){
     int p = 0;

     if(x > 266) p = 68;

     if(d == DCHA)
        blit(ghosbmp, ghos, k*33+p, 34, 0, 0, 33, 34);
     else
        blit(ghosbmp, ghos, k*33+p, 0, 0, 0, 33, 34);

     draw_sprite(buffer, ghos, ex, ey);
}


void Mover_Enemigos(){
     for(it_eneg = lista_enemigos.begin(); it_eneg != lista_enemigos.end(); ++it_eneg){

           if(it_eneg->n % 2 == 0){
               if(it_eneg->d == DCHA && Puede_pasar(int(it_eneg->x)+1, it_eneg->y) && mapa[it_eneg->y+1][int(it_eneg->x) + 1] != ' ')
                  (it_eneg->x)+=0.25;
               else
                  it_eneg->d = IZDA;

               if(it_eneg->d == IZDA && Puede_pasar(int(it_eneg->x)-1, it_eneg->y) && mapa[it_eneg->y+1][int(it_eneg->x) - 1] != ' ')
                  (it_eneg->x)-=0.25;
               else
                  it_eneg->d = DCHA;
           }

           (it_eneg->n)++;

            if(it_eneg->n == 4)
               it_eneg->n = 0;

     }
}


void DRAW_MAP(){
     blit(cielo, buffer, coord_map, 0 ,0 ,0 , 600, 330);

     if(x < 260)
        blit(fondo, escena, coord_map, 0 ,0 ,0 , 600, 330);
     else if(x < 805)
        blit(fondo_negro, escena, coord_map, 0 ,0 ,0 , 600, 330);
     else
        blit(disco, escena, 0, 0 ,0 ,0 , 600, 330);

     draw_sprite(buffer, escena, 0, 0);

     for(int i = x-10; i < x+10; i++){
         for(int j = 0; j < 11; j++){
             if(mapa[j][i] == 'X'){
                if(i%2 == 0)
                   draw_sprite(buffer, cesped1, (i-x+10)*30, j*30);
                else
                   draw_sprite(buffer, cesped2, (i-x+10)*30, j*30);
             }
             if(mapa[j][i] == 'T'){
                if(i%2 == 0)
                   draw_sprite(buffer, tierra1, (i-x+10)*30, j*30);
                else
                   draw_sprite(buffer, tierra2, (i-x+10)*30, j*30);
             }
             if(mapa[j][i] == 'L'){
                if(i%2 == 0)
                   blit(ladrillobmp, ladrillo, 0, 0 ,0 ,0 , 30, 30);
                else
                   blit(ladrillobmp, ladrillo, 30, 0 ,0 ,0 , 30, 30);

                draw_sprite(buffer, ladrillo, (i-x+10)*30, j*30);
             }
             if(mapa[j][i] == 'c' || mapa[j][i] == 'd'){
                draw_sprite(buffer, canion_bmp, (i-x+10)*30, j*30);
             }
             if(mapa[j][i] == 's'){
                draw_sprite(buffer, saltador, (i-x+10)*30, j*30);
             }
             if(mapa[j][i] == 'A'){
                blit(lad_azul, cuadrado, ((j+i)%2)*30, 0 ,0 ,0 , 30, 30);
                draw_sprite(buffer, cuadrado, (i-x+10)*30, j*30);
             }
             for(int w = 1; w < 10; w++)
                 if(mapa[j][i] == w+'0'){
                    if(x == i && y == j)
                       pintar_puerta(j, i, true);
                    else
                       pintar_puerta(j, i, false);
                 }
         }
     }
}


void DRAW_PIPES(){
     for(int i = x-10; i < x+10; i++){
         for(int j = 0; j < 11; j++){
             if(mapa[j][i] == 'D')
                draw_sprite(buffer, tubo_dbmp, (i-x+10)*30, int((j-0.5)*30));
             if(mapa[j][i] == 'I')
                draw_sprite(buffer, tubo_ibmp, (i-x+10)*30, int((j-0.5)*30));
         }
     }
}


void Morir(){
     lista_balas.clear();
     game_over = false;
     paralizado = false;
     x = inicio, y = 1;
     play_sample(morir, 300, 150, 1000, 0);
     clear(buffer);
     rest(1000);
}

void _SCREEN(){
     blit(buffer, screen, 0, 0 ,0 ,0 , 600, 330);
}

void DRAW_ENEMY_AND_CHECK_COLISION(){
     int aux = 0;

     for(it_eneg = lista_enemigos.begin(); it_eneg != lista_enemigos.end(); ++it_eneg){
           if(it_eneg->n > 1) aux = 1;

           Dibujar_eneg(int((it_eneg->x-x+10)*30), it_eneg->y*30, aux, it_eneg->d);


           if(int(it_eneg->x) == x && it_eneg->y == y || int(it_eneg->x) == x_a && it_eneg->y == y_a)
                 game_over = true;

     }
}



/******* MARIO BROS ********/

void DRAW_MARIO(){
     if(subir && dir == DCHA)
         blit(mario_saltabmp, mario, saltar*34, 0, 0, 0, 34, 34);
     else if(caer&& dir == DCHA)
         blit(mario_saltabmp, mario, bajar*34, 0, 0, 0, 34, 34);
     else if(parado && dir == DCHA)
         blit(mario_paradobmp, mario, parar*34, 0, 0, 0, 34, 34);
     else if(dir == DCHA)
         blit(mario_andabmp, mario, andar*34, 0, 0, 0, 34, 34);
     else if(subir && dir == IZDA)
         blit(mario_saltabmp, mario, saltar*34, 34, 0, 0, 34, 34);
     else if(caer&& dir == IZDA)
         blit(mario_saltabmp, mario, bajar*34, 34, 0, 0, 34, 34);
     else if(parado && dir == IZDA)
         blit(mario_paradobmp, mario, parar*34, 34, 0, 0, 34, 34);
     else if(dir == IZDA)
         blit(mario_andabmp, mario, andar*34, 34, 0, 0, 34, 34);

     draw_sprite(buffer, mario, 10*30,y*30 );

}

/******** MOVIMIENTOS MARIO ********/

void FALL(){
     if(Puede_pasar(x, y+1)){
        y++;
        bajar++;
        if(bajar >= 6)
           bajar = 3;
     }
     else{
        caer = false;
        bajar = 3;
     }
}

void JUMP(){
     if(Puede_pasar(x, y-1)){
        play_sample(saltar_music, 300, 150, 1000, 0);
        n++;
        y--;
        subir = true;
     }
}

void SUBIR(){

     int s = 3;

     if(tiempo_salto > 0) s = 6;

     if(Puede_pasar(x, y-1) && n < s){
        saltar++;
        n++;
        y--;
     }
     else{
        caer = true;
        subir = false;
        n = 0;
        saltar = 0;
     }
}

void MOVE_LEFT(){
     dir = IZDA;
     if(Puede_pasar(x-1, y)){
        andar++;
        x--;
        coord_map--;

        if(coord_map < 0 && x < 260)
           coord_map = 391;
        else if(coord_map < 0 && x > 260)
           coord_map = 198;
     }
}

void MOVE_RIGHT(){
     dir = DCHA;
     if(Puede_pasar(x+1, y)) {
        andar++;
        x++;
        coord_map++;
        if(coord_map > 391 && x < 260)
           coord_map = 0;
        else if(coord_map > 198 && x > 260)
           coord_map = 0;

     }
}

void Disparar(int _x, int _y, Direccion direct, bool maldad = false);

int puedo_disparar = 0;

void MOVE_MARIO(){
     if(caer) FALL();

     if(subir)
        SUBIR();
     if(key[KEY_UP] && !subir && !caer)
        JUMP();
     if(key[KEY_RIGHT])
        MOVE_RIGHT();
     if(key[KEY_LEFT])
        MOVE_LEFT();

     if(key[KEY_SPACE])
        if(puedo_disparar == 0){
           Disparar(x, y, dir);
           puedo_disparar = 4;
        }

     if(Puede_pasar(x, y+1) && !subir)
        caer = true;

     if(andar >= 6)
        andar = 0;
}

void pantalla(){
     clear(buffer);
     DRAW_MAP();
     if(!paralizado) DRAW_MARIO();
     DRAW_ENEMY_AND_CHECK_COLISION();
     DRAW_PIPES();
}

void teletransporte_tubo(){
      if(mapa[y][x] == '-'){
         pantalla();
         _SCREEN();
         rest(500);

         bool salir = false;
         int n = -1;
         if(dir == DCHA) n = 1;

         for(int j = x+n; !salir; j+=n)
                 for(int i = 0; i < 20 && !salir; i++)
                     if(mapa[i][j] == '-'){
                        y = i;
                        x = j;
                        salir = true;
                     }

         x+=n;
         pantalla();
         _SCREEN();
         rest(1000);

         x+=n; pantalla();
         _SCREEN();
         rest(200);
     }
}

class Bala{
      private:
         double bx;
         int by;
         int k;
         bool a_tomar_por_culo, explosion;
         bool maldad;
         Direccion d;

      public:
         Bala(double _x, int _y, Direccion _d, bool Maldad): bx(_x), by(_y), d(_d), k(0), maldad(Maldad), a_tomar_por_culo(false), explosion(false){}

         void Mover_bala(){
              if(!explosion){
                  k++;

                  if(d == DCHA) bx+=1.5;
                  else          bx-=1.5;
              }
         }

         void Pintar_bala(){
              if(!explosion){
                      if(k%2==0){
                          if(d == DCHA)
                             blit(bala_bmp, _bala, 34, 0, 0, 0, 34, 34);
                          else
                             blit(bala_bmp, _bala, 0, 0, 0, 0, 34, 34);
                      }
                      else{
                          if(d == DCHA)
                             blit(bala_bmp, _bala, 34, 34, 0, 0, 34, 34);
                          else
                             blit(bala_bmp, _bala, 0, 34, 0, 0, 34, 34);
                           }

                      draw_sprite(buffer, _bala, int((bx-x+10)*30), by*30);
              }
              else{
                   blit(bala_bmp, _bala, k*34, 0, 0, 0, 34, 34);
                   draw_sprite(buffer, _bala, int((bx-x+10)*30), by*30);
                   k++;
                   if(k == 11) a_tomar_por_culo = true;
              }

         }

         bool destruir(){
               if(paralizado && a_tomar_por_culo) game_over = true;

               if(a_tomar_por_culo)
                  return true;

               if(!maldad){
                   if(int(bx)-x > 10 || x-int(bx) > 10 )
                      return true;
               }else{
                   if(int(bx)-x > 50 || x-int(bx) > 50 )
                      return true;
               }

               if((mapa[by][int(bx)] != 'X' && mapa[by][int(bx)] != 'T' && mapa[by][int(bx)] != 'L' && mapa[by][int(bx)] != 'x'))
                  return false;
               else if(!explosion){
                  explosion = true;
                  k = 2;
                  play_sample(sound_explo, 300, 150, 1000, 0);
                  return false;
               }
               else
                  return false;
         }

         void matar_enemigo(){
              for(it_eneg = lista_enemigos.begin(); it_eneg != lista_enemigos.end(); ++it_eneg){
                  if(it_eneg->y == by &&(int(it_eneg->x) == int(bx) || int(it_eneg->x) == int(bx+0.6) || int(it_eneg->x) == int(bx-0.6))){
                     it_eneg == lista_enemigos.erase(it_eneg);
                     explosion = true;
                     bx = it_eneg->x;
                     k = 2;
                     play_sample(sound_explo, 300, 150, 1000, 0);
                     break;
                  }
              }
         }

         void matar_mario(){
              if(by == y && (int(bx) == x || int(bx-0.5) == x || int(bx+0.5) == x) && !explosion){
                 paralizado = true;
                 k = 2;
                 explosion = true;
                 play_sample(sound_explo, 300, 150, 1000, 0);
              }
         }

         void matar_objetivo(){
              if(maldad) matar_mario();
              else       matar_enemigo();
         }
};

void Disparar(int _x, int _y, Direccion direct, bool maldad){
     Bala b(_x, _y, direct, maldad);
     lista_balas.push_back(b);
}

class canion{
      private:
         int cx, cy;
         Direccion d;
         int temporizador;
         int frec;

      public:
         canion(int _x, int _y, Direccion _d): cx(_x), cy(_y), d(_d), temporizador(0), frec(20){}

         void Disparo(){
              if(temporizador == frec){
                 Disparar(cx, cy, d, true);
                 play_sample(sound_explo, 300, 150, 1000, 0);
                 temporizador = 0;
              }
              else
                 temporizador++;
         }

         int pos(){
             return cx;
         }
};

list<canion> lista_canion;
list<canion>::iterator it_canion;

void Inicializar_canion(){
     for(int j = 0; j < 200; j++)
         for(int i = 0; i < 1024; i++)
             if(mapa[j][i] == 'c'){
                canion ca(i, j, IZDA);
                lista_canion.push_back(ca);
             }
             else if(mapa[j][i] == 'd'){
                canion ca(i, j, DCHA);
                lista_canion.push_back(ca);
             }
}

int main(){
    init();

    _LOAD_IMAGE();
    _LOAD_SOUND();
    _READ_FILE("inicio.txt", mapa);
    _SCREEN();
    play_sample(pacman_sound, 300, 150, 1000, 0);
    clear(buffer);
    blit(mario_init, buffer, 0, 0 ,0 ,0 , 600, 330);
    _SCREEN();
    rest(4000);
    Inicializar_enemigos();
    Inicializar_canion();


    play_midi(musica, 1);
    while(!key[KEY_ESC]){
          x_a = x;
          y_a = y;

          if(x > 680) inicio = 680;
          else if(x > 540) inicio = 540;
          else if(x > 270) inicio = 271;

          if(mapa[y][x] == '.')
             game_over = true;

          teletransporte_tubo();

          x_ant = x;
          clear(buffer);
          if(!paralizado){
              MOVE_MARIO();
              Mover_Enemigos();
          }

          if(x_ant == x){
             parado = true;
             parar_aux++;

             if(parar_aux % 2 == 0)
                parar++;

             if(parar == 5)
                parar = 0;
          }
          else
             parado = false;

          pantalla();
          for(it = lista_balas.begin(); it!= lista_balas.end(); ++it){
              if(it->destruir()){
                 it = lista_balas.erase(it);
              }
              else{
                 it->Mover_bala();
                 it->Pintar_bala();
                 it->matar_objetivo();
              }
          }

          for(it_canion = lista_canion.begin(); it_canion!= lista_canion.end(); ++it_canion){
              if(it_canion->pos() < x+50 && it_canion->pos() > x-50)
                 it_canion->Disparo();
          }

          _SCREEN();
          rest(80);

          if(game_over){
             Morir();
             lista_enemigos.clear();
             Inicializar_enemigos();
          }

          if(puedo_disparar > 0)
             puedo_disparar--;


          if(mapa[y][x] == 's'){
             tiempo_salto = 100;
             play_sample(jumping, 300, 150, 1000, 0);
          }

          if(tiempo_salto > 0) tiempo_salto--;
          if(tiempo_salto == 20) play_sample(sirena, 300, 150, 1000, 0);

    }

}
END_OF_MAIN();















