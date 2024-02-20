// author: Tomas Husar (xhusar11)
#include <Wire.h>
#include <U8g2lib.h>

// DISPLAY TYPE AND COMMUNICATION PROTOCOL
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// SPACESHIP BITMAP
const unsigned char spaceship_bitmap[] PROGMEM = {
  0b00100100,
  0b01000010,
  0b10011001,
  0b10111101,
  0b11100111,
  0b10000001,
  0b01000010,
  0b00111100,
};

// ASTEROID BITMAP
const unsigned char asteroid_bitmap[] PROGMEM = {
  0b0110,
  0b1101,
  0b1011,
  0b0110
};

// DELAY
int period = 50;  // delay in ms

// GAME STATS
int hp = 3;
int score = 0; 
int max_score = 9999;

// JOYSTICK VARIABLES
const int x_pin = A0;  // pin to read from
int x_value = 0; // x value from joystick
int x_middle;  // used to "normalize" the x_value 

// INITIAL SPACESHIP POSITIONS
int spaceship_size = 8;
int spaceship_x = 60;
int spaceship_y = u8g2.getHeight() - spaceship_size;

// ASTEROIDS
const int num_of_asteroids = 5;
int asteroid_x[num_of_asteroids];
int asteroid_y[num_of_asteroids];
int asteroid_speed = 2;
int asteroid_size = 4;

// MOVE THE SPACESHIP
void spaceship_movement (int x_value) {
  if (x_value > 900) {
    spaceship_x = min(spaceship_x + 3, u8g2.getWidth() - spaceship_size);
  } else if (x_value > 500) {
    spaceship_x = min(spaceship_x + 2, u8g2.getWidth() - spaceship_size);
  } else if (x_value > 100) {
    spaceship_x = min(spaceship_x + 1, u8g2.getWidth() - spaceship_size);
  } else if (x_value < -900) {
    spaceship_x = max(spaceship_x - 3, spaceship_size);
  } else if (x_value < -500) {
    spaceship_x = max(spaceship_x - 2, spaceship_size);
  } else if (x_value < -100) {
    spaceship_x = max(spaceship_x - 1, spaceship_size);
  }
}

// INITIALIZE ASTEROIDS
void asteroid_init () {
  for (int i = 0; i < num_of_asteroids; i++) {
    asteroid_y[i] = 0;
    asteroid_x[i] = random(u8g2.getWidth() - asteroid_size);
  }
}

// MOVE ALL ASTEROIDS (and increase the score if the player dodged the asteroid)
void asteroid_movement () {
  for (int i = 0; i < num_of_asteroids; i++) {
    asteroid_y[i] += asteroid_speed;

    // RESET ASTEROIDS
    if (asteroid_y[i] >= u8g2.getHeight()) {
      asteroid_y[i] = random(-20, 0);
      asteroid_x[i] = random(u8g2.getWidth() - asteroid_size);
      
      if (score < max_score) {
        score++;  // increment the score after dodging the asteroid
      }
    }
  }
}

// CHECK COLLISION WITH ASTEROIDS
bool collision_check (int spaceship_x, int spaceship_y, int asteroid_x, int asteroid_y) {
  //check if the objects overlap on x and y
  if(spaceship_x < asteroid_x + asteroid_size && spaceship_x + spaceship_size > asteroid_x &&
    spaceship_y < asteroid_y + asteroid_size && spaceship_y + spaceship_size > asteroid_y) {
      return true;
  } else {
    return false;
  }
}

void game_display () {
  u8g2.clearBuffer();
  
  u8g2.setFont(u8g2_font_profont10_mr);
  u8g2.setCursor(2, 6); // top left
  u8g2.print("hp:");
  u8g2.print(hp);
  
  u8g2.setCursor(78, 6); // top right
  u8g2.print("score:");
  u8g2.print(score);

  u8g2.drawXBMP(spaceship_x, spaceship_y, spaceship_size, spaceship_size, spaceship_bitmap);  // display spaceship
  
  for (int i = 0; i < num_of_asteroids; i++) {
    u8g2.drawXBM(asteroid_x[i], asteroid_y[i], asteroid_size, asteroid_size, asteroid_bitmap);  // display asteroids
  }

  u8g2.sendBuffer();
}

// GAME OVER CHECK
int game_over_check (int hp) {
  if (hp <= 0) {
    return true;
  } else {
    return false;
  }
}

// GAME OVER SCREEN
void game_over_print () {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_profont22_tf);
  u8g2.setCursor(10, 30); // top left
  u8g2.print("GAME OVER");

  u8g2.setFont(u8g2_font_profont15_tf);
  u8g2.setCursor(30, 50); // top right
  u8g2.print("score: ");
  u8g2.print(score);

  u8g2.sendBuffer();
}

// HP AND SCORE RESET
void game_restart () {
  hp = 3;
  score = 0;
}

// SETUP
void setup() {
  Serial.begin(115200);
  Serial.print("init");

  Wire.begin();
  u8g2.begin();
  u8g2.setDisplayRotation(U8G2_R0);

  x_middle = analogRead(x_pin);

  asteroid_init();
}

// LOOP
void loop() {
  int time_now = millis();
  x_value = analogRead(x_pin) - x_middle;

  asteroid_movement();
  spaceship_movement(x_value);

  for (int i = 0; i < num_of_asteroids; i++) {
    if (collision_check(spaceship_x, spaceship_y, asteroid_x[i], asteroid_y[i])) {
      hp--;
      asteroid_init();
      spaceship_x = 60;
      delay(1000);
    }
  }

  if (game_over_check(hp)) {
    game_over_print();
    delay(5000); // stop the game
    game_restart();
  }

  game_display(); //draw the game

  while (millis() < time_now + period) {
    // wait period ms
  }
}