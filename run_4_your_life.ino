#include <LiquidCrystal.h>

int VELOCITY = 200;

#define NOTE_G4  392
#define NOTE_G5  784
#define NOTE_G6  1568

#define START_BUTTON 13
#define PANEL_POSITION 14
#define MAX_INITIAL_OBJECTS_POSITION_X 13
#define vrx A0  
#define vry A1  
#define BATTERY_POSSIBILITY 30

#define MAX_ENERGY 100
#define SCORE_TO_WIN 100
#define MIN_SCORE 0

#define OBSTACLE_1 1
#define OBSTACLE_2 2
#define OBSTACLE_3 3
#define OBSTACLE_4 4

#define CAR 5
//#define ACCELERATOR 6
#define EXPLOSION 6
#define BATTERY 7
#define FLAG 8

const byte obstacleForm1[8]   = {B10010,B11011,B11011,B11011,B11011,B11011,B11011,B01001}; // OK - Barreira
const byte obstacleForm2[8]   = {B00100,B00100,B01110,B00000,B00000,B00100,B00100,B01110}; // OK - Cones
const byte obstacleForm3[8]   = {B00000,B01110,B10011,B10001,B10001,B11001,B01110,B00000}; // OK - Buraco
const byte obstacleForm4[8]   = {B00000,B00000,B10010,B11111,B11111,B10010,B00000,B00000}; // OK - Outro carro

const byte carForm[8]         = {B00000,B00000,B10010,B11111,B11111,B10010,B00000,B00000}; // OK
const byte explosionForm[8]   = {B01010,B10101,B01010,B10001,B10001,B01010,B10101,B01010}; // OK
const byte batteryForm[8]     = {B01110,B11011,B10001,B10101,B10101,B10101,B10001,B11111}; // OK - Battery
const byte flagForm[8]        = {B10111,B11001,B10111,B11000,B10000,B10000,B10000,B10000}; // OK
//const byte acceleratorForm[8] = {B00000,B10100,B01010,B00101,B00101,B01010,B10100,B00000}; // OK

int pyCar = 0, pxCar = 0;
//int pxAccelerator = 0, pyAccelerator = 0;
int pxExplosion = 0, pyExplosion = 0;
int pxBattery = MAX_INITIAL_OBJECTS_POSITION_X, pyBattery = 0;
//int pxFlag = 0, pyFlag = 0;
int pxObstacle = MAX_INITIAL_OBJECTS_POSITION_X, pyObstacle = 0;

bool accelerate = false;
bool game = false, hasShoot = false, hasBattery = false, playSound = false;
int score = MIN_SCORE, rotationEnergy = 0;
double energy = MAX_ENERGY;
byte obstacleGenerated = 0;

int switchValue = 0, xValue = 0, yValue = 0;

int PIEZO = 12;
long freqIn, blow1, blow2;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

enum STATUS { INITIAL, WIN, LOSE, NO_ENERGY };

void setup() {
  
  // inicializar serial
  Serial.begin(9600);
  
  // inicializar display
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(PIEZO, OUTPUT);
 
  // incluir personagens na memoria do display
  createCharacters();

}

void loop() {

  if(game) {

    lcd.clear();
    
    // imprimir pontos e energia
    drawPanel(PANEL_POSITION);

    // avalidar se o joystick esta direcionado para algum lugar e desenhar o carro
    availableCar();  
    
    // verificar se asteroid chegou no limite da tela
    availableObstacle();

    // avaliar se vai gerar uma bateria
    availableBattery();
    
    // avaliar se houve colisao entre carro e obstaculo
    availableCarAndObstacleCollision();

    // avaliar se houve colisao entre carro e bateria
    availableCarAndBatteryCollision();

    // avaliar se jogador atingiu pontuacao maxima para vitoria
    availableScoreGame();

    // avaliar se energia acabou
    availableBatteryShip();

    // avalia se deve incrementar a velocidade
    availableIncreaseVelocity();
    
    energy -= 0.25;

    playCarSound();
    
    delay(VELOCITY);
    
  } else {

    drawMessageStoppedGame(INITIAL);
    
    // se jogador apertou botao, entao iniciar o jogo
    if(digitalRead(START_BUTTON) == 0) {
      resetGame();
    }

  }

}

// avalia se deve incrementar a velocidade
void availableIncreaseVelocity() {
  if(accelerate) { 
    VELOCITY -= 15;
    Serial.println("VELOCITY: " + (String)VELOCITY);
    accelerate = false;
    score += 2; 
  }
  if(score > 0 && score % 10 == 0) {
    accelerate = true;
  }
}

// avaliar se energia acabou
void availableBatteryShip() {
  finishGame(energy < 0, NO_ENERGY);
}

// avaliar se jogador atingiu pontuacao maxima para vitoria
void availableScoreGame() {
  finishGame(score >= SCORE_TO_WIN, WIN);
}

void finishGame(bool validation, int character) {
  if(validation) {
    game = false;
    drawMessageStoppedGame(character);
    delay(5000);
    lcd.clear();
  }
}

// avaliar se houve colisao entre carro e bateria
void availableCarAndBatteryCollision() {

  //  Serial.println("hasBattery: " + (String)hasBattery);
  bool checkCollision = (pxCar == pxBattery) && (pyCar == pyBattery) && hasBattery;
  bool alternativeCheckCollision = ((pxCar == pxBattery+1) || (pxCar == pxBattery-1)) && (pyCar == pyBattery) && hasBattery;

  // houve uma colisao, remover bateria 
  if(checkCollision || alternativeCheckCollision) {
    energy+=10;
    
    // Serial.println("Incrementou energia");
    if(energy > MAX_ENERGY) { 
      energy = MAX_ENERGY;
      // Serial.println("Resetou energia");
    }
    
    hasBattery = false;
    pxBattery = MAX_INITIAL_OBJECTS_POSITION_X;
    playTakeBattery();
  }
  
}

// avaliar se vai gerar uma bateria
void availableBattery() {

  if(!hasBattery) {
    
    // se for 5... gerar uma bateria
    if(random(0, BATTERY_POSSIBILITY) == 5) {
      
      hasBattery = true;
      pxBattery = MAX_INITIAL_OBJECTS_POSITION_X;
      pyBattery = random(0, 2);

      if((pxBattery == pxObstacle || (pxBattery == pxObstacle + 1) || (pxBattery + 1 == pxObstacle) && pyBattery == pyObstacle)) {
        pxBattery = MAX_INITIAL_OBJECTS_POSITION_X + 10;
      }

    }
    
  } else {
    
    pxBattery--;
    
    // se bateria atingiu limite da tela
    if(pxBattery < 0) {
      hasBattery = false;
      pxBattery = MAX_INITIAL_OBJECTS_POSITION_X;
    } else {
       drawSomething(pxBattery, pyBattery, BATTERY);
    }
    
  }
  
}

// avaliar se houve colisao entre carro e obstaculo
void availableCarAndObstacleCollision() {

  bool checkCollision = (pxCar == pxObstacle) && (pyCar == pyObstacle);
  bool alternativeCheckCollision = (pxCar == pxObstacle+1) && (pyCar == pyObstacle);

  // houve uma colisao, remover obstaculo e carro e desenhar explosao
  if(checkCollision || alternativeCheckCollision) {    
    game = false;
    drawSomething(pxCar, pyCar, EXPLOSION);
    playExplosionSound();
    drawMessageStoppedGame(LOSE);
    pxObstacle = MAX_INITIAL_OBJECTS_POSITION_X;
    delay(5000);
    lcd.clear();
  }
  
}

// verificar se obstaculo chegou no limite da tela e desenhar em caso negativo
void availableObstacle() {
  
  // diminuir a posicao de um obstaculo
  pxObstacle--;
  
  // verificar se o obstaculo vai ser gerado no mesmo lugar que uma bateria, uma bandeira ou um acelerador
  if((pxBattery == pxObstacle || (pxBattery == pxObstacle + 1) || (pxBattery + 1 == pxObstacle) && pyBattery == pyObstacle)) {
    pxBattery = MAX_INITIAL_OBJECTS_POSITION_X + 10;
  }
  
  // desenhar obstaculo
  if(obstacleGenerated == 0) {
    obstacleGenerated = random(1, 5); // será gerado um obstaculo no formato de OBSTACLE_1 (mancha), OBSTACLE_2 (cones), OBSTACLE_3 (buraco) ou OBSTACLE_4 (outro carro)
  }
  
  drawSomething(pxObstacle, pyObstacle, obstacleGenerated);
  
  // se obstaculo atingiu limite da tela, voltar ele para a posicao inicial
  if(pxObstacle < 0) {
    score += 2;
    pxObstacle = MAX_INITIAL_OBJECTS_POSITION_X;
    pyObstacle = random(0, 2);
    obstacleGenerated = 0;
  }

}

// avaliar se o joystick esta direcionado para algum lugar e desenhar o carro
void availableCar() {
  
  // lendo a posicao do joystick
  xValue = analogRead(vrx);
  yValue = analogRead(vry);
  
  if((yValue+10) >= 660) { // se joystick for direcionado para baixo
    pyCar = 1;  
  } else if((yValue-10) <= 220) { // se joystick for direcionado para cima
    pyCar = 0;
  }
  // desenhar carro
  drawSomething(pxCar, pyCar, CAR); 

}

void resetGame() {
  score = MIN_SCORE;
  energy = MAX_ENERGY;
  game = true;
  VELOCITY = 200;
}

void drawExplosionShip(int px, int py) {
  lcd.clear();
  drawSomething(px, py, EXPLOSION);
  delay(1000);
  lcd.clear();
}

void drawSomething(int px, int py, int character) {
  lcd.setCursor(px, py);
  lcd.write(character);
}

void drawPanel(int px) {
  lcd.setCursor(px, 0);
  lcd.print(score);
  lcd.setCursor(px, 1);
  lcd.print(energy);
}

// desenhar a tela conforme status passado no parametro
void drawMessageStoppedGame(STATUS pStatus) { 

  String showScore = "";
  lcd.setCursor(0, 0);
  switch (pStatus) {
    case INITIAL: 
      lcd.write(CAR);
      lcd.setCursor(3, 0);
      lcd.print("RUN 4 LIFE");
       lcd.setCursor(15, 0);
       lcd.write(FLAG);
      lcd.setCursor(2, 1);
      lcd.print("PRESS BUTTON");
      break;
    case WIN: 
    case LOSE: 
    case NO_ENERGY: 
      lcd.clear();
      lcd.print(pStatus == WIN ? ":)  YOU WIN  :)" : (pStatus == LOSE ? ":( GAME OVER :(" : "BATTERY IS OVER :(") ); 
      lcd.setCursor(0, 1);
      showScore = " SCORE: " + (String)score + " PTS";
      lcd.print(showScore);
      break;
  }
  
}

 void playCarSound() {
    tone(PIEZO, 100, 500);
 }

void playExplosionSound() {
  for(int k = 0; k < 250; k++){
    blow1 = random(500,1000);
    blow2 = random(1,3);
    piezoTone(blow1,blow2);
  } 
}

void playTakeBattery() {
  
  // Play Fireball sound
  tone(PIEZO, NOTE_G4, 35);
  delay(35);
  tone(PIEZO, NOTE_G5, 35);
  delay(35);
  tone(PIEZO, NOTE_G6, 35);
  delay(35);
  noTone(8);
  
}

void piezoTone(long freq, long duration){
  
  long aSecond = 1000000;
  long period = aSecond/freq;
  duration = duration*1000;
  duration = duration/period;
  for(long k = 0; k < duration; k++){
    digitalWrite(PIEZO, HIGH);
    delayMicroseconds(period/2);
    digitalWrite(PIEZO, LOW);
    delayMicroseconds(period/2);
  }
  
} 

// Cria os personagens na memoria do display
void createCharacters() {
  
  // cria o obstaculo 1
  lcd.createChar(OBSTACLE_1, obstacleForm1);
  
  // cria o obstaculo 2
  lcd.createChar(OBSTACLE_2, obstacleForm2);

  // cria o obstaculo 3
  lcd.createChar(OBSTACLE_3, obstacleForm3);

  // cria o obstaculo 4
  lcd.createChar(OBSTACLE_4, obstacleForm4);

  // cria o carro
  lcd.createChar(CAR, carForm);

  // cria a explosao
  lcd.createChar(EXPLOSION, explosionForm);
  
  // cria a bateria
  lcd.createChar(BATTERY, batteryForm);

  // cria a bandeira 
  lcd.createChar(FLAG, flagForm);
  
}
