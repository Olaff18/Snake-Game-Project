// tutorials used:
// https://lazyfoo.net/tutorials/SDL/

#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	   640
#define SCREEN_HEIGHT	   480
#define INIT_LENGTH		   10   // the initial length of a snake
#define MAX_TURNING_POINTS 1000 // max amount of turning points at once
#define MOVE_DELAY         100  // the delay between the possible turns
#define TOLERANCE          5	// tolerance of pixels touching - snake & fruit
#define STEP_SIZE          5	// the step size of the snake
#define SPEED_INT          2	// interval after which the snake speeds up
#define SPEED_FACT         40	// the factor by which the snake speeds up (%)
#define MENU_HEIGHT        54   // the px height of upper menu
#define FRUIT_MARGIN       10   // minimum fruit margin from edges
#define FRUIT_SIZE         10	// dimensions of the fruit
#define SEGMENT_SIZE       10	// dimensions of the snake element
#define DELAY			   50	// delay of the game - snakes speed
#define RED_INT 		   10	// interval after which red dot disappears (s)
#define RED_SPEED          20	// slowing factor of red dot (%)
#define RED_START          1	// fastest time of appearing of the red dot
#define RED_END            5	// longest time of appearing of the red dot
#define SCORE_FILE "scores.txt" // scores file name
#define BEST_COUNT 3			// amount of scores
#define MAX_NAME_LENGTH 20		// maximum length of a name

enum Direction{
	STOP = 0,
	RIGHT,
	LEFT,
	DOWN,
	UP
};

typedef struct {
    int x;
    int y;
    int dir;
} TurningPoint; // struct for collecting turning points

typedef struct {
    int score;
    char name[100];
} BestScore; // struct for collecting best scores

//------------------------------------------------
//-------- PROF. DERENIOWSKI'S FUNCTIONS ---------
//------------------------------------------------

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoci l w pionie (gdy dx = 0, dy = 1) 
// b¹d poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

#ifdef __cplusplus
extern "C"
#endif

//------------------------------------------------
//---------  FILE/SCORES FUNCTIONS ---------------
//------------------------------------------------

void loadBestScores(BestScore* scores) { // opening the file
    FILE* file = fopen(SCORE_FILE, "r");
    if (file == NULL) {
        // if the file doesnt exist init to default values
        for (int i = 0; i < BEST_COUNT; i++) {
            scores[i].score = 0;
            strcpy(scores[i].name, "---");  // placeholder for empty names
        }
        return;
    }

    for (int i = 0; i < BEST_COUNT; i++) {
        if (fscanf(file, "%d %s", &scores[i].score, scores[i].name) != 2) { // returns 2 only when succesful
            // default values if the file is incomplete
            scores[i].score = 0;
            strcpy(scores[i].name, "---");
        }
    }
    fclose(file);
}

void collectPlayerName(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, char* playerName, int red, int blue) { // collecting players name when top3 score
    int cursor = 0;
    playerName[0] = '\0';  // starting with empty name

    SDL_Event event;
    int done = 0;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && cursor > 0) {
                    playerName[--cursor] = '\0'; // backspade handling
                } else if (event.key.keysym.sym == SDLK_RETURN && cursor > 0) {
                    // confirm input if enter
                    done = 1;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // exit the input process
                    playerName[0] = '\0';
                    done = 1;
                } else {
                    // valid characters
                    if (cursor < MAX_NAME_LENGTH - 1 && ((event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) ||
                                                         (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9) ||
                                                          event.key.keysym.sym == SDLK_SPACE)) { 
                        playerName[cursor++] = (char)event.key.keysym.sym;
                        playerName[cursor] = '\0'; // ensuring it remains null terminated
                    }
                }
            }
        }

        // rendering the input prompt
        DrawRectangle(screen, 100, 100, SCREEN_WIDTH - 200, 100, red, blue);
        char text[128];
        sprintf(text, "Enter your name: %s", playerName);
        DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, 120, text, charset);

        // updating the screen
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}

//saving scores in the file
void saveBestScores(BestScore* scores) { 
    FILE* file = fopen(SCORE_FILE, "w");
    if (file == NULL) {
        printf("Error: Unable to open score file for writing.\n");
        return;
    }

    for (int i = 0; i < BEST_COUNT; i++) {
        fprintf(file, "%d %s\n", scores[i].score, scores[i].name); // save score and name
    }
    fclose(file);
}

// replacing scores if needed
void updateBestScores(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, BestScore* scores, int points, int red, int blue) {
    for (int i = 0; i < BEST_COUNT; i++) {
        if (points > scores[i].score) {
            // shifting lower scores down
            for (int j = BEST_COUNT - 1; j > i; j--) {
                scores[j] = scores[j - 1];
            }

            // collecting the name
            char playerName[MAX_NAME_LENGTH];
            collectPlayerName(scrtex, renderer, screen, charset, playerName, red, blue);

            // upd the current score and name
            scores[i].score = points;
            strncpy(scores[i].name, playerName, MAX_NAME_LENGTH - 1);
            scores[i].name[MAX_NAME_LENGTH - 1] = '\0'; // making sure for the null termination
            break;
        }
    }
}

// finally displaying top scores
void displayBestScores(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, BestScore* scores, int red, int blue, char text[], SDL_Surface* charset) {
    DrawRectangle(screen, 100, 100, SCREEN_WIDTH - 200, 2 * MENU_HEIGHT, red, blue);

    strcpy(text, "=== Best Scores ===");
    DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, 110, text, charset);

    for (int i = 0; i < BEST_COUNT; i++) {
        sprintf(text, "%d. %s - %d", i + 1, scores[i].name, scores[i].score);
        DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, 140 + i * 20, text, charset); // Position each entry
    }
    SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, scrtex, NULL, NULL);
    SDL_RenderPresent(renderer);
}

//------------------------------------------------
//----------------  RESTART ----------------------
//------------------------------------------------

// game restart logic
void restartGame(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Rect *body, int *dir, double *worldTime, SDL_Surface *screen, int *tpCount, int *length, double *delay, int *spdup, int *t3, int* randd, int *points, BestScore *scores, int red, int blue, char text[], SDL_Surface *charset) {
    // reset the head position
	if(*length > INIT_LENGTH){
		SDL_Rect *new_body = (SDL_Rect *)realloc(body, (INIT_LENGTH) * sizeof(SDL_Rect));
		if (new_body == NULL) {
			// handle realloc failure
			printf("Memory allocation failed for body array\n");
			return;
		}
		body = new_body;
		int *new_dir = (int *)realloc(dir, (INIT_LENGTH) * sizeof(int));
		if (new_dir == NULL) {
			// handle realloc failure
			printf("Memory allocation failed for direction array\n");
			return;
		}
		dir = new_dir;
		*length = INIT_LENGTH;
	}
	int p=0;
    for(int i=0; i<INIT_LENGTH; i++){
		body[i] = {SCREEN_WIDTH/2-p,SCREEN_HEIGHT/2,10,10};
		dir[i] = RIGHT;
		p+=10;
	}

    // reset time, score, and variables
    *worldTime = 0;
	*tpCount = 0;
	*delay = DELAY;
	*spdup = 1;
	*t3 = 0;
	*randd = 1;

	// scores logic
	loadBestScores(scores);
	updateBestScores(scrtex, renderer, screen, charset, scores, *points, red, blue);
	displayBestScores(scrtex, renderer, screen, scores, red, blue, text, charset);
	saveBestScores(scores);
	SDL_Delay(5000); // 5s wait time to show scores
	*points = 0;
	

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	
}

//------------------------------------------------
//----------------  MOVEMENT ---------------------
//------------------------------------------------

// setting directions if border is hit
void directions(SDL_Rect *body, int *dir, int *tpCount, TurningPoint *turningPoints, int* u_no, int* d_no, int* r_no, int* l_no){
	if (body[0].x < body[0].h/2-1) { // if snake head meets left border
		if (dir[0] == LEFT && body[0].y > MENU_HEIGHT+body[0].h/2+20+18) {
			dir[0] = UP;  // if it is not traveling on the first line
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
		}
		else if (dir[0] == LEFT && body[0].y < MENU_HEIGHT+body[0].h/2+20+18){ 
			dir[0]= DOWN;// if it is	
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
		}
		*l_no = 1;	
	} 
	else{
		*l_no = 0;
	} // if head meets right border
	if (body[0].x > (SCREEN_WIDTH - body[0].w - 4)) {
		if (dir[0] == RIGHT && body[0].y < SCREEN_HEIGHT-15){
			dir[0]= DOWN;   // change direction to left isf possible
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
		}
		else if (dir[0] == RIGHT && body[0].y >= SCREEN_HEIGHT-15){
			dir[0] = UP;		
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
		}
		*r_no = 1;
	}
	else{
		*r_no = 0;
	}
	// if head meets the upper border
	if (body[0].y < MENU_HEIGHT+body[0].h/2 +5+18) {
		if (dir[0]== UP && body[0].x > SCREEN_WIDTH-body[0].w - 6){
			dir[0] = LEFT;	
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
			*u_no = 1;
		}
		else if (dir[0] == UP && body[0].x < SCREEN_WIDTH-body[0].w/2){
			dir[0] = RIGHT;  
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, RIGHT};
		}
		*u_no = 1;
	} 
	else{
		*u_no = 0;
	} // if head meets the lower border
	if (body[0].y > SCREEN_HEIGHT-1.5*body[0].h) {
		if (dir[0] == DOWN && body[0].x > body[0].w/2){
			dir[0] = LEFT;
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
		}
		else if (dir[0]== DOWN && body[0].x <= 1.5*body[0].h+4){
			dir[0] = RIGHT;		
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, RIGHT};
		}
		*d_no = 1;
	}
	else{
		*d_no = 0;
	}
}

// moving snake based on its dir
void move(int *dir, TurningPoint *turningPoints, SDL_Rect *body, int *selfCollision, int *tpCount, int step, int length){
	for (int i = 0; i < length; i++) {
		// move the segment in its current direction
		switch (dir[i]) {
			case DOWN: body[i].y += (int)step; break;
			case UP: body[i].y -= (int)step; break;
			case RIGHT: body[i].x += (int)step; break;
			case LEFT: body[i].x -= (int)step; break;
		}
		// if self collision
		if (i != 0 && body[0].x == body[i].x && body[0].y == body[i].y) {
			*selfCollision = 1;
			break;
		}
		// check each turning point
		for (int j = 0; j < *tpCount; j++) {
			// if the segment reaches the turning point, its direction is changed accordingly
			if (abs(body[i].x - turningPoints[j].x) < TOLERANCE && abs(body[i].y - turningPoints[j].y) < TOLERANCE) {
				dir[i] = turningPoints[j].dir;
				// remove the turning point if it's no longer relevant
				if (i == length - 1) { // last segment passed the turning point
					for (int k = j; k < *tpCount - 1; k++) {
						turningPoints[k].dir = turningPoints[k + 1].dir;
						turningPoints[k].y = turningPoints[k + 1].y;
						turningPoints[k].x = turningPoints[k + 1].x;
					}
					(*tpCount)--;
				}
			}
		}
	}
}

// if self collision happens
void collision(SDL_Surface *screen, SDL_Surface *charset, SDL_Renderer *renderer, SDL_Texture *scrtex, int *selfCollision, int *quit, char text[], SDL_Rect *body, int* dir, double *worldTime, int *tpCount, int *length, int black, int red, int blue, double *delay, int *spdup, int *t3, int *randd, int *points, BestScore *scores){
	if (*selfCollision) {
		*selfCollision = 0;
		DrawRectangle(screen, SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 - 22, 220, 54, red, blue); // Adjust rectangle to center
		sprintf(text, "Snake hit itself!");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - 16, text, charset);
		sprintf(text, "1. Press ESC to exit;");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, charset);
		sprintf(text, "2. Press N for a new game;");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 16, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		int waitForInput = 1;
		while (waitForInput) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						waitForInput = 0; // exit game
						*quit = 1;
					} else if (event.key.keysym.sym == SDLK_n) {
						waitForInput = 0; // restart game
						restartGame(scrtex, renderer, body, dir, worldTime, screen, tpCount, length, delay, spdup, t3, randd, points, scores, red, blue, text, charset);
					}
				}
			}
		}
		SDL_FillRect(screen, NULL, black);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

}

// checking for overlapping of fruit and snake
bool isOverlapping(int fruitX, int fruitY, SDL_Rect segment) {
    // check if two rectangles (fruit and segment) overlap
    return !(fruitX + FRUIT_SIZE <= segment.x ||  // fruit is left of segment
             fruitX >= segment.x + SEGMENT_SIZE || // fruit is right of segment
             fruitY + FRUIT_SIZE <= segment.y ||  // fruit is above segment
             fruitY >= segment.y + SEGMENT_SIZE); // fruit is below segment
}

// initial snake position
void initSnake(SDL_Rect *body, int *dir, int *p){
	for(int i=0; i<INIT_LENGTH; i++){
		body[i] = {SCREEN_WIDTH/2-*p,SCREEN_HEIGHT/2,SEGMENT_SIZE,SEGMENT_SIZE};
		dir[i] = RIGHT;
		(*p)+=10;
	}	
}


//------------------------------------------------
//-------------------  LOOP ----------------------
//------------------------------------------------

// loop for collecting head turning point movements
void mainLoop(SDL_Texture* scrtex, SDL_Renderer* renderer, TurningPoint *turningPoints, SDL_Event event, int *lastMoveTime, int t2, int *quit, SDL_Rect *body, int* dir, double *worldTime, SDL_Surface *screen, int *tpCount, int *length, double *delay, int *spdup, int *t3, int* randd, int u_no, int d_no, int r_no, int l_no, int *points, BestScore *scores, int red, int blue, SDL_Surface *charset, char text[]){
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE) *quit = 1;
				if(event.key.keysym.sym == SDLK_n) restartGame(scrtex, renderer, body, dir, worldTime, screen, tpCount, length, delay, spdup, t3, randd, points, scores, red, blue, text, charset);
				// check if it's time for the next movement
				// adds a turning point coordinates when head makes the move
				if (t2 - *lastMoveTime > MOVE_DELAY) {
					*lastMoveTime = t2;
					if(event.key.keysym.sym == SDLK_UP && dir[0] != DOWN && u_no != 1){
						dir[0] = UP;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
					}
					if(event.key.keysym.sym == SDLK_DOWN && dir[0] != UP && d_no != 1) {
						dir[0] = DOWN;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
					}
					if(event.key.keysym.sym == SDLK_LEFT && dir[0] != RIGHT && l_no != 1) {
						dir[0] = LEFT;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
					}
					if(event.key.keysym.sym == SDLK_RIGHT && dir[0] != LEFT && r_no != 1) {
						dir[0] = RIGHT;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, RIGHT};
					}
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				*quit = 1;
				break;
		}
	}

}


//------------------------------------------------
//----------------  FRUIT & DOT ------------------
//------------------------------------------------


// generating random fruit coordinates
void generateFruitCoordinates(SDL_Rect* body, int length, int *x, int *y) {
    bool positionValid = false;

    while (!positionValid) {
        // generate random fruit coordinates within screen bounds
		*x = rand() % (SCREEN_WIDTH - FRUIT_MARGIN * 2) + FRUIT_MARGIN;
		*y = MENU_HEIGHT + rand() % (SCREEN_HEIGHT - MENU_HEIGHT - 24 - FRUIT_MARGIN);

        // check if the fruit overlaps with any segment of the snake
        positionValid = true;
        for (int i = 0; i < length; ++i) {
            if (isOverlapping(*x, *y, body[i])) {
                positionValid = false;
                break;
            }
        }
    }
}

// logic for head touching the fruit
void fruitTouch(SDL_Rect **body, SDL_Rect *Fruit, int fruitX, int fruitY, int *length, int **dir, int *points){
	if((*body)[0].x < Fruit->x+FRUIT_SIZE && (*body)[0].x+SEGMENT_SIZE > Fruit->x &&
		(*body)[0].y < Fruit->y+FRUIT_SIZE && (*body)[0].y+SEGMENT_SIZE > Fruit->y){
		fruitX = rand() % (SCREEN_WIDTH - FRUIT_MARGIN * 2) + FRUIT_MARGIN;
		(*points)+=1;
		fruitY = MENU_HEIGHT + rand() % (SCREEN_HEIGHT - MENU_HEIGHT - FRUIT_MARGIN);
		generateFruitCoordinates(*body, *length, &fruitX, &fruitY);
		Fruit->x = fruitX;
		Fruit->y = fruitY;
		*length = *length + 1;
		SDL_Rect *new_body = (SDL_Rect *)realloc(*body, (*length) * sizeof(SDL_Rect));
		if (new_body == NULL) {
			// handle realloc failure
			printf("Memory allocation failed for body array\n");
			exit(1);
		}
		*body = new_body;
		int *new_dir = (int *)realloc(*dir, (*length) * sizeof(int));
		if (new_dir == NULL) {
			// handle realloc failure
			printf("Memory allocation failed for direction array\n");
			exit(1);
		}
		*dir = new_dir;
		// giving a direction and coordinates to the newly added segment
		if((*dir)[*length-2] == UP){
			(*body)[*length-1].x = (*body)[*length-2].x;
			(*body)[*length-1].y = (*body)[*length-2].y+10;
		}
		if((*dir)[*length-2] == DOWN){
			(*body)[*length-1].x = (*body)[*length-2].x;
			(*body)[*length-1].y = (*body)[*length-2].y-10;
		}
		if((*dir)[*length-2] == RIGHT){
			(*body)[*length-1].x = (*body)[*length-2].x-10;
			(*body)[*length-1].y = (*body)[*length-2].y;
		}
		if((*dir)[*length-2] == LEFT){
			(*body)[*length-1].x = (*body)[*length-2].x+10;
			(*body)[*length-1].y = (*body)[*length-2].y;
		}
		(*body)[*length-1].h = SEGMENT_SIZE;
		(*body)[*length-1].w = SEGMENT_SIZE;
		(*dir)[*length-1] = (*dir)[*length-2];
	}					
}

// logic for head touching the red dot
void dotTouch(SDL_Rect **body, SDL_Rect *Dot, int dotX, int dotY, int fruitX, int fruitY, int *length, int **dir, double *delay, int *t3, int *randd, int worldTime, double *redTime, int *points){
	if((*body)[0].x < Dot->x+FRUIT_SIZE && (*body)[0].x+SEGMENT_SIZE > Dot->x &&
		(*body)[0].y < Dot->y+FRUIT_SIZE && (*body)[0].y+SEGMENT_SIZE > Dot->y){
		(*points)+=2; // 2pts for eating red dot
		do{
			generateFruitCoordinates(*body, *length, &dotX, &dotY);
		}while(dotX == fruitX && dotY == fruitY); // ensuring red dot and fruit dont have same coords

		double f = RED_SPEED;
		double a = (1+(f/100)); // slowdown factor
		(*delay)*=a;

		Dot->x = dotX;
		Dot->y = dotY;

		*randd = 1;
		*t3 = worldTime;
		*redTime = 0;
	}
}

// loading animation for the remaining time of red dot
void redDotLoad(SDL_Surface *screen, double redTime, int red, int blue, int white){
	for(int i=0; i<RED_INT;i++){
		if(redTime == 0){
			for(int j=0; j<RED_INT;j++){
				DrawRectangle(screen, 125+1+j*15, 60, 15, 15, red, blue);			
			}
		}
		else{
			if(i < redTime){
				for(int j=0; j<=i; j++){
					DrawRectangle(screen, 125+1+j*15, 60, 15, 15, red, white);			
				}
			}
			else{
				for(int j=i+1; j<RED_INT; j++){
					DrawRectangle(screen, 125+1+j*15, 60, 15, 15, red, blue);			
				}
			}
		}
	}
}

//------------------------------------------------
//----------------  GRAPHIC ----------------------
//------------------------------------------------

// loading snake bitmaps created by me
void loadBitmaps(SDL_Surface **headU, SDL_Surface **headD, SDL_Surface **headL, SDL_Surface **headR, SDL_Surface **bodyUD, SDL_Surface **bodyLR){
	*headU = SDL_LoadBMP("bmps/snake_h_u.bmp");
	if (headU == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
	*headD = SDL_LoadBMP("bmps/snake_h_d.bmp");
	if (headD == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
	*headR = SDL_LoadBMP("bmps/snake_h_r.bmp");
	if (headR == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
	*headL = SDL_LoadBMP("bmps/snake_h_l.bmp");
	if (headL == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
	*bodyLR = SDL_LoadBMP("bmps/snake_b_rl.bmp");
	if (bodyLR == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
	*bodyUD = SDL_LoadBMP("bmps/snake_b_ud.bmp");
	if (bodyUD == NULL){
		printf("Error: %s", SDL_GetError());
		exit(1);  // handle the error if loading fails
	}
}

// drawing my bitmaps on the snake
void drawBody(SDL_Rect* body, int length, int *dir, SDL_Surface *screen, SDL_Surface *headU, SDL_Surface *headD, SDL_Surface *headL, SDL_Surface *headR, SDL_Surface *bodyUD, SDL_Surface *bodyLR){
	for(int i=0; i<length; i++){
		if(i==0){
			if(dir[0] == UP) SDL_BlitSurface(headU, NULL, screen, &body[0]);
			if(dir[0] == DOWN) SDL_BlitSurface(headD, NULL, screen, &body[0]);
			if(dir[0] == LEFT) SDL_BlitSurface(headL, NULL, screen, &body[0]);
			if(dir[0] == RIGHT) SDL_BlitSurface(headR, NULL, screen, &body[0]);
		
		}
		else{
			if(dir[i] == UP || dir[i] == DOWN){
				SDL_BlitSurface(bodyUD, NULL, screen, &body[i]);
			}
			else{
				SDL_BlitSurface(bodyLR, NULL, screen, &body[i]);
			}
		}

	}
}

//------------------------------------------------
//----------------  MAIN FUNCTION ----------------
//------------------------------------------------

int main(){
    int t1, t2, t3, quit, rc;
	int fruitX, fruitY; // blue fruits coords
	int dotX, dotY; // red dots coords
	int randd=1; // flag for randomizing red dot spawn
	int redWait;
	int l_no = 0; // left turn blockade
	int r_no = 0; // right turn blockade
	int u_no = 0; // up turn blockade
	int d_no = 0; // down turn blockade
	double step;
	step = STEP_SIZE;
	double delay;
	delay = DELAY;
	int points = 0;
    SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *headU, *headD, *headL, *headR, *bodyUD, *bodyLR; // SNAKE BITMAPS
	SDL_Surface *snake;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	loadBitmaps(&headU, &headD, &headL, &headR, &bodyUD, &bodyLR);
	
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "SNAKE GAME 203726");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
								0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// turn off cursor
	SDL_ShowCursor(SDL_DISABLE);

	int black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int white = SDL_MapRGB(screen->format, 255, 255, 255);

	charset = SDL_LoadBMP("./cs8x8.bmp");

	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	double delta, worldTime, redTime; // redTime - time the redDot has been on the screen
    char text[128];
    quit = 0; // quit variable - if 1 - game ends
    worldTime = 0;
	int lastMoveTime = 0; // used for controlling the access to keys
	t1 = SDL_GetTicks();
	TurningPoint turningPoints[MAX_TURNING_POINTS]; // struct storing turningpoints coords
	int tpCount = 0; // counter of turning points
	SDL_Rect* body = (SDL_Rect*) malloc(INIT_LENGTH*sizeof(SDL_Rect));   // struct for body of the snake
	int* dir = (int*)malloc(INIT_LENGTH * sizeof(int));  // struct for each segments directions
	int p=0; // x-coordinate scaler for each body segment
	int length = INIT_LENGTH;
	int spdup = 1; // flag for snake speed up
	t3 = 0;
	
	generateFruitCoordinates(body, length, &fruitX, &fruitY);
	SDL_Rect Fruit = {fruitX, fruitY, FRUIT_SIZE, FRUIT_SIZE}; // struct for the blue fruit
	SDL_FillRect(screen, &Fruit, blue);
	do{
		generateFruitCoordinates(body, length, &dotX, &dotY); // condition so red dot coords are diff. than blue dot
	}while(dotX == fruitX && dotY == fruitY);
	SDL_Rect Dot = {dotX, dotY, FRUIT_SIZE, FRUIT_SIZE};  // struct for the red dot
	SDL_FillRect(screen, &Dot, black);
	redTime = 0;
	BestScore scores[BEST_COUNT]; 
	// initial snake position and directions
	initSnake(body, dir, &p);
	
	double f = SPEED_FACT; // factor by which snakes speed is changed for speedup
	double a = (1+(f/100)); // factor represented by value a >= 1
	
	while(!quit) {
        t2 = SDL_GetTicks();
        delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

        SDL_FillRect(screen, NULL, black);

        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, MENU_HEIGHT, red, blue); // menu window
		sprintf(text, "Snake game | Michal Binek 203726 | Time = %.1lf s", worldTime);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Requirements: 1-4; A-D + F");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		sprintf(text, "Esc - exit | n - new game");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
		
		DrawRectangle(screen, 4, MENU_HEIGHT+2, SCREEN_WIDTH - 8, 22, red, blue); // red dot and points window
		sprintf(text, "Red dot time: ");
		DrawString(screen, 10, 63, text, charset);
		sprintf(text, "Points: %d", points);
		DrawString(screen, SCREEN_WIDTH-200, 63, text, charset);
		DrawRectangle(screen, 125, MENU_HEIGHT+5, RED_INT*15+2, 17, red, blue);

		redDotLoad(screen, redTime, red, blue, white);
		
		mainLoop(scrtex, renderer, turningPoints, event, &lastMoveTime, t2, &quit, body, dir, &worldTime, screen, &tpCount, &length, &delay, &spdup, &t3, &randd, u_no, d_no, r_no, l_no, &points, scores, red, blue, charset, text);
        // moving the head
		int selfCollision = 0; // self collision flag
		move(dir, turningPoints, body, &selfCollision, &tpCount, step, length);
		// if self collision detected, the option menu is displayed
		collision(screen, charset, renderer, scrtex, &selfCollision, &quit, text, body, dir, &worldTime, &tpCount, &length, black, red, blue, &delay, &spdup, &t3, &randd, &points, scores);
		directions(body, dir, &tpCount, turningPoints, &u_no, &d_no, &r_no, &l_no);
		// snake touching the fruit conditions
		fruitTouch(&body, &Fruit, fruitX, fruitY, &length, &dir, &points);
		// deciding the random point in time when red dot appears
		if(randd == 1){
			redWait = rand() % RED_END + RED_START;
			randd = 0;
		}
		// if the random waiting time has pased generate red dot
		if(worldTime - t3 > redWait){
			redTime += delta; 
			SDL_FillRect(screen, &Dot, red);
        	dotTouch(&body, &Dot, dotX, dotY, fruitX, fruitY, &length, &dir, &delay, &t3, &randd, worldTime, &redTime, &points);
			
			// if the dot presence exceeds the allowed time condition
			if(redTime>RED_INT){
				redTime = 0;
				randd = 1;
				t3 = worldTime;
				SDL_FillRect(screen, &Dot, black);
				do{
					generateFruitCoordinates(body, length, &dotX, &dotY);
				}while(dotX == fruitX && dotY == fruitY);
				Dot.x = dotX;
				Dot.y = dotY;
			}
		}
		else{ 
			SDL_FillRect(screen, &Dot, black);
			redTime = 0;
		}
		// clear the window
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // draw body
		drawBody(body, length, dir, screen, headU, headD, headL, headR, bodyUD, bodyLR);

		SDL_FillRect(screen, &Fruit, blue);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		// speeding up the snake
		if(spdup == 1 && worldTime > SPEED_INT){
        	double f = SPEED_FACT;
			double a = (1-(f/100));
			delay *= a;
			SDL_Delay(delay);
			spdup = 0;
		}else{
			SDL_Delay(delay);
		}

	
    }
}
