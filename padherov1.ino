#include <Adafruit_NeoPixel.h>
#include "notas.h"
#include "canciones.h"

//Declaraciones de tira de leds
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        10 //Pin datos a leds
#define NUMPIXELS 36//Numero total de pixeles
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//end of Declaraciones de tira de leds

//Plantilla matriz 4x9 SAMPLE
//Para saber qué numero corresponde a cada led
const int Matrix4x9[4][9] =
{
  {0,   1,  2,  3,  4,  5,  6,  7, 8},
  {17, 16, 15, 14, 13, 12, 11, 10, 9},
  {18, 19, 20, 21, 22, 23, 24, 25, 26},
  {35, 34, 33, 32, 31, 30, 29, 28, 27}

};
//Plantilla matriz 4x9 para control de quien está iluminado
//empiezan todos apagados
int ledPos[4][10] =
{
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

//Estados
const int ENCENDIENDO = 0;//Animacion de encendido
const int ELECCION = 1;   //Elegimos cancion
const int JUEGO = 2;      //Empieza el juego con la cancion seleccionada
const int FINAL = 3;      //Animacion de fallo y vuelta a ENCENDIENDO
const int PIANO = 4;      //Modo piano
const int SIMON = 5;

//Eventos
int estado = ENCENDIENDO; //Estado inicial
int timer = 0;            //Evento timer
int chequeo = 0;          //Evento interrupcion botones

//Fake timer
unsigned long periodo = 250;
unsigned long periodoant = 0;

//Contadores y demás variables globales
int contador_melodia = 0; //Actualizado con timer, para seguir transcurso de cancion
int menortono = 5000;     //Maxima frecuencia de tono
int mayortono = 0;        //Menor frecuencia de tono
int puntuacion_aux = 0;   //Auxiliar en recuento de puntuacion(tambien sirve para reproducir diferentes notas en estado JUEGO)
int puntuacion_total = 0; //Contador de puntuacion conseguida
int eleccion_cancion = 0; //'Circular' para elegir con qué canción jugar
int aux = 0;              //Contador auxiliar para animacion de puntuacion final
int aux_animation = 0;    //Contador auxiliar para diferentes animaciones
uint32_t aux_color = pixels.Color(random(255), random(255), random(255));//Color aleatorio animaciones eleccion
bool animation = HIGH;    //Bool para pintar ó borrar en animaciones
unsigned long  rebote = 0;
unsigned long  rebote2 = 0;
unsigned long  rebote3 = 0;
unsigned long  rebote4 = 0;

/*Piano*/
int c1 = 0;
int conter = 9;

/*Simon*/
const int MAX_LEVEL = 100;
int sequence[MAX_LEVEL];
int your_sequence[MAX_LEVEL];
int level = 1;
int velocity = 1000;

//auxiliares música. Son punteros a arrays en 'canciones.h'
int lenght = lenght1;
int *melody = melody1;
int *tempo = tempo1;


void setup()
{
  pixels.begin();                     // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(75);           //Set a brillo
  pixels.clear();                     //Limpia leds

  pinMode(11, OUTPUT);                //Zumbador
  pinMode(0, INPUT_PULLUP);           // boton 1
  pinMode(1, INPUT_PULLUP);           //boton 2
  pinMode(2, INPUT_PULLUP);           //boton 3
  pinMode(3, INPUT_PULLUP);           //boton 4
  attachInterrupt(digitalPinToInterrupt(0), Boton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(1), Boton2, FALLING);
  attachInterrupt(digitalPinToInterrupt(2), Boton3, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), Boton4, FALLING);
}
void(* resetFunc) (void) = 0;

void loop()
{
  //TODO Crear subfunciones para que máquina de estados no este tan cargada

  //Fake timer
  if (millis() - periodoant > periodo)
  {
    timer = 1;
  }

  switch (estado)
  {
    case (ENCENDIENDO):
      //Si el timer interrumpe
      if (timer)
      {

        //Cancion que suena en la intro
        //En cada interrupcion del timer sonara una nota diferente recorriendo el array de melodia cada vez.
        tone(11, intro_melody[contador_melodia], 1000 / intro_tempo[contador_melodia]);
        contador_melodia++;
        if (contador_melodia > intro_lenght)
          contador_melodia = 0;

        //"Animacion" mientras suena cancion de intro
        pixels.setPixelColor(Matrix4x9[random(4)][random(9)], pixels.Color(random(255), random(255), random(255)));
        pixels.show();
        //End of "Animacion" mientras suena cancion de intro

        periodoant = millis();
        timer = 0;  //Reseteamos interrupcion del timer
      }
      //Deteccion de flanco de bajada
      if (chequeo == 1 || chequeo == 4) //Cambiará a Guitar hero mode //BOTON 1
      {
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        contador_melodia = 0;
        pixels.clear();
        pixels.show();
        estado = ELECCION;
      }
      if (chequeo == 2) //Cambiará a Pianillo mode //BOTON 2
      {
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        pixels.clear();
        pixels.show();
        AnimacionPiano();
        estado = PIANO;
      }
      if (chequeo == 3) //Cambiará a SIMON DICE mode //BOTON 3
      {
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        pixels.clear();
        pixels.show();
        estado = SIMON;
      }

      break;

    case (PIANO):

      while (digitalRead(0) == LOW)
      {
        c1 = 1;
        tone(11, 440, 15);
      }
      while (digitalRead(1) == LOW)
      {
        c1 = 2;
        tone(11, 261.63, 25);
      }
      while (digitalRead(2) == LOW)
      {
        c1 = 3;
        tone(11, 392, 25);
      }
      while (digitalRead(3) == LOW)
      {
        c1 = 4;
        tone(11, 329.63, 25);
      }

      /***********************/
      if (c1 == 1)
      {

        Serial.println(conter);
        for (int i = 0; i < 4; i++)
        {
          Serial.println(Matrix4x9[i][conter]);
          pixels.setPixelColor(Matrix4x9[i][conter], pixels.Color(255, 255, 0));
        }
        conter++;
        c1 = 0;
      }
      if (c1 == 2)
      {

        Serial.println(conter);
        for (int i = 0; i < 4; i++)
        {
          pixels.setPixelColor(Matrix4x9[i][conter], pixels.Color(255, 0, 0));
        }
        conter++;
        c1 = 0;
      }
      if (c1 == 3)
      {

        Serial.println(conter);
        for (int i = 0; i < 4; i++)
        {
          pixels.setPixelColor(Matrix4x9[i][conter], pixels.Color(255, 0, 255));
        }
        conter++;
        c1 = 0;
      }
      if (c1 == 4)
      {

        Serial.println(conter);
        for (int i = 0; i < 4; i++)
        {
          pixels.setPixelColor(Matrix4x9[i][conter], pixels.Color(0, 255, 0));
        }
        conter++;
        c1 = 0;
      }
      if (conter >= 8)
      {
        conter = 0;
      }
      pixels.show();
      delay(50);
      break;

    case (SIMON):
      if (level == 1)
        generate_sequence();//generate a sequence;


      show_sequence();    //show the sequence
      get_sequence();     //wait for your sequence


      break;

    case (ELECCION)://Eleccion DE CANCION

      if (timer)
      {
        tone(11, melody[contador_melodia], 1000 / tempo[contador_melodia]);
        contador_melodia++;
        if (contador_melodia > lenght)
          contador_melodia = 0;

        periodoant = millis();
        timer = 0;
        //Añadir animacion mientras suena cancion a elegir??? TODO(¿¿nivel de dificultad por ejemplo??)


        if (animation)
        {
          pixels.setPixelColor(0 + aux_animation, aux_color);
          pixels.setPixelColor(35 - aux_animation, aux_color);

        }
        else
        {
          pixels.setPixelColor(0 + aux_animation, pixels.Color(0, 0, 0));
          pixels.setPixelColor(35 - aux_animation, pixels.Color(0, 0, 0));
        }

        pixels.show();
        aux_animation++;
        if (aux_animation >= 18)
        {
          animation = !animation;
          aux_animation = 0;
        }

      }

      if (chequeo == 1) //boton 1
      {
        pixels.clear();
        animation = HIGH;
        aux_color = pixels.Color(random(255), random(255), random(255));
        aux_animation = 0;
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        eleccion_cancion++;
        eligeMelodia();
        contador_melodia = 0;
        //Cancion siguiente TODO

      }
      if ((chequeo == 2) || (chequeo == 3))
      {
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        //confirma cancion y estado siguiente
        calculaRangos();                    //Calcula, dependiendo del valor de notas(frecuencias) que haya en cancion, en que fila irá "entrará" cada nota
        contador_melodia = 0;
        periodo = 500;
        estado = JUEGO;
        pixels.clear();
        pixels.show();
      }
      if (chequeo == 4)
      {
        pixels.clear();
        aux_color = pixels.Color(random(255), random(255), random(255));
        aux_animation = 0;
        animation = HIGH;
        chequeo = 0;
        tone(11, 3128, 1000 / 4); //nota que suena al pulsar
        eleccion_cancion--;
        eligeMelodia();
        contador_melodia = 0;
        //Cancion anterior TODO
      }

      break;

    case (JUEGO)://**JUEGO**JUEGO**JUEGO**JUEGO**JUEGO**JUEGO**JUEGO**JUEGO**//
      if (timer)
      {
        updateLEDS();
        addMelodyLED();

        contador_melodia++;
        //para hacer melodia cíclica
        if (contador_melodia >= lenght)
          contador_melodia = 0;

        periodoant = millis();
        timer = 0;
      }

      if (ledPos[0][7] == 1 && chequeo == 1) //boton 1
      {
        chequeo = 0;
        ledPos[0][7] = 0;
        tone(11, melody[puntuacion_aux], 1000 / tempo[puntuacion_aux]);
        puntuacion_aux++;
        periodo--;
        pixels.setPixelColor(Matrix4x9[0][7], pixels.Color(0, 0, 0));
        pixels.setPixelColor(Matrix4x9[0][8], pixels.Color(0, 255, 0));
        pixels.show();

      }
      else if (ledPos[1][7] == 1 && chequeo == 2) //boton 2
      {
        chequeo = 0;
        ledPos[1][7] = 0;
        tone(11, melody[puntuacion_aux], 1000 / tempo[puntuacion_aux]);
        puntuacion_aux++;
        periodo--;
        pixels.setPixelColor(Matrix4x9[1][7], pixels.Color(0, 0, 0));
        pixels.setPixelColor(Matrix4x9[1][8], pixels.Color(0, 255, 0));
        pixels.show();
      }
      else if (ledPos[2][7] == 1 && chequeo == 3) //boton 3
      {
        chequeo = 0;
        ledPos[2][7] = 0;
        tone(11, melody[puntuacion_aux], 1000 / tempo[puntuacion_aux]);
        puntuacion_aux++;
        periodo--;
        pixels.setPixelColor(Matrix4x9[2][7], pixels.Color(0, 0, 0));
        pixels.setPixelColor(Matrix4x9[2][8], pixels.Color(0, 255, 0));
        pixels.show();
      }
      else if (ledPos[3][7] == 1 && chequeo == 4) //boton 4
      {
        chequeo = 0;
        ledPos[3][7] = 0;
        tone(11, melody[puntuacion_aux], 1000 / tempo[puntuacion_aux]);
        puntuacion_aux++;
        periodo--;
        pixels.setPixelColor(Matrix4x9[3][7], pixels.Color(0, 0, 0));
        pixels.setPixelColor(Matrix4x9[3][8], pixels.Color(0, 255, 0));
        pixels.show();
      }
      else if (chequeo || ledPos[0][8] == 1 || ledPos[1][8] == 1 || ledPos[2][8] == 1 || ledPos[3][8] == 1 )
      {
        pixels.clear();
        pixels.show();
        puntuacion_total += puntuacion_aux;
        chequeo = 0;

        puntuacion_aux = 0;
        for (unsigned i = 0; i <= 35; i++)
        {
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        tone(11, 50, 500); // Play thisNote for duration.
        delay(500);
        pixels.clear();
        pixels.show();
        estado = FINAL;
      }


      if (puntuacion_aux > lenght)
      {
        puntuacion_total += puntuacion_aux;
        puntuacion_aux = 0;
      }

      break;

    case (FINAL):


      while (aux < puntuacion_total / 5)
      {
        tone(11, aux * 100, 80); //cada vez más agudo
        pixels.setPixelColor(aux, pixels.Color(255, 255, 255));
        pixels.show();
        aux++;
        delay(100);//Es el unico momento donde no me importa hacer delay...(Para no complicar más con flags de timer...)
        chequeo = 0;
      }
      if (chequeo)
      {
        resetFunc();
      }
      break;
  }
}

void updateLEDS()
{
  //RECORRE MATRIZ AUXILIAR(LAS FILAS DEBEN IR DE MÁS A MENOS
  //PARA QUE LEDS TENGAN EFECTO DE GRAVEDAD)
  for (int i = 0; i < 4 ; i++)
  {
    for (int j = 9; j >= 0; j--)
    {

      if (j == 0)
        ledPos[i][j] = 0;
      else
        ledPos[i][j] = ledPos[i][j - 1];
    }
  }
  //RECORRE Y ENCIENDE/APAGA LEDS CONFORME A MATRIZ AUXILIAR
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 9; j++)
    {
      if (ledPos[i][j] == 0) //apaga led
      {
        pixels.setPixelColor(Matrix4x9[i][j], pixels.Color(0, 0, 0));
      }
      else
      { //enciende leds
        pixels.setPixelColor(Matrix4x9[i][j], pixels.ColorHSV(i * 16000, 255, 255));
      }
    }
  }
  pixels.show();
}

//Recorriendo el array de frecuencias en la melodía
void addMelodyLED()
{
  int columna = map (melody[contador_melodia], menortono, mayortono, 0, 3);
  if (melody[contador_melodia] == 0)
  {
    //No añade ningún led
  }
  else
  {
    ledPos[columna][0] = 1;
    pixels.setPixelColor(Matrix4x9[columna][0], pixels.ColorHSV(columna * 16000, 255, 255));
    pixels.show();
  }
}

void calculaRangos()
{
  menortono = 5000;
  mayortono = 0;
  for (int i = 0; i < lenght; i++)
  {
    if (melody[i] != 0)
    {
      if (mayortono < melody[i])
        mayortono = melody[i];
      if (menortono > melody[i])
        menortono = melody[i];
    }
  }
}

//Funciones asociadas a interrupcion de botones
void Boton1()
{

  static unsigned long ultimo_rebote = 0;
  rebote = millis();

  if (rebote - ultimo_rebote > 250)
  {

    chequeo = 1;

  }
  ultimo_rebote = rebote;
}

void Boton2()
{
  static unsigned long ultimo_rebote2 = 0;
  rebote2 = millis();

  if (rebote2 - ultimo_rebote2 > 250)
  {

    chequeo = 2;

  }
  ultimo_rebote2 = rebote2;
}

void Boton3()
{
  static unsigned long ultimo_rebote3 = 0;
  rebote3 = millis();

  if (rebote3 - ultimo_rebote3 > 250)
  {

    chequeo = 3;

  }
  ultimo_rebote3 = rebote3;
}

void Boton4()
{
  static unsigned long ultimo_rebote4 = 0;
  rebote4 = millis();

  if (rebote4 - ultimo_rebote4 > 250)
  {

    chequeo = 4;

  }
  ultimo_rebote4 = rebote4;
}
void eligeMelodia()
{
  if (eleccion_cancion < 0)
    eleccion_cancion = 5;

  if (eleccion_cancion > 7)
    eleccion_cancion = 0;

  if (eleccion_cancion == 0)
  {
    melody = melody1;
    tempo = tempo1;
    lenght = lenght1;
  }
  if (eleccion_cancion == 1)
  {
    melody = melody2;
    tempo = tempo2;
    lenght = lenght2;
  }
  if (eleccion_cancion == 2)
  {
    melody = melody3;
    tempo = tempo3;
    lenght = lenght3;
  }
  if (eleccion_cancion == 3)
  {
    melody = melody4;
    tempo = tempo4;
    lenght = lenght4;
  }
  if (eleccion_cancion == 4)
  {
    melody = melody5;
    tempo = tempo5;
    lenght = lenght5;
  }
  if (eleccion_cancion == 5)
  {
    melody = intro_melody;
    tempo = intro_tempo;
    lenght = intro_lenght;
  }
}
void AnimacionPiano()
{
  pixels.setPixelColor(8, pixels.Color(255, 255, 0));
  pixels.setPixelColor(9, pixels.Color(255, 0, 0));
  pixels.setPixelColor(26, pixels.Color(255, 0, 255));
  pixels.setPixelColor(27, pixels.Color(0, 255, 0));
  pixels.show();
}
void show_sequence()
{
  pixels.clear();
  pixels.show();

  for (int i = 0; i < level; i++)
  {
    if (sequence[i] == 1)
    {
      //Enciende led0
      for (int i = 0; i <= 8; i++)
        pixels.setPixelColor(i, pixels.Color(255, 255, 0));

      tone(11, 440, 50);
    }
    if (sequence[i] == 2)
    {
      //Enciende led1
      for (int i = 9; i <= 17; i++)
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
      tone(11, 261.63, 50);
    }
    if (sequence[i] == 3)
    {
      //Enciende led2
      for (int i = 18; i <= 26; i++)
        pixels.setPixelColor(i, pixels.Color(0, 255, 0));
      tone(11, 392, 50);
    }
    if (sequence[i] == 4)
    {
      //Enciende led3
      for (int i = 27; i <= 35; i++)
        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
      tone(11, 329.63, 50);
    }
    pixels.show();

    delay(velocity);

    pixels.clear();
    pixels.show();
    delay(200);
  }
  chequeo == 0;
}

void get_sequence()
{
  int flag = 0; //this flag indicates if the sequence is correct

  for (int i = 0; i < level; i++)
  {
    flag = 0;
    while (flag == 0)
    {
      if (chequeo == 1)
      {
        chequeo = 0;
        for (int i = 0; i <= 8; i++)
          pixels.setPixelColor(i, pixels.Color(255, 255, 0));

        pixels.show();
        tone(11, 440, 50);

        your_sequence[i] = 1;
        flag = 1;
        delay(200);
        if (your_sequence[i] != sequence[i])
        {
          wrong_sequence();
          return;
        }
        pixels.clear();
        pixels.show();
      }
      if (chequeo == 2)
      {
        chequeo = 0;
        for (int i = 9; i <= 17; i++)
          pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();
        tone(11, 261.63, 50);

        your_sequence[i] = 2;
        flag = 1;
        delay(200);
        if (your_sequence[i] != sequence[i])
        {
          wrong_sequence();
          return;
        }
        pixels.clear();
        pixels.show();
      }
      if (chequeo == 3)
      {
        chequeo = 0;
        for (int i = 18; i <= 26; i++)
          pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        pixels.show();
        tone(11, 392, 50);

        your_sequence[i] = 3;
        flag = 1;
        delay(200);
        if (your_sequence[i] != sequence[i])
        {
          wrong_sequence();
          return;
        }
        pixels.clear();
        pixels.show();
      }
      if (chequeo == 4)
      {
        chequeo = 0;
        for (int i = 27; i <= 35; i++)
          pixels.setPixelColor(i, pixels.Color(0, 0, 255));
        pixels.show();
        tone(11, 329.63, 50);

        your_sequence[i] = 4;
        flag = 1;
        delay(200);
        if (your_sequence[i] != sequence[i])
        {
          wrong_sequence();
          return;
        }
        pixels.clear();
        pixels.show();
      }
    }
  }
  right_sequence();
}

void generate_sequence()
{
  randomSeed(millis()); //in this way is really random!!!

  for (int i = 0; i < MAX_LEVEL; i++)
  {
    sequence[i] = random(1, 5);
    Serial.print(sequence[i]);
  }
}
void wrong_sequence()
{
  for (unsigned i = 0; i <= 35; i++)
  {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }
  pixels.show();
  tone(11, 50, 500); // Play thisNote for duration.
  delay(500);
  pixels.clear();
  pixels.show();

  puntuacion_total = (level - 1) * 5;
  estado = FINAL;
}

void right_sequence()
{

  pixels.clear();
  pixels.show();
  delay(250);

  for (int i = 0; i <= 35; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
  }
  pixels.show();

  tone(11, 1080, 500);
  delay(500);

  pixels.clear();
  pixels.show();
  delay(500);

  if (level < MAX_LEVEL)
    level++;

  velocity -= 50; //increase difficulty
}
