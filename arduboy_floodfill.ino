//====================================================================================
// Flood Fill
//  by Gary Franz
//
// v1.0.0 - Dec 17, 2016 - Initial Version
// v1.1.0 - Dec 22, 2016 - Save last level to eeprom. New tiles for better visibility.
//                         Fixed faulty logic used to determine flooded tiles.
//                         Increased stack size, found a level that was needing to exceed it.
// v1.1.1 - Dec 22, 2016 - Added Help screen to display QR code that links to github page.
//                         Changed title screen to make it quicker to change levels.
//
//====================================================================================

#include <Arduboy2.h>
#include <EEPROM.h>

//read on arduboy community site to have a unique save location
#define SAVE (EEPROM_STORAGE_SPACE_START + 353) //fld-353

//the maximum size for the stack used in the flood fill algorithm
//some levels bugged out a little when the stack size was set around 100-128
//this is probably overkill at 220, but level 167 would reach max stack size of 192
static const int MAXSTACK PROGMEM = 220; 

//total number of levels
static const int MAXLEVEL PROGMEM = 999;

//seems to be challenging on some levels
//at least requires you to think instead of just randomly selecting tiles
static const int MOVES PROGMEM = 20; 

//game states
static const int TITLE PROGMEM = 0;
static const int GAME PROGMEM = 1;
static const int WIN PROGMEM = 2;
static const int LOSE PROGMEM = 3;
static const int HELP PROGMEM = 4;

//board setup
static const int SCREENWIDTH PROGMEM = 128;
static const int TILEWIDTH PROGMEM = 8;
static const int TILEHEIGHT PROGMEM = 8;
static const int NUMBERTILESHORIZONTAL PROGMEM = 12;
static const int NUMBERTILESVERTICAL PROGMEM = 8;
static const int SELECTORSIZE PROGMEM = 12;

//different shapes/patterns/tiles
const unsigned char PATTERN1[] PROGMEM = {
  0x55, 0x00, 0xaa, 0x00, 0x55, 0x00, 0xaa, 0x00, 
};
const unsigned char PATTERN2[] PROGMEM = {
  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 
};
const unsigned char PATTERN3[] PROGMEM = {
  0x55, 0xff, 0xaa, 0xff, 0x55, 0xff, 0xaa, 0xff,
};
const unsigned char PATTERN4[] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
};
const unsigned char PATTERN5[] PROGMEM = {
  0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 
};
const unsigned char PATTERN6[] PROGMEM = {
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
};

const unsigned char SADFACE[] PROGMEM = {
  0x00, 0x00, 0xc0, 0xe0, 0x30, 0x18, 0xc, 0xc, 0x6, 0x6, 
  0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0xc, 0xc, 0x18, 0x30, 0xe0, 
  0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
  0x7, 0x7, 0x7, 0x00, 0x00, 0x00, 0x00, 0x7, 0x7, 0x7, 0x00, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x3, 0x7, 
  0xc, 0x18, 0x30, 0x31, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 
  0x61, 0x61, 0x31, 0x30, 0x18, 0xc, 0x7, 0x3, 0x00, 0x00, 
};

const unsigned char HAPPYFACE[] PROGMEM = {
  0x00, 0x00, 0xc0, 0xe0, 0x30, 0x18, 0xc, 0xc, 0x6, 0x6, 0x6, 
  0x6, 0x6, 0x6, 0x6, 0x6, 0xc, 0xc, 0x18, 0x30, 0xe0, 0xc0, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x40, 0x87, 
  0x7, 0x7, 0x00, 0x00, 0x00, 0x00, 0x7, 0x7, 0x87, 0x40, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x3, 0x7, 0xc, 0x18, 
  0x30, 0x30, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 
  0x30, 0x30, 0x18, 0xc, 0x7, 0x3, 0x00, 0x00, 
};

const unsigned char QRCODE[] PROGMEM = {

  0xff, 0xff, 0xff, 0x7, 0x7, 0xe7, 0xe7, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 
  0xe7, 0xe7, 0x7, 0x7, 0xff, 0xff, 0x67, 0x67, 0x1f, 0x1f, 0x7, 0x7, 0x9f, 
  0x9f, 0x87, 0x87, 0x7, 0x7, 0x67, 0x67, 0xe7, 0xe7, 0x7, 0x7, 0x87, 0x87, 
  0x7f, 0x7f, 0xe7, 0xe7, 0x87, 0x87, 0xff, 0xff, 0x7, 0x7, 0xe7, 0xe7, 0x67, 
  0x67, 0x67, 0x67, 0x67, 0x67, 0xe7, 0xe7, 0x7, 0x7, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x7f, 0x7f, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7f, 
  0x7f, 0x00, 0x00, 0xff, 0xff, 0x60, 0x60, 0xf8, 0xf8, 0x18, 0x18, 0x87, 0x87, 
  0x61, 0x61, 0xf8, 0xf8, 0x00, 0x00, 0xe7, 0xe7, 0x60, 0x60, 0x81, 0x81, 0x1e, 
  0x1e, 0x87, 0x87, 0x19, 0x19, 0xff, 0xff, 0x00, 0x00, 0x7f, 0x7f, 0x60, 0x60, 
  0x60, 0x60, 0x60, 0x60, 0x7f, 0x7f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0x86, 0x86, 0x6, 0x6, 0x66, 0x66, 0x6, 0x6, 0x9e, 0x9e, 0xfe, 0xfe, 0x66, 
  0x66, 0x1f, 0x1f, 0x6, 0x6, 0x7f, 0x7f, 0x00, 0x00, 0x67, 0x67, 0x00, 0x00, 0x9f, 
  0x9f, 0xe0, 0xe0, 0xe1, 0xe1, 0x80, 0x80, 0xe1, 0xe1, 0xe6, 0xe6, 0x87, 0x87, 0x98, 
  0x98, 0x67, 0x67, 0x9e, 0x9e, 0x1e, 0x1e, 0x86, 0x86, 0xe6, 0xe6, 0x66, 0x66, 0x7e, 
  0x7e, 0x86, 0x86, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x19, 0x19, 0x9e, 0x9e, 0x80, 
  0x80, 0x6, 0x6, 0x19, 0x19, 0x19, 0x19, 0x66, 0x66, 0x9e, 0x9e, 0x18, 0x18, 0xe0, 
  0xe0, 0xe0, 0xe0, 0x1e, 0x1e, 0x86, 0x86, 0x81, 0x81, 0x7, 0x7, 0x81, 0x81, 0x99, 
  0x99, 0x87, 0x87, 0xff, 0xff, 0x1f, 0x1f, 0x79, 0x79, 0xf8, 0xf8, 0x19, 0x19, 0xfe, 
  0xfe, 0xf9, 0xf9, 0xe7, 0xe7, 0x6, 0x6, 0x1e, 0x1e, 0x19, 0x19, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0x78, 0x78, 0x9f, 0x9f, 0xe7, 0xe7, 0xfe, 0xfe, 0xe6, 0xe6, 0x6, 
  0x6, 0x66, 0x66, 0x19, 0x19, 0xf8, 0xf8, 0x61, 0x61, 0x1, 0x1, 0xf8, 0xf8, 0x7f, 
  0x7f, 0x67, 0x67, 0x78, 0x78, 0x19, 0x19, 0xe1, 0xe1, 0x1f, 0x1f, 0xe7, 0xe7, 0xe0, 
  0xe0, 0x60, 0x60, 0xf9, 0xf9, 0x78, 0x78, 0x19, 0x19, 0x61, 0x61, 0x87, 0x87, 0x1e, 
  0x1e, 0x80, 0x80, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e, 0x67, 
  0x67, 0x67, 0x67, 0x79, 0x79, 0x61, 0x61, 0x60, 0x60, 0x66, 0x66, 0xfe, 0xfe, 0x99, 
  0x99, 0x9e, 0x9e, 0x00, 0x00, 0x1, 0x1, 0xf8, 0xf8, 0xfe, 0xfe, 0xe6, 0xe6, 0xfe, 
  0xfe, 0x81, 0x81, 0x80, 0x80, 0xe7, 0xe7, 0x7f, 0x7f, 0x6, 0x6, 0xe7, 0xe7, 0x60, 0x60, 
  0xe0, 0xe0, 0x00, 0x00, 0x7, 0x7, 0x80, 0x80, 0x1f, 0x1f, 0x9f, 0x9f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0x00, 0x00, 0xfe, 0xfe, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0xfe, 0xfe, 0x00, 
  0x00, 0xff, 0xff, 0x1f, 0x1f, 0x1, 0x1, 0x86, 0x86, 0xe6, 0xe6, 0x1, 0x1, 0xe1, 0xe1, 0x7, 
  0x7, 0x81, 0x81, 0x81, 0x81, 0x67, 0x67, 0x67, 0x67, 0x9e, 0x9e, 0x60, 0x60, 0x87, 0x87, 
  0xe6, 0xe6, 0x7, 0x7, 0x80, 0x80, 0x98, 0x98, 0x67, 0x67, 0xfe, 0xfe, 0x7, 0x7, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xe0, 0xe7, 0xe7, 0xe6, 0xe6, 0xe6, 0xe6, 0xe6, 0xe6, 
  0xe7, 0xe7, 0xe0, 0xe0, 0xff, 0xff, 0xe0, 0xe0, 0xe0, 0xe0, 0xe1, 0xe1, 0xff, 0xff, 0xf8, 
  0xf8, 0xe7, 0xe7, 0xfe, 0xfe, 0xf9, 0xf9, 0xff, 0xff, 0xfe, 0xfe, 0xe6, 0xe6, 0xe7, 0xe7, 
  0xe0, 0xe0, 0xe1, 0xe1, 0xe7, 0xe7, 0xf8, 0xf8, 0xff, 0xff, 0xf9, 0xf9, 0xfe, 0xfe, 0xe1, 
  0xe1, 0xfe, 0xfe, 0xff, 0xff, 0xff, 
  
};

Arduboy2 arduboy;

int countTotalFlood;
int countTotalMoves;
int level;

char buttonAvailable;
char arrowAvailable;
char newTile;
char originalTile;
char state;
char titleSelection;

char gameTiles[NUMBERTILESHORIZONTAL][NUMBERTILESVERTICAL];
char floodedTiles[NUMBERTILESHORIZONTAL][NUMBERTILESVERTICAL];

struct point {
  char x;
  char y;
};
typedef struct point POINT;

struct stack {
  POINT stk[MAXSTACK];
  int top;
};
typedef struct stack STACK;
STACK floodStack;

//===================================== SETUP =====================================
void setup() {

  arduboy.begin();
  arduboy.clear();

  arduboy.setFrameRate(60); //not sure if this is needed or should be different for this game
  
  //make sure we start out with buttons available to be pressed
  buttonAvailable = 1; 
  arrowAvailable = 1;

  // hundreds-0, tenths-1, ones-2, help button-3
  titleSelection = 2;

  newTile = 0;
  originalTile = 0;
  state = TITLE; //start the game out on the title screen
  
  EEPROM.get(SAVE, level);
  if(level > MAXLEVEL) {
    level = MAXLEVEL;
  }
  if(level < 1) {
    level = 1;
  }

}

//===================================== LOOP =====================================
void loop() {

  if(!arduboy.nextFrame()) {
    return;
  }
  
  arduboy.clear();

  //what screen/state do we need to display?
  switch (state) {

  case HELP:

    if (((arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON)) && buttonAvailable == 1) ||
        ((arduboy.pressed(UP_BUTTON) || arduboy.pressed(DOWN_BUTTON) || arduboy.pressed(LEFT_BUTTON) || arduboy.pressed(RIGHT_BUTTON)) && arrowAvailable == 1)){
      
      buttonAvailable = 0;
      arrowAvailable = 0;
      state = TITLE;

    }

    arduboy.drawBitmap(32, 0, QRCODE, 64, 64, WHITE); 

    break; //HELP

  case TITLE:

    if (arduboy.pressed(A_BUTTON) && buttonAvailable == 1) {
      //start the game or view help, depends on selection

      buttonAvailable = 0;

      if (titleSelection == 3) {
        //show help
        state = HELP;
        
      } else {
        //start game
        state = GAME;

        EEPROM.put(SAVE, level);

        setupNewGame();
      }
      
      

    }

    if (arduboy.pressed(UP_BUTTON) == true && arrowAvailable == 1) {
      //increment the level based on cursor

      arrowAvailable = 0;

      switch(titleSelection) {
        case 0:
          //increment hundreds 
          if (level + 100 <= MAXLEVEL) {
            level = level + 100;
          } else {
            level = MAXLEVEL;
          }
          break;
          
        case 1:
          //increment tens
          if (level + 10 <= MAXLEVEL) {
            level = level + 10;
          } else {
            level = MAXLEVEL;
          }
          break;
          
        case 2:
          //increment ones
          if (level + 1 <= MAXLEVEL) {
            level++;
          }
          break;
      }
      
    }

    if (arduboy.pressed(DOWN_BUTTON) == true && arrowAvailable == 1) {
      //decrement the level based on cursor

      arrowAvailable = 0;

      switch(titleSelection) {
        case 0:
          //decrement hundreds 
          if (level - 100 >= 1) {
            level = level - 100;
          } else {
            level = 1;
          }
          break;
          
        case 1:
          //decrement tens
          if (level - 10 >= 1) {
            level = level - 10;
          } else {
            level = 1;
          }
          break;
          
        case 2:
          //decrement ones
          if (level - 1 >= 1) {
            level--;
          }
          break;
      }

    }

    if (arduboy.pressed(LEFT_BUTTON) == true && arrowAvailable == 1) {
      //move selector left

      arrowAvailable = 0;

      if (titleSelection > 0) {
        titleSelection--;
      }

    }

    if (arduboy.pressed(RIGHT_BUTTON) == true && arrowAvailable == 1) {
      //move selector right

      arrowAvailable = 0;

      if (titleSelection < 3) {
        titleSelection++;
      }
      
    }

    //display title screen stuff
    arduboy.setCursor(4, 2);
    arduboy.setTextSize(2);
    arduboy.print("FLOOD FILL");
    arduboy.drawLine(0, 17, 128, 17, WHITE);
    
    arduboy.setTextSize(1);
    arduboy.setCursor(12, 22);
    arduboy.print("Select");
    arduboy.setCursor(15, 30);
    arduboy.print("Level");

    arduboy.setCursor(84, 26);
    arduboy.print("Help");

    arduboy.setTextSize(2);
    if (level < 10) {
      arduboy.setCursor(12, 44);
      arduboy.print("00");
      arduboy.setCursor(36, 44);
      arduboy.print(level);
    } else if (level < 100) {
      arduboy.setCursor(12, 44);
      arduboy.print("0");
      arduboy.setCursor(24, 44);
      arduboy.print(level);
    } else {
      arduboy.setCursor(12, 44);
      arduboy.print(level);
    }

    if (titleSelection == 2) {
        arduboy.drawLine(36, 62, 46, 62, WHITE);
    } else if (titleSelection == 1) {
        arduboy.drawLine(24, 62, 34, 62, WHITE);
    } else if (titleSelection == 0) {
        arduboy.drawLine(12, 62, 22, 62, WHITE);
    } else {
      arduboy.drawLine(84, 35, 106, 35, WHITE);
    }

    break; //TITLE

    
  case GAME:

    if (arduboy.pressed(UP_BUTTON) == true && arrowAvailable == 1) {

      if (newTile >= 2) {
        newTile = newTile - 2;
      }

      arrowAvailable = 0;
    }

    if (arduboy.pressed(DOWN_BUTTON) == true && arrowAvailable == 1) {

      if (newTile <= 3) {
        newTile = newTile + 2;
      }

      arrowAvailable = 0;
    }

    if (arduboy.pressed(LEFT_BUTTON) == true && arrowAvailable == 1) {

      if (newTile == 1 || newTile == 3 || newTile == 5) {
        newTile--;
      }

      arrowAvailable = 0;
    }

    if (arduboy.pressed(RIGHT_BUTTON) == true && arrowAvailable == 1) {

      if (newTile == 0 || newTile == 2 || newTile == 4) {
        newTile++;
      }

      arrowAvailable = 0;
    }

    if (arduboy.pressed(A_BUTTON) == true && buttonAvailable == 1) {
      //attempt to flood fill using the selected tile
      buttonAvailable = 0;
      flood();
    }

    if (arduboy.pressed(B_BUTTON) == true && buttonAvailable == 1) {
      //go back to the title screen
      buttonAvailable = 0;
      state = TITLE;
      countTotalMoves = MOVES;
    }

    //draw the actual game board
    drawBoard();

    //draw selection tiles
    //ordered like so:
    // 0 1
    // 2 3
    // 4 5
    arduboy.drawBitmap(3, 28, PATTERN1, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 0
    arduboy.drawBitmap(3, 41, PATTERN3, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 2
    arduboy.drawBitmap(3, 54, PATTERN5, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 4
    arduboy.drawBitmap(16, 28, PATTERN2, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 1
    arduboy.drawBitmap(16, 41, PATTERN4, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 3
    arduboy.drawBitmap(16, 54, PATTERN6, TILEWIDTH, TILEHEIGHT, WHITE); //newTile 5

    //draw newTile selection rectangle
    switch (newTile) {
    case 0:
      arduboy.drawRect(1, 26, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;
    case 1:
      arduboy.drawRect(14, 26, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;
    case 2:
      arduboy.drawRect(1, 39, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;
    case 3:
      arduboy.drawRect(14, 39, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;
    case 4:
      arduboy.drawRect(1, 52, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;
    case 5:
      arduboy.drawRect(14, 52, SELECTORSIZE, SELECTORSIZE, WHITE);
      break;

    }

    arduboy.drawLine(0, 9, 29, 9, WHITE);
    arduboy.drawLine(0, 21, 29, 21, WHITE);

    arduboy.setTextSize(1);
    arduboy.setCursor(2, 0);
    arduboy.print("#");
    arduboy.print(level);

    if (countTotalMoves < 4) {
      
      arduboy.setCursor(5, 12);
      arduboy.print("*");
      arduboy.print(countTotalMoves);
      arduboy.print("*");
      
    } else if (countTotalMoves < 10) {
      
      arduboy.setCursor(11, 12);
      arduboy.print(countTotalMoves);
      
    } else {
      
      arduboy.setCursor(8, 12);
      arduboy.print(countTotalMoves);
      
    }
    

    break; //GAME

  case WIN:

    //draw the flooded game board
    drawBoard();

    arduboy.drawBitmap(0, 28, HAPPYFACE, 24, 24, WHITE);

    arduboy.setTextSize(1);
    arduboy.setCursor(4, 0);
    arduboy.print("You");
    arduboy.setCursor(4, 9);
    arduboy.print("Won");

    if ((arduboy.pressed(UP_BUTTON) ||
        arduboy.pressed(DOWN_BUTTON) ||
        arduboy.pressed(LEFT_BUTTON) ||
        arduboy.pressed(RIGHT_BUTTON) ||
        arduboy.pressed(A_BUTTON) ||
        arduboy.pressed(B_BUTTON)) && buttonAvailable == 1 && arrowAvailable == 1) {
      //any button pressed will start next game
      
      buttonAvailable = 0;
      arrowAvailable = 0;
      state = GAME;

      level++;
      if (level == MAXLEVEL + 1) {
        level = 1;
      }

      EEPROM.put(SAVE, level);
      
      setupNewGame();
      
    }

    break; //WIN

  case LOSE: 

    //draw the final game board
    drawBoard();

    arduboy.drawBitmap(0, 28, SADFACE, 24, 24, WHITE);

    arduboy.setTextSize(1);
    arduboy.setCursor(4, 0);
    arduboy.print("You");
    arduboy.setCursor(1, 9);
    arduboy.print("Lost");
    
    if ((arduboy.pressed(UP_BUTTON) ||
        arduboy.pressed(DOWN_BUTTON) ||
        arduboy.pressed(LEFT_BUTTON) ||
        arduboy.pressed(RIGHT_BUTTON) ||
        arduboy.pressed(A_BUTTON) ||
        arduboy.pressed(B_BUTTON)) && buttonAvailable == 1 && arrowAvailable == 1) {
      //any button pressed will start next game

      buttonAvailable = 0;
      arrowAvailable = 0;
      state = GAME;

      setupNewGame();
    }

    break; //LOSE

  } //switch(state)

  if (buttonAvailable == 0) {
    if (!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON)) {
      //button is no longer pressed, make sure to allow another button to be pressed
      buttonAvailable = 1;
    }
  }
  if (arrowAvailable == 0) {
    if (!arduboy.pressed(UP_BUTTON) && !arduboy.pressed(DOWN_BUTTON) && !arduboy.pressed(LEFT_BUTTON) && !arduboy.pressed(RIGHT_BUTTON)) {
      //arrow is no longer pressed, make sure to allow another arrow to be pressed
      arrowAvailable = 1;
    }
  }

  arduboy.display();

}

//===================================== GAME BOARD =====================================
void drawBoard() {

  arduboy.drawLine(29, 0, 29, 64, WHITE);

  char backgroundx;
  char backgroundy;

  for (char x = 0; x < NUMBERTILESHORIZONTAL; x++) {
    for (char y = 0; y < NUMBERTILESVERTICAL; y++) {

      backgroundx = SCREENWIDTH - (NUMBERTILESHORIZONTAL - x) * TILEWIDTH;
      backgroundy = y * TILEHEIGHT;

      switch (gameTiles[x][y]) {

      case 0:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN1, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      case 1:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN2, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      case 2:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN3, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      case 3:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN4, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      case 4:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN5, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      case 5:
        arduboy.drawBitmap(backgroundx, backgroundy, PATTERN6, TILEWIDTH, TILEHEIGHT, WHITE);
        break;

      }

    }
  }

}

void createGameBoard() {

  //use the level as a seed
  srand(level);

  for (char w = 0; w < NUMBERTILESHORIZONTAL; w++) {

    for (char h = 0; h < NUMBERTILESVERTICAL; h++) {

      gameTiles[w][h] = rand() % 6;
      //floodedTiles[w][h] = 0;
      ResetFloodedTiles();

    }
  }

  originalTile = gameTiles[0][0];

}

void setupNewGame() {
  
  countTotalFlood = 0;
  countTotalMoves = MOVES;
  createGameBoard(); 
   
}

//===================================== FLOOD =====================================
void flood() {

  originalTile = gameTiles[0][0];

  if (newTile != originalTile) {

    countTotalMoves--;

    FloodFill();
    FindAndCountFloodedTiles();

    if (countTotalFlood == NUMBERTILESHORIZONTAL * NUMBERTILESVERTICAL) {
      //win
      state = WIN;

    } else if (countTotalMoves == 0) {
      //lose 
      state = LOSE;

    }

  }

}

void FloodFill() {

  //reset the stack
  floodStack.top = -1;

  POINT p;
  p.x = 0;
  p.y = 0;

  POINT p1;

  push(p);

  while (floodStack.top >= 0) {

    p = pop();

    if (p.y < 0 || p.y > NUMBERTILESVERTICAL - 1 || p.x < 0 || p.x > NUMBERTILESHORIZONTAL - 1) {
      //do nothing in this case
    } else {
      if (gameTiles[p.x][p.y] == originalTile) {

        gameTiles[p.x][p.y] = newTile;

        p1.x = p.x + 1;
        p1.y = p.y;
        if (!(p1.y < 0 || p1.y > NUMBERTILESVERTICAL - 1 || p1.x < 0 || p1.x > NUMBERTILESHORIZONTAL - 1)) {
          //only push if the new point is legal
          push(p1);
        }

        p1.x = p.x - 1;
        p1.y = p.y;
        if (!(p1.y < 0 || p1.y > NUMBERTILESVERTICAL - 1 || p1.x < 0 || p1.x > NUMBERTILESHORIZONTAL - 1)) {
          //only push if the new point is legal
          push(p1);
        }

        p1.x = p.x;
        p1.y = p.y + 1;
        if (!(p1.y < 0 || p1.y > NUMBERTILESVERTICAL - 1 || p1.x < 0 || p1.x > NUMBERTILESHORIZONTAL - 1)) {
          //only push if the new point is legal
          push(p1);
        }

        p1.x = p.x;
        p1.y = p.y - 1;
        if (!(p1.y < 0 || p1.y > NUMBERTILESVERTICAL - 1 || p1.x < 0 || p1.x > NUMBERTILESHORIZONTAL - 1)) {
          //only push if the new point is legal
          push(p1);
        }

      }
    }

  }

}

void ResetFloodedTiles() {
  
  for (char w = 0; w < NUMBERTILESHORIZONTAL; w++) {
    for (char h = 0; h < NUMBERTILESVERTICAL; h++) {
      floodedTiles[w][h] = 0;
    }
  }
  
}

void FindAndCountFloodedTiles() {
  //reset the stack
  floodStack.top = -1;

  countTotalFlood = 0;

  ResetFloodedTiles();

  POINT p;
  p.x = 0;
  p.y = 0;

  POINT p1;

  push(p);

  while (floodStack.top >= 0) {

    p = pop();

    if (p.y < 0 || p.y > NUMBERTILESVERTICAL - 1 || p.x < 0 || p.x > NUMBERTILESHORIZONTAL - 1) {
      //do nothing in this case
    } else {
      if (gameTiles[p.x][p.y] == newTile && floodedTiles[p.x][p.y] != 1) {
        
        floodedTiles[p.x][p.y] = 1; //indicate this point is flooded
        countTotalFlood++;

        p1.x = p.x + 1;
        p1.y = p.y;
          push(p1);

        p1.x = p.x - 1;
        p1.y = p.y;
          push(p1);

        p1.x = p.x;
        p1.y = p.y + 1;
          push(p1);

        p1.x = p.x;
        p1.y = p.y - 1;
          push(p1);

      }
    }

  }
}

//===================================== STACK IMPLEMENTATION =====================================
void push(POINT p) {
  if (floodStack.top == (MAXSTACK - 1)) {
    return;
  } else {
    floodStack.top = floodStack.top + 1;
    floodStack.stk[floodStack.top] = p;
  }
  return;
}

POINT pop() {
  POINT p;
  if (floodStack.top == -1) {
    return (p);
  } else {
    p = floodStack.stk[floodStack.top];
    floodStack.top = floodStack.top - 1;
  }
  return (p);
}
