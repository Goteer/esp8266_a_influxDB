#include <ESP8266WiFi.h>
#include <InfluxDb.h>

 #define INFLUXDB_HOST "influxdb-c.catedras.linti.unlp.edu.ar" //Si no funciona, verificar firewall y puertos. Suele funcionar con ip publica.
 #define INFLUXDB_PORT 443
 #define INFLUXDB_DATABASE "joaquindiazchurria" //Nombre de la base de datos
 //#define INFLUXDB_ORG "joaquindiazchurria" //nombre organizacion
 //#define INFLUXDB_BUCKET "cubeta" //nombre bucket que recibe datos

 //#define INFLUXDB_TOKEN "n/a" //Token de acceso para autorizar al dispositivo
 
 //Si resulta necesario autenticarse...
 #define INFLUXDB_USER "joaquindiazchurria"//Datos del usuario del dispositivo
 
 #define INFLUXDB_PASS "joaquindiazchurria_iot"

 #define DISPOSITIVO "d1" //Nombre del dispositivo
 #define SENSOR_OBSTRUCCION "reflectivo"

bool sensorIntEstaObstruido;
bool sensorExtEstaObstruido;
double valorSimulado;
Influxdb influx(INFLUXDB_HOST);
const byte INTERRUPT_PIN_SENSOR_INT = 13; //Corresponde al pin D7 en la placa ESP8266. Sensor interior.
const byte INTERRUPT_PIN_SENSOR_EXT = 12; //Corresponde al pin D6 en la placa. Sensor exterior.
volatile byte interruptCounter_int = 0; //Cantidad de interrupciones pendientes
volatile byte interruptCounter_ext = 0;
const int SENSOR_DELAY = 100; //Con 40, funciona correctamente, aunque los sensores tienen que estar a 3 dedos de distancia entre si.

unsigned int tiempo_int = 0;
unsigned int tiempo_ext = 0;

// -------------------------------- SETUP ------------------------------


void setup() {
  
  Serial.begin(115200);
  
  pinMode(INTERRUPT_PIN_SENSOR_INT, INPUT_PULLUP); //Se define al pin interruptor como input
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_SENSOR_INT), sensorInterrupt_int, RISING); //Se vincula la funcion "sensorInterrupt" a la subida de señal del pin de interrupcion.
  
  pinMode(INTERRUPT_PIN_SENSOR_EXT, INPUT_PULLUP); //Se define al pin interruptor como input
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_SENSOR_EXT), sensorInterrupt_ext, RISING); //Se vincula la funcion "sensorInterrupt" a la subida de señal del pin de interrupcion.
  
/*
 * Se intenta conectar al wifi, indicando en monitor serie a cual se intenta conectar.
 * Si falla la conexion, reintenta indefinidamente hasta que se logra la conexion.
 * Una vez conectado, avisa por monitor serie.
 */

  
 // connect to WiFi
  const char* ssid     = "ox-demo"; //"Linti";
  const char* password = "ox-over-dns";//"passdelap2015!";
  
//usuario de grafana
//usuario_iot

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  sensorIntEstaObstruido=false;
  sensorExtEstaObstruido=false;

//Connect to InfluxDB
// Influxdb influx(INFLUXDB_HOST, INFLUXDB_PORT); // deprecated by the creation of an instance with the host as arg for constructor

 Serial.println("Conectando a influx...");

 // set the target database
 //influx.setDb(INFLUXDB_DATABASE);

 // or use a db with auth
  influx.setDbAuth(INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASS); // with authentication

 
// influx.setVersion(1);
// influx.setOrg(INFLUXDB_ORG);
// influx.setBucket(INFLUXDB_BUCKET);
// influx.setToken(INFLUXDB_TOKEN);
// influx.setPort(INFLUXDB_PORT);
 valorSimulado = 0;

 Serial.println("Conectado a influx!");


}

// ----------------------------- RUTINAS DE INTERRUPCIONES --------------------------------

void sensorInterrupt_int() {
  interruptCounter_int++;
  }
  
void sensorInterrupt_ext() {
  interruptCounter_ext++;
  }

// ------------------------ FUNCIONES -------------------



void influx_send_is_obstruido(double value){ //Se envia el tiempo que estuvo obstruido
  InfluxData measurement ("Obstruccion");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addValue("value", value);

  // write it into db
  influx.write(measurement);
  
  }

void influx_send_is_persona(int value){ //Se conto una persona.
  InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addValue("value", value);

  // write it into db
  influx.write(measurement);
  
  }

void influx_send_entra_persona(){ //Se conto una persona.
  InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addTag("accion", "entrar");
  measurement.addValue("value", 1);

  // write it into db
  influx.write(measurement);
  
  }

  
void influx_send_sale_persona(){ //Se conto una persona.
  InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addTag("accion", "salir");
  measurement.addValue("value", -1);

  // write it into db
  influx.write(measurement);
  
  }

void influx_test_limpieza(){ //Se conto una persona.
  InfluxData measurement ("TestLimpieza");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addValue("value", 1);

  // write it into db
  influx.write(measurement);
  
  }

void influx_send_sensor_value(String sensor,double value){ //Se envia un valor general de un sensor
  InfluxData measurement ("Valor_Sensor");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", sensor);
  measurement.addValue("value", value);

  // write it into db
  influx.write(measurement);
  
  }

void influx_send_test_value(double value){
  InfluxData measurement ("Medida"); //Identificador del "_measurement"
  measurement.addTag("device", DISPOSITIVO); //Etiqueta de "_device" de valor DISPOSITIVO
  measurement.addTag("sensor", "simulado2"); //Etiqueta de "_sensor" de valor "simulado2"
  measurement.addValue("value", value);// "_value" de valor "value"

  // write it into db
  influx.write(measurement);

  }



// -------------------------- LOOP ------------------------


void loop() {
  // put your main code here, to run repeatedly:

  if (interruptCounter_int>0) {
    tiempo_int+=1;
    }

  if (interruptCounter_ext>0) {
    tiempo_ext+=1;
    }

  if (tiempo_int>900000){
     //Si estuvo mucho tiempo esperando el otro sensor, reiniciar contadores
            interruptCounter_int=0;
            interruptCounter_ext=0;
            tiempo_ext=0;
            tiempo_int=0;
          Serial.println("Reinicio int");
          
    }
    


    if (tiempo_ext>900000){
     //Si estuvo mucho tiempo esperando el otro sensor, reiniciar contadores
            interruptCounter_int=0;
            interruptCounter_ext=0;
            tiempo_ext=0;
            tiempo_int=0;
          Serial.println("Reinicio ext");
          
    }

    if (interruptCounter_int>10) { //Si se tapo el interior
      if (interruptCounter_ext>10 && interruptCounter_ext<interruptCounter_int && tiempo_ext<1000) { //Revisar si se tapo el exterior *despues* del interior
        Serial.println("Persona sale");
        influx_send_sale_persona();
        interruptCounter_int=0;
        interruptCounter_ext=0;
        tiempo_int+=800000;
        tiempo_ext+=800000;
        }
      }

      
      
      if (interruptCounter_ext>10) {
      if (interruptCounter_int>10 && interruptCounter_int<interruptCounter_ext && tiempo_int<1000) {//Revisar si se tapo el interior *despues* del exterior
        Serial.println("Persona entra");
        influx_send_entra_persona();
        interruptCounter_int=0;
        interruptCounter_ext=0;
        tiempo_ext+=800000;
        tiempo_int+=800000;
          
      }
      
    }

    
   // Serial.print("interno counter: "); Serial.print(interruptCounter_int); Serial.print("externo counter: "); Serial.println(interruptCounter_ext);
    
}
