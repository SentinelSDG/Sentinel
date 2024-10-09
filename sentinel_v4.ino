#include "BluetoothSerial.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const byte ROWS = 4; // 4 baris
const byte COLS = 4; // 3 kolom
char keys[ROWS][COLS] = {
  {'1', '2', '3','A'},
  {'4', '5', '6','B'},
  {'7', '8', '9','C'},
  {'*', '0', '#','D'}
};
byte rowPins[ROWS] = {12, 13, 22, 5}; // Pin baris
byte colPins[COLS] = {4, 25, 15, 14}; // Pin kolom

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 21, 16);

const int len_key = 7;
char master_key[len_key] = {'2','5','7','9','A','C','D'};
char attempt_key[len_key];
int z=0;


const int servoPin1 = 18;    //Motor Servo menggunakan pin D18
const int servoPin2 = 32;    //motor servo menggunakan pin D32
const int pushbutton1 = 17;   //pushbutton1 untuk buka/tutup pintu manual 
const int pushbutton2 = 23;   //pushbutton2 untuk buka/tutup jendela manual
const int indikator1 = 27;     //indikator alarm pintu on/off
const int indikator2 = 33;    //indikator alarm jendela on/off
int sw1 = 0; // variabel untuk penanda on/off pintu
int sw2 = 0; // variabel untuk penanda on/off jendela

Servo servo1, servo2;
int pos = 0;  //posisi awal motor servo 0 derajat
bool IRSensor1; // variabel penampung data dari sensor Infra merah
bool IRSensor2;

BluetoothSerial ESP_BT; // Object for Bluetooth

int incoming; // membuat variabel penampung data dari bluetooth
int pintu_aktif;
int jendela_aktif;

void setup() {
  {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Insert Password");
  }
  
    
  ESP_BT.begin("Sentinel"); // memberi nama modul bluetooth
  servo1.attach(servoPin1, 550, 2400); // inisialisasi motor servo1
  servo2.attach(servoPin2, 550, 2400); // inisialisasi motor servo2 
  pinMode(34, INPUT); // inisialisasi pin D2 sebagai INPUT SENSOR infra merah pintu
  pinMode(35, INPUT); // inisialisasi pin D15 sebagai INPUT SENSOR infra merah2 jendela
  pinMode(26, INPUT); // inisialisasi pin D15 sebagai INPUT SENSOR geter
  pinMode(19, OUTPUT); // inisialisasi pin D19 sebagai output ke BUZZER
  pinMode(pushbutton1, INPUT_PULLUP); // inisialisasi sebagai input1
  pinMode(pushbutton2, INPUT_PULLUP); // inisialisasi sebagai input2
  pinMode(indikator1, OUTPUT); // inisialisasi sebagai output1
  pinMode(indikator2, OUTPUT); // inisialisasi sebagai output2
  digitalWrite(19, HIGH); // pin D19 diset HIGH alarm buzzer
  digitalWrite(indikator1, HIGH); // pin indikator1 pintu diset HIGH
  digitalWrite(indikator2, HIGH); // pin indikator2 jendwla diset HIGH
}

void loop() {
  {
  char key = keypad.getKey();
  lcd.setCursor(z-1,1);
  lcd.print("*");
  if (key){ 
    switch(key){
    case '*':
      z=0;
      break;
      case '#':
      delay (100);
      checkKEY();
      break;
    default:
       Serial.println(key);
       attempt_key[z]=key;
       z++;
       if (z == len_key){
        checkKEY();
       }
     }
 }
}
  // Jika pushbutton1 ditekan pertama kali
  if (digitalRead(pushbutton1) == 0 && sw1 == 0) {
    servo1.write(100); // kunci pintu
    pintu_aktif = 1; // sistem aktif
    
    digitalWrite(indikator1, LOW); // nyalakan Led sebagai indikator sistem aktif
    sw1 = 1; // set sw=1 menandakan Led nyala
    delay(1000); // delay 1000 ms atau 1s
  } else if (digitalRead(pushbutton1) == 0 && sw1 == 1) { // jika pushbutton ditekan kedua kali
    servo1.write(0); // buka pintu
    pintu_aktif = 0; // sistem tidak aktif
   
    digitalWrite(indikator1, HIGH); // padamkan Led indikator
    sw1 = 0; // set sw1=0, menandakan Led Padam
    delay(1000); // delay 1000ms atau 1s
  }

  // Jika pushbutton2 jwndwla ditekan pertama kali
  else if (digitalRead(pushbutton2) == 0 && sw2 == 0) {
    servo2.write(0); // kunci pintu
    jendela_aktif = 1; // sistem aktif
    
    digitalWrite(indikator2, LOW); // nyalakan Led sebagai indikator sistem aktif
    sw2 = 1; // set sw=1 menandakan Led nyala
    delay(1000); // delay 1000 ms atau 1s
  } else if (digitalRead(pushbutton2) == 0 && sw2 == 1) { // jika pushbutton ditekan kedua kali
    servo2.write(40); // buka pintu
    jendela_aktif = 0; // sistem tidak aktif
    
    digitalWrite(indikator2, HIGH); // padamkan Led indikator
    sw2 = 0; // set sw2=0, menandakan Led Padam
    delay(1000); // delay 1000ms atau 1s
  }

  if (ESP_BT.available()) { // Check if we receive anything from Bluetooth
    {
    incoming = ESP_BT.read(); // Read  

    if (incoming == '0') { // lock & enable system
      servo1.write(100); // kunci pintu
      servo2.write(0); // kunci pintu
      pintu_aktif = 1; // sistem aktif
      jendela_aktif = 1; // sistem aktif
      
      digitalWrite(indikator1, LOW); // LED indikator menyala
      digitalWrite(indikator2, LOW); // LED indikator menyala
    }   
   }

    if (incoming == '1') { // unlock
      servo1.write(0); // buka pintu
      servo2.write(40); // buka pintu
      pintu_aktif = 0; // sistem tidak aktif
     
      jendela_aktif = 0; // sistem tidak aktif
      
      digitalWrite(indikator1, HIGH); // LED indikator padam
      digitalWrite(indikator2, HIGH); // LED indikator padam
    }
  }
    
    // Jika sistem aktif
  if (pintu_aktif == 1) {
      IRSensor1 = digitalRead(34); // baca sensor infra merah
      if (IRSensor1 == 0)  { // jika ada objek terdeteksi
        digitalWrite(19, LOW); // BUZZER BUNYI
        delay(5000); // delay 5000 ms atau 5 s
        digitalWrite(19, HIGH); // BUZZER MATI
      } else {
        digitalWrite(19, HIGH); // BUZZER MATI
      }
  }
      
  else if (jendela_aktif == 1) {
      IRSensor2 = digitalRead(35); // baca sensor infra merah
      if (IRSensor2 == 1)  { // jika ada objek terdeteksi
        digitalWrite(19, LOW); // BUZZER BUNYI
        delay(5000); // delay 5000 ms atau 5 s
        digitalWrite(19, HIGH); // BUZZER MATI
      } else {
        digitalWrite(19, HIGH); // BUZZER MATI
      }
    }
  }
void checkKEY()
{
  int correct=0;
  int i;
  for(i=0; i<len_key; i++){
    if (attempt_key[i]==master_key[i]){
      correct++;
    }
  }
  if(correct==len_key && z==len_key){
    lcd.setCursor(0,1);
    lcd.print("Correct Key");
    Serial.println("Correct Key"); 
    delay (3000);   
    z=0;
    
    servo1.write(0); // buka pintu
    pintu_aktif = 0; // sistem tidak aktif
    digitalWrite(indikator1, HIGH); // padamkan Led indikator
    sw1 = 0; // set sw1=0, menandakan Led Padam
    servo2.write(40); // buka pintu
    jendela_aktif = 0; // sistem tidak aktif
    digitalWrite(indikator2, HIGH); // padamkan Led indikator
    sw2 = 0; // set sw2=0, menandakan Led Padam
    delay(1000); // delay 1000ms atau 1s
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sentinel key");
    Serial.println("Insert key");
  }
  else
  {
    digitalWrite(19, LOW); // BUZZER BUNYI
    delay(500); // delay 5000 ms atau 5 s
    digitalWrite(19, HIGH); // BUZZER MATI
    lcd.setCursor(0,1);
    lcd.print("Incorrect Key");
    Serial.println("Incorrect Key");
    delay(3000);
    z=0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sentinel key");
    Serial.println("Sentinel key");
  }
  for(int zz=0; zz<len_key; zz++){
    attempt_key[zz]=0;
  }
}
