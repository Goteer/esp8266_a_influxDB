#include <ESP8266WiFi.h>
#include <InfluxDb.h>

 #define INFLUXDB_HOST "163.10.10.123" //Si no funciona, verificar firewall y puertos. Suele funcionar con ip publica.
 #define INFLUXDB_PORT 9999
 #define INFLUXDB_DATABASE "prueba" //Nombre de la base de datos
 #define INFLUXDB_ORG "prueba" //nombre organizacion
 #define INFLUXDB_BUCKET "cubeta" //nombre bucket que recibe datos

 #define INFLUXDB_TOKEN "VGJcpLkEZUTAtZW5nG3uS1s2LOfV8slUR6m6L1UY0qTz9ab7qc0mKxG9spKVk72gSn4iUWVCQyc-drRfpUdSsA==" //Token de acceso para autorizar al dispositivo
 
 //Si resulta necesario autenticarse...
// #define INFLUXDB_USER "user"//Datos del usuario del dispositivo
// #define INFLUXDB_PASS "pass"

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
const int SENSOR_DELAY = 40; //Con 40, funciona correctamente, aunque los sensores tienen que estar a 3 dedos de distancia entre si.


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
  const char* ssid     = "ox-demo";
  const char* password = "ox-over-dns";
  



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
 influx.setDb(INFLUXDB_DATABASE);
 
 influx.setVersion(2);
 influx.setOrg(INFLUXDB_ORG);
 influx.setBucket(INFLUXDB_BUCKET);
 influx.setToken(INFLUXDB_TOKEN);
 influx.setPort(INFLUXDB_PORT);
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
  
  if (interruptCounter_int>0) { //Gestor de interrupcion cuando se detecta una obstruccion desde adentro
    

    delay(SENSOR_DELAY); //Revisar si la obstruccion dura al menos 300ms para medio-confirmar que es una persona
    sensorIntEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_INT);
    if (sensorIntEstaObstruido) {
      delay(SENSOR_DELAY); 
      sensorIntEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_INT);
      sensorExtEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_EXT);
      if (sensorExtEstaObstruido or sensorIntEstaObstruido) {
        delay(SENSOR_DELAY); 
        sensorIntEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_INT);
        sensorExtEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_EXT);
        //Si el sensor estuvo obstruido por al menos 150ms, se envia un paquete informando que pasó una pesona.
        if (sensorExtEstaObstruido) {
          Serial.print("Salida reconocida, avisando a la base de datos que salio una persona: "); Serial.println(valorSimulado);
          influx_send_sale_persona();
          //influx_test_limpieza();
          sensorExtEstaObstruido=false;
          }
        }
      }else{
        Serial.println("Interno activado por error");
        }

    
    
     
    sensorIntEstaObstruido=false;
    interruptCounter_ext=0;
    interruptCounter_int=0; //Para inhibir la interrupcion disparada por el otro sensor
    } //Fin interrupcion.

    
    if (interruptCounter_ext>0) { //Gestor de interrupcion cuando se detecta una obstruccion desde adentro
    

    delay(SENSOR_DELAY); //Revisar si la obstruccion dura al menos 300ms para medio-confirmar que es una persona
    sensorExtEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_EXT);
    if (sensorExtEstaObstruido) {
      delay(SENSOR_DELAY); 
      sensorIntEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_INT);
      sensorExtEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_EXT);
      if (sensorExtEstaObstruido or sensorIntEstaObstruido) {
        delay(SENSOR_DELAY); 
        sensorIntEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_INT);
        sensorExtEstaObstruido=!digitalRead(INTERRUPT_PIN_SENSOR_EXT);
        //Si el sensor estuvo obstruido por al menos 150ms, se envia un paquete informando que pasó una pesona.
        if (sensorIntEstaObstruido) {
          Serial.print("Entrada reconocida, avisando a la base de datos que entro una persona: "); Serial.println(valorSimulado);
          Serial.print("Valor int:"); Serial.print(sensorIntEstaObstruido); Serial.print("Valor ext:"); Serial.println(sensorExtEstaObstruido);
          influx_send_entra_persona();
          //influx_test_limpieza();
          sensorIntEstaObstruido=false;
          }
        }
      } else{
        Serial.println("Externo activado por error");
        }
  
    
    
     
    sensorExtEstaObstruido=false;
    interruptCounter_ext=0;
    interruptCounter_int=0; //Para inhibir la interrupcion disparada por el otro sensor
    } //Fin interrupcion.


    
    
}
