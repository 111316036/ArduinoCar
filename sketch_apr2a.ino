#include <Ultrasonic.h>

Ultrasonic ultrasonic(12, 11);

const byte A_1A = 6; const byte A_1B = 5; 
const byte B_1A = 10; const byte B_1B = 9; 

const int S1 = A0; // 最左
const int S2 = A1; // 左中
const int S3 = A2; // 右中
const int S4 = A3; // 最右

byte runSpeedL = 200; 
byte runSpeedR = 190; 
byte avoidOuterR = 210; 
byte avoidInnerL = 60;

int lastState = 0; 

void setup() { 
  pinMode(A_1A, OUTPUT); pinMode(A_1B, OUTPUT);
  pinMode(B_1A, OUTPUT); pinMode(B_1B, OUTPUT);
  pinMode(S1, INPUT_PULLUP); pinMode(S2, INPUT_PULLUP);
  pinMode(S3, INPUT_PULLUP); pinMode(S4, INPUT_PULLUP);
  Serial.begin(9600);
}

// --- 動作控制函數 ---
void forward() {
  analogWrite(A_1A, runSpeedL); digitalWrite(A_1B, LOW);
  analogWrite(B_1A, runSpeedR); digitalWrite(B_1B, LOW);
}

void sharpLeft() { // 強力原地左轉
  digitalWrite(A_1A, LOW); analogWrite(A_1B, 150);
  analogWrite(B_1A, 150); digitalWrite(B_1B, LOW);
}

void sharpRight() { // 強力原地右轉
  analogWrite(A_1A, 150); digitalWrite(A_1B, LOW);
  digitalWrite(B_1A, LOW); analogWrite(B_1B, 150);
}

void stopMotor() {
  digitalWrite(A_1A, LOW); digitalWrite(A_1B, LOW);
  digitalWrite(B_1A, LOW); digitalWrite(B_1B, LOW);
}

// --- 避障動作 (雙斜前衝邏輯) ---
void avoid() {
  stopMotor();
  delay(300);

  sharpLeft();
  delay(300); 
  stopMotor(); delay(100);
  
  forward();
  delay(1300); 
  stopMotor(); delay(100);

  sharpRight();
  delay(400); 
  stopMotor(); delay(100);
  
  forward();
  delay(1000); 
  stopMotor(); delay(100);

  sharpLeft();
  delay(200); 
  stopMotor(); delay(100);

  analogWrite(A_1A, 160); digitalWrite(A_1B, LOW); 
  analogWrite(B_1A, 160); digitalWrite(B_1B, LOW); 

  unsigned long runStartTime = millis(); 
  while (digitalRead(S2) == LOW && digitalRead(S3) == LOW) {
    delay(5); 
    if (millis() - runStartTime > 5000) break; 
  }

  sharpLeft();
  delay(200); 
  forward(); 
  delay(150);
  
  lastState = 0; 
  stopMotor();
  delay(200);
}

void loop() {
  long distance = ultrasonic.read();
  int L1 = digitalRead(S1); 
  int L2 = digitalRead(S2); 
  int R1 = digitalRead(S3); 
  int R2 = digitalRead(S4); 

  // 1. 避障優先
  static int obstacleCounter = 0; 
  if (distance > 0 && distance < 20) obstacleCounter++;
  else obstacleCounter = 0;
  if (obstacleCounter >= 3) { 
    avoid(); 
    return; 
  }

  // 2. 終點判斷 (優先權次高)
  if (L1 == HIGH && L2 == HIGH && R1 == HIGH && R2 == HIGH) {
    stopMotor();
    Serial.println("Goal Reached!");
    while(1); 
  }

  // 3. 四路循線邏輯 (針對直角優化：判斷順序至關重要)
  
  // 第一順位：最外側碰到線，代表已經到了直角彎，必須立刻原地轉
  if (L1 == HIGH) {
    sharpLeft();
    lastState = 1;
  } 
  else if (R2 == HIGH) {
    sharpRight();
    lastState = 2;
  }
  // 第二順位：中間兩顆在線上，執行前進
  else if (L2 == HIGH || R1 == HIGH) {
    forward();
    lastState = 0; 
  } 
  // 第三順位：全白空窗期 (記憶救回)
  else {
    if (lastState == 1) {
      sharpLeft(); 
    } 
    else if (lastState == 2) {
      sharpRight();
    }
    else {
      // 這裡不放 stopMotor() 以防地圖上的虛線導致車子停止
      forward(); // 或者是維持最後狀態，看地圖需求
    }
  }
  delay(5); // 縮短延遲提高對 90 度彎的反應速度
}