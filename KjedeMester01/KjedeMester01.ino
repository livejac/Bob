#include <Wire.h> 
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define HAAND_PIN 10
#define UARM_PIN 9
#define OARM_PIN 8
#define RST_PIN 2

Servo hode;
Servo munn;

int ledHaand = 5;
int ledUarm = 4;
int ledOarm = 3;
int magneter = 2;

int nystart = 1;
int nedtelling = 2;

int antall = 0;
int riktig = 0;
int sistriktig = 0;
int riktigfordet = 0;
int riktigfordetigjen = 0;

int tid = 0;
int startTid = 0;
int totalTid = 0;

int lydNr = 0;

MFRC522 haand(HAAND_PIN, RST_PIN);
MFRC522 uarm(UARM_PIN, RST_PIN);
MFRC522 oarm(OARM_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  
  Wire.begin(); 

  haand.PCD_Init();
  uarm.PCD_Init();
  oarm.PCD_Init();

  pinMode(ledHaand, OUTPUT);
  pinMode(ledUarm, OUTPUT);
  pinMode(ledOarm, OUTPUT);
  digitalWrite(ledUarm, LOW);
  digitalWrite(ledOarm, LOW);
  digitalWrite(ledHaand, LOW);

  pinMode(magneter, OUTPUT);
  digitalWrite(magneter, HIGH);

  hode.attach(7);
  munn.attach(6);

  hode.write(70);
  munn.write(0);
}


//Sjekker status til RFID-lesere
void check_reader(MFRC522 reader, int id){
  if( ! reader.PICC_IsNewCardPresent() || ! reader.PICC_ReadCardSerial() ) {
        return;
  }
  unsigned int hex_num;
   hex_num =  reader.uid.uidByte[0] << 24;
    hex_num += reader.uid.uidByte[1] << 16;
    hex_num += reader.uid.uidByte[2] <<  8;
    hex_num += reader.uid.uidByte[3];
    //Hvis leserens id matcher chipens id skrus riktig led på, og antall riktige øker med 1
    if ((id == 1 && hex_num == 19529) || (id == 2 && hex_num == 14549 ) || (id == 3 && hex_num == 36084)) {
      if (id != sistriktig && id != riktigfordet){
        switch(id){
          case 1:
            blinke(ledHaand);
            break;
          case 2:
            blinke(ledUarm);
            break; 
          case 3:
            blinke(ledOarm);
        }
        antall++;
        tid++;
        //Følgende variabler endres for at samme ben ikke skal registreres riktig flere ganger
        riktig = id;
        riktigfordet = sistriktig;
        sistriktig = riktig;
        
        lydNr = random(150, 169);
        snakke(lydNr);
        lydNr = 0;
     } 
    } else {
       tid = tid+1;
    }
    delay(100);
}

void omstart(){
  //Gir alle slaver beskjed om å resette alle variabler
  Wire.beginTransmission(2);
  Wire.write(nystart); 
  Wire.endTransmission();    
  Wire.beginTransmission(3);
  Wire.write(nystart);              
  Wire.endTransmission();    
  Wire.beginTransmission(4);
  Wire.write(nystart);             
  Wire.endTransmission();
  
  //Blinkesekvens av de forskjellige sonene, for fancy effekt
  blinke(ledHaand);
  delay(200);
  blinke(ledUarm);
  delay(200);
  blinke(ledOarm);

  //Spiller av "takk for hjelpa"-lyd og hodebevegelse
  Serial.println(171);
  hode_Vri();
  munn_Snakk();
  delay(1500);
  hode_Vri();
  munn_Snakk();
  delay(1500);
  
  //Resetter alle variabler og skrur av alt lys
  digitalWrite(ledHaand, LOW);
  digitalWrite(ledUarm, LOW);
  digitalWrite(ledOarm, LOW);
  riktig = 0;
  sistriktig = 0;
  riktigfordet = 0;
  totalTid = (millis() - startTid)/1000;
  
  //Sender variabelen totalTid til en Raspberry Pi via Serialkommunikasjon, postes til Twitter
  Serial.println(totalTid);
  tid = 0;
  totalTid = 0;
  start_tid();

  //Skrur magneter av og på
  digitalWrite(magneter, LOW);
  delay(500);
  digitalWrite(magneter, HIGH);
}

void start_tid(){
  //Variabelen tid øker med 1 hver gang en sensor registrerer aktivitet
  //startTid er tidspunktet første hendelse skjer
  //Sender beskjed til slavene som styrer nedtellingslysene om å starte sekvensene
  if (tid == 1){
    startTid = millis();
    Wire.beginTransmission(2);
    Wire.write(nedtelling);
    Wire.endTransmission();
    Wire.beginTransmission(3);
    Wire.write(nedtelling); 
    Wire.endTransmission();
    tid = 2;
  }
}

//Blinker aktuell LED-sone før den skrus på
void blinke(int blinker){
  digitalWrite(blinker, HIGH);
  delay(100);
  digitalWrite(blinker, LOW);
  delay(100);
  digitalWrite(blinker, HIGH);
  delay(100);
  digitalWrite(blinker, LOW);
  delay(100);
  digitalWrite(blinker, HIGH);
}

//Spør slave om status på variabelen antall riktige
void request(int nr, int ant){
  Wire.requestFrom(nr, ant);
  Serial.println(Wire.available());
  while (Wire.available()){
    int x = Wire.read();
    Serial.println(x);
    if (x == 1){
      tid++;
      x=0;
    } else if (x == 3){
      antall = antall+3;
     x=0;
    }
  }
}

//Ber Raspberry Pi spille av riktig lydfil via Serialkommunikasjon
//Beveger hode og munn til riktig tid
void snakke(int lydNr){
  if(151 < lydNr < 160){
    Serial.println(lydNr);
    hode_Vri();
    munn_Snakk();
  }else if (lydNr > 160){
    Serial.println(lydNr);
    hode_Vri();
    munn_Snakk();
    delay(1500);
    hode_Vri();
    munn_Snakk();
    delay(1500);
    hode_Vri();
    munn_Snakk();
    delay(1500);
  }
}

void hode_Vri(){
  hode.write(75);
  delay(600);
  hode.write(90);
  delay(400);
  hode.write(80);
  delay(300);
  hode.write(70);
}

void munn_Snakk(){
  munn.write(10);
  delay(300);
  munn.write(30);
  delay(400);
  munn.write(0);
  delay(150);
  munn.write(30);
  delay(100);
  munn.write(10);
  delay(150);
  munn.write(20);
  delay(100);
  munn.write(10);
  delay(100);
  munn.write(20);
  delay(100);
  munn.write(0);
}

void loop() {
  request(2, 1);
  request(3, 1);
  request(4, 1);
  check_reader(haand, 1);
  check_reader(uarm, 2);
  check_reader(oarm, 3);
  start_tid();
  Serial.println(startTid);
  
  //Når alle 12 sensorer er registert riktig, resettes spillet og
  //score registreres på Twitter
  if (antall == 12){
    antall = 0;
    omstart();
    delay(3000);
  }
  delay(400);
}

