#include <Servo.h>

#define INIT   0
#define GOING  1
#define ATTACK 2
#define RIGHT  3
#define LEFT   4
#define STOP   5
#define BACK   6
#define RANDOM 7

#define ENA  5
#define ENB  11
#define IN1  6
#define IN2  7
#define IN3  8
#define IN4  9
#define ECHO A4
#define TRIG A5

#define ABS  115

#define SX     10
#define TAVOLO 2
#define DX     4
#define SERVO  3

#define STRAIGHT_ANGLE 100
#define RIGHT_ANGLE    45
#define LEFT_ANGLE     165

volatile int state;
unsigned long timer;
int tavolo, sx, dx;
volatile int pendingInterrupt;
Servo myServo;

int getDistance() {
    digitalWrite(TRIG, LOW);     
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);    
    delayMicroseconds(20);
    digitalWrite(TRIG, LOW);     
    float distance = pulseIn(ECHO, HIGH);    
    distance = distance/58;
    Serial.println(distance);
    return (int)distance;
}

void forward() {
    analogWrite(ENA, ABS);
    analogWrite(ENB, ABS);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("go forward!");
}

void back() {
    analogWrite(ENA, ABS);
    analogWrite(ENB, ABS);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.println("go back!");
}

void left() {
    analogWrite(ENA, ABS);
    analogWrite(ENB, ABS);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW); 
    Serial.println("go left!");
}

void right() {
    analogWrite(ENA, ABS);
    analogWrite(ENB, ABS);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.println("go right!");
} 

void stop() {
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);
    Serial.println("Stop!");
} 

// vero quando non c'e' niente, falso quando c'e' l'ostacolo (led acceso)
void readInput() {
    sx = digitalRead(SX);
    tavolo = digitalRead(TAVOLO);
    dx = digitalRead(DX);
}

// quando si spegne il led centrale, forza il cambio di stato
// CI SONO GROSSI PROBLEMI DI CONCORRENZA DA GESTIRE
void missingTableInterrupt() {
    stop();
    state = STOP;
    pendingInterrupt = 1;
}

void changeState(int newState) {
    noInterrupts();
    if (!pendingInterrupt) {
        state = newState;
    }
    interrupts();
}

// ritorna 1 se il delay viene interrotto prematuramente dall'interrupt
int interruptibleDelay(int ms) {
    unsigned long localTimer = millis();
    while (millis() - localTimer < ms) {
        if (pendingInterrupt)
            return 1;
    }
    return 0;
}

void setup() {
    Serial.begin(9600);   
    
    pinMode(ECHO, INPUT);        
    pinMode(TRIG, OUTPUT);    
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    stop();
    
    myServo.attach(SERVO);
    myServo.write(STRAIGHT_ANGLE);
    
    randomSeed(analogRead(A0));
    state = INIT;
    timer = millis();
    pendingInterrupt = 0;

    attachInterrupt(digitalPinToInterrupt(TAVOLO), missingTableInterrupt, RISING);
}

void loop() {
    readInput();

    switch (state) {
        case INIT:
            if (tavolo) {
                stop();
            } else {
                forward();
                changeState(GOING);
            }
            break;
        case GOING:
            if (tavolo) {
                stop();
                changeState(STOP);
            } else if (!sx && !dx) {
                forward();
                changeState(ATTACK);
            } else if (!sx && dx) {
                left();
                timer = millis();
                changeState(LEFT);
            } else if (sx && !dx) {
                right();
                timer = millis();
                changeState(RIGHT);
            } else {
                // prima ha controllato gli infrarossi
                // se non c'e' niente controlla il sonar
                myServo.write(STRAIGHT_ANGLE);
                if (interruptibleDelay(random(600, 900))) return;  // ferma loop() se nel mentre sparisce il tavolo
                float distance = getDistance();

                // ostacolo davanti
                if (distance <= 40) {
                    forward();
                    changeState(ATTACK);
                } else {
                    stop();
                    myServo.write(LEFT_ANGLE); delay(random(600, 900));
                    distance = getDistance();

                    // ostacolo a sinistra
                    if (distance <= 40) {
                        myServo.write(STRAIGHT_ANGLE);
                        left(); delay(random(1000, 1500));
                        forward();
                        changeState(ATTACK);
                    } else {
                        myServo.write(RIGHT_ANGLE); delay(random(600, 900));
                        distance = getDistance();

                        // ostacolo a destra
                        if (distance <= 40) {
                            myServo.write(STRAIGHT_ANGLE);
                            right(); delay(random(1000, 1500));
                            forward();
                            changeState(ATTACK);
                        } else {
                            // non c'e' niente, prosegui
                            myServo.write(STRAIGHT_ANGLE);
                            forward();
                            if (interruptibleDelay(random(1500, 3000))) return;  // ferma loop() se nel mentre sparisce il tavolo
                        }
                    }
                }
            }
            break;
        case RIGHT:
        case LEFT:
            if (millis() - timer > 2000) {
                forward();
                changeState(GOING);
            } else if (!sx && !dx) {
                forward();
                changeState(ATTACK);
            }
            break;
        case ATTACK:
            if (tavolo) {
                stop();
                changeState(STOP);
            } else if (!sx && dx) {
                left(); delay(random(100, 400));
                forward();
                if (interruptibleDelay(random(100, 400))) return;
            } else if (sx && !dx) {
                right(); delay(random(100, 400));
                forward();
                if (interruptibleDelay(random(100, 400))) return;
            }
            break;
        case STOP:
            stop();
            back();
            if (pendingInterrupt) pendingInterrupt = 0;
            timer = millis();
            changeState(BACK);
            break;
        case BACK:
            if (millis() - timer > 500) {
                stop();
                changeState(RANDOM);
            }
            break;
        case RANDOM:
            int val = random(0, 2);
            if (val) {
                right();
                timer = millis();
                delay(random(2000, 3000));
                changeState(RIGHT);
            } else {
                left();
                timer = millis();
                delay(random(2000, 3000));
                changeState(LEFT);
            }
            break;
        default:
            break;
    }
}
