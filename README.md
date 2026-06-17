# Snake Game (SDL2)

A Snake game written in C/C++ using the SDL2 library.

## Project Description

This project was created as part of a university programming assignment. The game is based on the classic Snake game, where the player controls a snake that grows longer after collecting fruits while avoiding collisions with itself.

The project uses SDL2 for graphics rendering, event handling, and window management.

## Features

### Implemented

* Snake movement controlled by arrow keys
* Snake body following system using turning points
* Fruit spawning at random positions
* Fruit collision detection
* Snake growth after collecting fruit
* Self-collision detection
* Game over screen
* Restart game option (`N`)
* Exit game option (`ESC`)
* Information panel displaying:

  * elapsed game time
  * project information
* Automatic border navigation

### Additional Functionality

* Dynamic memory allocation for snake growth (`malloc` / `realloc`)
* Prevention of fruit spawning inside the snake body
* Configurable game constants using `#define`

## Controls

| Key | Action         |
| --- | -------------- |
| ↑   | Move Up        |
| ↓   | Move Down      |
| ←   | Move Left      |
| →   | Move Right     |
| N   | Start New Game |
| ESC | Exit Game      |

## Project Structure

The program consists of several logical modules:

### Drawing Functions

Responsible for rendering:

* text
* pixels
* lines
* rectangles

Functions:

* `DrawString()`
* `DrawPixel()`
* `DrawLine()`
* `DrawRectangle()`

### Game Logic

Functions:

* `move()`
* `directions()`
* `fruitTouch()`
* `collision()`
* `restartGame()`

### Data Structures

#### Direction

```cpp
enum Direction {
    STOP = 0,
    RIGHT,
    LEFT,
    DOWN,
    UP
};
```

#### TurningPoint

Stores positions where the snake changes direction.

```cpp
typedef struct {
    int x;
    int y;
    int dir;
} TurningPoint;
```

## Configuration

Game parameters can be easily modified at the beginning of the source code:

```cpp
#define SCREEN_WIDTH       640
#define SCREEN_HEIGHT      480
#define INIT_LENGTH        10
#define STEP_SIZE          5
#define MOVE_DELAY         100
#define FRUIT_SIZE         10
#define MENU_HEIGHT        54
```

## Compilation

The project requires SDL2.

Example compilation command:

```bash
g++ snake.cpp -o snake -lSDL2
```

Depending on the operating system and IDE configuration, additional SDL2 include/library paths may be required.

## Assignment Requirements

Implemented requirements:

* Basic Snake gameplay
* Snake growth after collecting fruit
* Self-collision detection
* Restart functionality
* Random fruit generation
* SDL2 graphical interface

## Author

**Michał Binek**

## License

This project was created for educational purposes as part of a university programming course.
