#include <Arduino.h>
#include <Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

//pines

//motor Izq
#define pin_ena1  27
#define pin_n1    14
#define pin_n2    4

//motor Der
#define pin_n3    2
#define pin_n4    32
#define pin_ena2  33

//servo
#define pin_servo 13

//HC-SR04
#define pin_trig  26
#define pin_echo  25

//IMU MPU92.65
#define pin_scl   22
#define pin_sda   21

//configuraciones

//wifi
const char* ssid = "ssid";
const char* password =  "passw";

//MQTT
const char *mqtt_server = "jetson.org";
const int mqtt_port = 1883;
const char *mqtt_user = "qtt_user";
const char *mqtt_pass = "pass";
const String root_topic = "topico_raiz";

//servo
#define servo_max_ang   180
#define servo_min_ang   0
#define servo_speed     25
#define servo_step      3

//var
Servo servo;                // se declara instancia de servo

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//mensajeria
String tosend = "";
char buf[20];

//funciones
int distance();
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void movement(String side); //side: left,right,straight,back

//set-up
void setup() {
  Serial.begin(115200);

  //PINMODES
  //hc-sr04
  pinMode(pin_trig,OUTPUT);
  pinMode(pin_echo,INPUT);

  //motor izq
  pinMode(pin_ena1,OUTPUT);   // h 
  pinMode(pin_n1,OUTPUT);     // l
  pinMode(pin_n2,OUTPUT);     // l

  //motor der
  pinMode(pin_ena2,OUTPUT);
  pinMode(pin_n3,OUTPUT);
  pinMode(pin_n4,OUTPUT);

  digitalWrite(pin_ena1,HIGH);
  digitalWrite(pin_ena2,HIGH);

  servo.attach(pin_servo); //pin 13

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  

}


//loop1
void loop() {
  if (!client.connected()) {
		reconnect();
	}
    delay(300);
    for(int posDegrees = servo_min_ang; posDegrees <= servo_max_ang; posDegrees+=servo_step) {
      if (posDegrees>24){
        servo.write(180-posDegrees);
      }
      delay(servo_speed);
      tosend = String(distance()) + "," + String(posDegrees);
      tosend.toCharArray(buf, 20);
      client.publish("topico_raiz/map", buf);
      client.loop();
    }
     servo.write(155);
}
  int distance(){
    digitalWrite(pin_trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_trig,LOW);
    long time = pulseIn(pin_echo,HIGH);
    int distance = time / 58;
    return distance;
  }
void movement(String side){
  if (side == "straight"){
      //rueda1
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,HIGH);
      
      //rueda2
      digitalWrite(pin_n3,HIGH);
      digitalWrite(pin_n4,LOW);
  }

  if (side == "back"){
      digitalWrite(pin_n1,HIGH);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,HIGH);
  }
  if (side == "left"){
      digitalWrite(pin_n1,HIGH);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,HIGH);
      digitalWrite(pin_n4,LOW);
  }
  if (side == "right"){
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,HIGH);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,HIGH);
  }
  if (side == "stop"){
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,LOW);
  }
}

//set-up wifi
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");

	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}

	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

  String topic_str(topic);

	if(topic_str == root_topic + "/movement"){
    movement(incoming);
  }

}

void reconnect() {
	while (!client.connected()) {
		Serial.print("Intentando conexión mqtt...");
		// Creamos un cliente ID
		String clientId = "Jetson_";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
			// Nos suscribimos
			if(client.subscribe("topico_raiz/movement")){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("Fallo suscripcion");
      }
		} else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");
			delay(5000);
		}
	}
}
