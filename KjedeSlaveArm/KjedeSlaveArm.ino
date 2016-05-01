#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define HAAND_PIN 9
#define UARM_PIN 8
#define OARM_PIN 7
#define RST_PIN 1

int ledHaand = 4;
int ledUarm = 6;
int ledOarm = 5;

int ledTid12 = 14;
int ledTid11 = 15;
int ledTid10 = 16;
int ledTid9 = 17;
int ledTid8 = 3;
int ledTid7 = 2;

int x = 0;
int tidTeller = 0;

int antall = 1;
int riktig = 0;
int sistriktig = 0;
int riktigfordet = 0;
int riktigfordetigjen = 0;

MFRC522 haand(HAAND_PIN, RST_PIN);
MFRC522 uarm(UARM_PIN, RST_PIN);
MFRC522 oarm(OARM_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  while(!Serial);

  SPI.begin();

  Wire.begin(2); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  haand.PCD_Init();
  uarm.PCD_Init();
  oarm.PCD_Init();

  pinMode(ledHaand, OUTPUT);
  pinMode(ledUarm, OUTPUT);
  pinMode(ledOarm, OUTPUT);
  pinMode(ledTid12, OUTPUT);
  pinMode(ledTid11, OUTPUT);
  pinMode(ledTid10, OUTPUT);
  pinMode(ledTid9, OUTPUT);
  pinMode(ledTid8, OUTPUT);
  pinMode(ledTid7, OUTPUT);

  digitalWrite(ledHaand, LOW);
  digitalWrite(ledUarm, LOW);
  digitalWrite(ledOarm, LOW);
  digitalWrite(ledTid12, HIGH);
  digitalWrite(ledTid11, HIGH);
  digitalWrite(ledTid10, HIGH);
  digitalWrite(ledTid9, HIGH);
  digitalWrite(ledTid8, HIGH);
  //digitalWrite(ledTid7, HIGH);
}

void requestEvent(){
  Wire.write(antall);
  Serial.println("sendt");
}

void receiveEvent(int bytes) {
 x = Wire.read();
    if (x == 1) {
      omstart();
    }else if(x == 2){
      tidTeller++;
  }
}

void check_reader(MFRC522 reader, int id){
  if( ! reader.PICC_IsNewCardPresent() || ! reader.PICC_ReadCardSerial() ) {
        return;
  }
  unsigned int hex_num;
   hex_num =  reader.uid.uidByte[0] << 24;
    hex_num += reader.uid.uidByte[1] << 16;
    hex_num += reader.uid.uidByte[2] <<  8;
    hex_num += reader.uid.uidByte[3];
    if ((id == 1 && hex_num == 16457) || (id == 2 && hex_num == 14549) || (id == 3 && hex_num == 36084)) {
      if (id != sistriktig && id != riktigfordet){
        switch(id){
          case 1:;
            blinke(ledHaand);
            break;
          case 2:
            blinke(ledUarm);
            break; 
          case 3:
            blinke(ledOarm);
        }
        antall = antall++;
        riktig = id;
        riktigfordet = sistriktig;
        sistriktig = riktig;
     } 
    } else {
      
       //antall øker midlertidig for at tidtakeren til master skal registerere hendelsen
       Serial.println(hex_num);
       antall++;
       delay(250);
       antall++;
    }
    Serial.println(hex_num);
    delay(100);
}

void omstart(){
  blinke(ledHaand);
  delay(200);
  blinke(ledUarm);
  delay(200);
  blinke(ledOarm);
  digitalWrite(ledHaand, LOW);
  digitalWrite(ledUarm, LOW);
  digitalWrite(ledOarm, LOW);
  digitalWrite(ledTid12, HIGH);
  digitalWrite(ledTid11, HIGH);
  digitalWrite(ledTid10, HIGH);
  digitalWrite(ledTid9, HIGH);
  digitalWrite(ledTid8, HIGH);
  digitalWrite(ledTid7, HIGH);
  riktig = 0;
  sistriktig = 0;
  riktigfordet = 0;
  tidTeller = 0;
}

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

//Spiller av første halvdel av nedtellingssekvens
//Andre halvdel spilles av KjedeSlaveBen01
void play(){
  delay(10000);
  digitalWrite(ledTid12, LOW);
  delay(10000);
  digitalWrite(ledTid11, LOW);
  delay(10000);
  digitalWrite(ledTid10, LOW);
  delay(10000);
  digitalWrite(ledTid9, LOW);
  delay(10000);
  digitalWrite(ledTid8, LOW);
  delay(10000);
  //digitalWrite(ledTid7, LOW);
}

void loop() {
  check_reader(haand, 1);
  check_reader(uarm, 2);
  check_reader(oarm, 3);
  if (tidTeller == 1){
    play();
    tidTeller++;
    }
  //Venter til KjedeMester01 har fått lest av variabelen antall før den nullstilles
  if (antall == 3){
    delay(100);
    antall = 0;
    delay(100);
  }
  delay(400);
}

