//LIBRARIES:
#include <ESP8266WiFi.h>
#include <PubSubClient.h>



//CONNECTIONS:
#define MOTOR_1_PWM D1  //propeller 1 (FL) blu wire
#define MOTOR_2_PWM D2  //propeller 2 (FR) purple wire
#define MOTOR_3_PWM D5  //propeller 3 (RL) yellow wire
#define MOTOR_4_PWM D6  //propeller 4 (RR) orange wire

#define MOTORS_DIR D0   //propeller 1,2,3,4 white wire

#define TRIGGER_PIN D7  //green wire
#define ECHO_PIN D8     //brown wire

//DRONE STATUS:
#define LANDED 0
#define LANDING 1
#define FLYING 2



//CONSTANTS:
const char *SSID = /*DA COMPLETARE*/;   //WiFi info
const char *PSW =  /*DA COMPLETARE*/;

const char *MQTT_BROKER = /*DA COMPLETARE*/;   //MQTT info
const int MQTT_PORT = /*DA COMPLETARE*/;
const char *MQTT_TOPIC = "drone";

const unsigned long TIMEOUT = 6000;  // [microsec] (time taken by sound to travel 2 m)
const  float SOUND_SPEED =  0.0343;   // [cm/microsec]

const float PERC_EPS_MAX = 0.2;         // [%]
const unsigned int RANGE_TRG_MIN = 10;  // [cm]
const unsigned int RANGE_TRG_MAX = 70;  // [cm]

const unsigned int RANGE_PWM_MIN = 0;
const unsigned int RANGE_PWM_MAX = 1023;



//VARIABILI:
int target = 0;   //target altitude [cm]
int eps = 0;      //error margin [cm]
int s = 0;        //current speed [%]

int status = LANDED;
boolean wifi_status = false;
boolean mqtt_status = false;

WiFiClient espClient;
PubSubClient client(espClient);



//WIFI FUNCS:
/*
 * Connection to the WiFi network.
 */
void wifiConn() {
  int countAttemps = 1;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSW);
    
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("WIFI:: Connecting to WiFi... (Attemp: %d Status: %d)\n", countAttemps, WiFi.status());
    if (countAttemps > 30) {
      Serial.println("WIFI:: Err: connection to WiFi failed, too many attemps failed");
      return;
    }
    countAttemps++;
    delay(1000);
  }

  wifi_status = true;
  Serial.println("WIFI: connected to the WiFi network");
  done();
  return;
}



//MQTT FUNCS:
/*
 * Connection to the MQTT broker.
 */
void mqttConn(){
  int countAttemps = 1;

  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
  
  String client_id = "WemosD1Mini_Drone_" + String(WiFi.macAddress());
  Serial.printf("MQTT:: Client %s connects to the mqtt broker %s on port %d\n", client_id.c_str(), MQTT_BROKER, MQTT_PORT);
  
  while (!client.connected()) {
    boolean b = client.connect(client_id.c_str());
    if (!b && countAttemps > 30) {
      Serial.printf("MQTT:: Err: connection failed with state %d (Attemps: %d)\n", client.state(), countAttemps);
      mqtt_status = false;
      return;
    }
    countAttemps++;
    delay(1000);
  }
  
  mqtt_status = true;
  Serial.println("MQTT:: connected to mqtt broker");
  done();
  
  client.subscribe(MQTT_TOPIC);
  return;
}

/*
 * This function is called when new messages are received.
 */
void callback(char *topic, byte *payload, unsigned int len) {
  if(payload[0] == 'R') return;
  
  char str[len+1];
  memcpy(str, payload, len);
  str[len] = '\0';

  Serial.printf("CALLBACK:: Msg: %s\n", str);

  char *token = strtok(str, ":");
  if(strcmp(token, "t") == 0){

    int t, e;
    
    token = strtok(NULL, ":");
    if(token == NULL) return;
    else t = atoi(token);
    
    token = strtok(NULL, ":");
    if(token == NULL) return;
    else e = atoi(token);

    if(t < RANGE_TRG_MIN || t > RANGE_TRG_MAX) return;
    if(e <= 0 || e > t*PERC_EPS_MAX) return;

    status = FLYING;
    target = t;
    eps = e;
    printInfo("CALLBACK:: new target set");
  }else if(strcmp(token, "S") == 0){
    status = LANDING;
    target = 0;
    eps = 0;
    printInfo("CALLBACK:: switch to landing procedure");
  }else if(strcmp(token, "I") == 0){
    printInfo("CALLBACK:: Status: " + String(status) + "\n Target: " + String(target) + "\n Eps: " + String(eps) + "\n Speed: " + String(s) + "\n Dist: " + String(estimateDist()));
  }
  
  return;
}

/*
 * It sends a mqtt msg and print on serial port info about the drone.
 */
void printInfo(String msg){
  msg = "Rep->  " + msg;
  Serial.println(msg);
  client.publish(MQTT_TOPIC, msg.c_str());
  return;
}



//SENSOR FUNCS:
/*
 * It will send out a sonic burst
 * which it will be received in the echo pin.
 */
void enableTrigger(){
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(20);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  return;
}

/*
 * It estimates the distance from the ground 
 * relying on the time taken by the sound wave to travel forward and bounce backward.
 */
int estimateDist(){
  int dist; 
  do{
    enableTrigger();
    unsigned long deltaTime = pulseIn(ECHO_PIN, HIGH);
  
    if(deltaTime > TIMEOUT) dist = 0;
    else dist = (deltaTime / 2) * SOUND_SPEED;
  }while(dist == 0);

  Serial.printf("Distanza: %d\n", dist);
  
  return dist;
}

/*
 * It returns the gap between current and past altitude.
 */
int deltaDist(){
  static int dist_prev = estimateDist();
  
  int dist_cur = estimateDist();
  int delta = dist_cur - dist_prev;

  Serial.printf("Delta: %d\n", delta);
  
  dist_prev = dist_cur;
  return delta;
}



//ACTUATORS FUNCS:
/*
 * It converts the speed expressed as percentage [0, 100]
 * into an acceptable value for the pwm.
 */
int speedToPwm(int spd){
   return RANGE_PWM_MAX - ((RANGE_PWM_MAX - RANGE_PWM_MIN) * spd / 100);
}

/*
 * It sets the motors to the speed passed as parameter.
 */
void setMotorsSpeed(int spd){
  s = spd;
  int pwm = speedToPwm(spd);

  if(spd == 0){
      digitalWrite(MOTOR_1_PWM, LOW);
      digitalWrite(MOTOR_2_PWM, LOW);
      digitalWrite(MOTOR_3_PWM, LOW);
      digitalWrite(MOTOR_4_PWM, LOW);
      
      digitalWrite(MOTORS_DIR, LOW);
    }else{
      analogWrite(MOTOR_1_PWM, pwm);
      analogWrite(MOTOR_2_PWM, pwm);
      analogWrite(MOTOR_3_PWM, pwm);
      analogWrite(MOTOR_4_PWM, pwm);
      
      digitalWrite(MOTORS_DIR, HIGH);
    }

  return;
}



//LED BUILTIN FUNCS:
/*
 * Blink in case of connection succeeded to WiFi/mqtt broker. 
 */
void done(){
  digitalWrite(LED_BUILTIN, HIGH);
  for(int i=0; i<10; i++){
    if(i%2 == 0) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  return;
}


//CONTROL FUNCS:
/* 
 *  It adjusts the speed of the motors according to the set target.
 */
void flightProc(){
    int dist = estimateDist();
    int delta = deltaDist();

    int var = 1;
    if(dist < target - (2*eps)) var = 5;
    if(dist > target + (2*eps)) var = 2;
    
    if(dist < target - eps) setMotorsSpeed(min(s+var, 100));
    if(dist > target + eps && delta >= 0) setMotorsSpeed(max(s-var, 0));
     
    Serial.printf("FLYING:: Speed: %d\n", s);

    return;
}

/*
 * The function makes the drone land safely, checking the descent speed.
 */
void landingProc(){  
  int delta;
  int dist = estimateDist();

  while(dist > 4){ 
    delta = deltaDist();
    if(delta >= 0) setMotorsSpeed(max(s-2, 0));
    if(delta <= -5) setMotorsSpeed(min(s+1, 100));
    
    Serial.printf("LANDING:: Speed: %d\n", s);
    
    delay(200);
    dist = estimateDist();
  }
  
  setMotorsSpeed(0);
  status = LANDED;
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("LANDED:: Speed: %d\n", s);
  
  return;
}



void setup(){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(MOTOR_1_PWM, OUTPUT);
  pinMode(MOTOR_2_PWM, OUTPUT);
  pinMode(MOTOR_3_PWM, OUTPUT);
  pinMode(MOTOR_4_PWM, OUTPUT);
  
  pinMode(MOTORS_DIR, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  wifiConn();
  if(wifi_status == true) mqttConn();

  printInfo("START");
}

void loop(){
  if(!client.connected()){
       landingProc();
       while(true){};           //WDT restarts the board
  }
  else if(status == LANDING) landingProc();
  else if(status == FLYING) flightProc();
  client.loop();
  delay(200);
}
