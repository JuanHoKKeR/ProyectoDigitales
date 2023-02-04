#include "mbed.h"
#include "TextLCD.h"
#include "ctype.h"
#include <cctype> //Libreria util para modificar cadena de caracteres, nativa
#define SDA PB_9
#define SCL PB_8
#define SLAVEADDRESS 0x27 // Direccion I2C de la LCD //Es necesario modificar una linea del codigo de la libreria para LCDI2C
#define Tiempo_Medida 500ms
//----------------------------------------------------------------
I2C i2c_lcd(SDA, SCL); // Inicializacion de la terminal I2C
static BufferedSerial serial(PA_2, PA_3); //Transmicion Serial con el PC
TextLCD_I2C lcd(&i2c_lcd,SLAVEADDRESS,  TextLCD::LCD20x4); //Inicializacion de la lcd, i2c, direccion y tipo de lcd
PwmOut Pinsalida(D6);
DigitalOut Trg(PA_10); //D2
DigitalIn Echo(PB_3);//D3
//----------------------------------------------------------------
char men_in[16];
long int li1=0;
char prueba[10]={0};
float distancia_cm;
char mensaje[40]={0};

// Prototipos de las funciones 
void rx_serie(void);
void servomotor(void);
void DisparoEntrada();
//-----------------------------------------------------------------
Thread hilo_servo;
Thread hilo_serie;
Thread hilo_DisparoEntrada;
//----------------------------------------------------------------
Timer tiempo1;



int main()
{
    lcd.setBacklight(TextLCD::LightOn); //LightOff , LightOn //NO HIZO NADA :V
    //lcd.setMode(TextLCD::DispOn); //DispOff, DispOn //Apaga o prende el display
    lcd.cls();
    lcd.printf("..Esperando...:(");
    hilo_serie.start(rx_serie);
    hilo_servo.start(servomotor);
    hilo_DisparoEntrada.start(DisparoEntrada);
    while (true) {
    }
   
}

void rx_serie(void)
{
    while(true)
    {
        if (serial.readable())
            {
            lcd.cls();
            char men_in[16]={0}; 
            serial.read(men_in,15);
            lcd.locate(0,0);                // columna 0 fila 1 (renglón Uno)
            lcd.printf(" Dato Recibido :");
            for(int indice=0; men_in[indice] != '\0'; ++indice){ //Ciclo recorre mensaje de envio
                men_in[indice]=toupper(men_in[indice]); //tolower->minusculas, toupper->MAYUSCULAS
            }
            lcd.locate(0,1);  // columna 1 fila 2 (renglón Uno)
            lcd.printf("%s", men_in);
            int numero=0;
            for(int i=0; men_in[i]!='\0';++i){
                char letra=men_in[i];
                if(isdigit(letra)){
                    numero=1;
                    break;
                }
            }
            if(numero){
                char * pEnd;
                li1 = strtol(men_in, &pEnd, 10);
                if(li1>180 || li1<0){
                li1=0;
                serial.write("ANGULO INVALIDO\n\r",16);
                lcd.locate(0,0);                // columna 0 fila 1 (renglón Uno)
                lcd.printf("ANGULO INVALIDO 0-180");
                ThisThread::sleep_for(500ms); 
            }
                sprintf(prueba, "%ld\n\r", li1);
                serial.write(prueba, sizeof(prueba));
                ThisThread::sleep_for(500ms); 
            }
            }    
        else
            {
            serial.write("..Esperando..\n\r",15);
            }
        ThisThread::sleep_for(500ms); 
        } 
}
void servomotor(void){
    float angulo=510; //cero grados 0.5ms
    float aux=0;
    Pinsalida.period_ms(20); //Establece el periodo PWM en milisefundos (int)
    Pinsalida.pulsewidth_us(angulo);//Establece el ancho de pulso PWM en microsegundos (int)
    ThisThread::sleep_for(1000ms);
    while (true) {
        aux=li1;
        angulo=((aux/180)*1600)+500;
        int angle=angulo;
        sprintf(prueba, "%d\n\r", angle);
        serial.write(prueba, sizeof(prueba));
        Pinsalida.pulsewidth_us(angle);
        ThisThread::sleep_for(500ms); 
    }
}

void DisparoEntrada(void)
{
    while(true)
    {
        Trg=1;
        wait_us(10);
        Trg=0;
        while(!Echo){};
        tiempo1.reset();
        tiempo1.start();
        while(Echo){};
        distancia_cm=((tiempo1.elapsed_time().count())/58);
        lcd.locate(0,2);
        lcd.printf("La distancia es:");
        lcd.locate(0,3);
        lcd.printf("%f \n\r", distancia_cm);
        //sprintf(mensaje,"La distancia es: %llu \n\r", distancia_cm); 
        //serial.write(mensaje, sizeof(mensaje));
        ThisThread::sleep_for(Tiempo_Medida);
    }
 }