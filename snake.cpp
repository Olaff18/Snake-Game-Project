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
#define INIT_LENGTH		   10
#define MAX_TURNING_POINTS 1000
#define MOVE_DELAY         100
#define TOLERANCE          5
#define STEP_SIZE          5
#define SPEED_INT          5
#define SPEED_FACT         90
#define MENU_HEIGHT        54
#define FRUIT_MARGIN       10  // minimum fruit margin from edges
#define FRUIT_SIZE         10
#define SEGMENT_SIZE       10

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
	int w;
	int h;
	int dir;
} Rect;

typedef struct {
    int x;
    int y;
    int dir;
} TurningPoint;


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


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt rodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
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


// main
#ifdef __cplusplus
extern "C"
#endif

void directions(SDL_Rect *body, int *dir, int *tpCount, TurningPoint *turningPoints){
	if (body[0].x < body[0].h/2-1) { // if snake head meets left border
		if (dir[0] == LEFT && body[0].y > MENU_HEIGHT+body[0].h/2+2) {
			dir[0] = UP;  // if it is not traveling on the first line
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
		}
		if (dir[0] == LEFT && body[0].y < MENU_HEIGHT+body[0].h/2+2){ 
			dir[0]= DOWN;// if it is	
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
		}	
	} 
	if (body[0].x > (SCREEN_WIDTH - body[0].w - 4)) {
		if (dir[0] == RIGHT && body[0].y < SCREEN_HEIGHT-1.5*body[0].h-2){
			dir[0]= DOWN;   // Change direction to left if possible
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
		}
		if (dir[0] == RIGHT && body[0].y > SCREEN_HEIGHT-1.5*body[0].h-2){
			dir[0] = UP;// if it is			
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
		}
	}

	if (body[0].y < MENU_HEIGHT+body[0].h/2) {
		// printf("%d", headRect.x);
		if (dir[0]== UP && body[0].x > SCREEN_WIDTH-body[0].w - 6){
			dir[0] = LEFT;	
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
		}
		if (dir[0] == UP && body[0].x < SCREEN_WIDTH-body[0].w/2){
			dir[0] = RIGHT;   // Change direction to down if possible
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, RIGHT};
		}
	} 
	if (body[0].y > SCREEN_HEIGHT-1.5*body[0].h+4) {
		if (dir[0] == DOWN && body[0].x > body[0].w/2){
			dir[0] = LEFT;
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
		}
		if (dir[0]== DOWN && body[0].x <= body[0].w/2){
			dir[0] = RIGHT;		
			turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, RIGHT};
		}
	}
}

void restartGame(SDL_Rect *body, int *dir, double *worldTime, SDL_Surface *screen, int *tpCount, int *length) {
    // Reset the head position
	if(*length > INIT_LENGTH){
		SDL_Rect *new_body = (SDL_Rect *)realloc(body, (INIT_LENGTH) * sizeof(SDL_Rect));
		if (new_body == NULL) {
			// Handle realloc failure
			printf("Memory allocation failed for body array\n");
			// Exit or return from function as appropriate
			return;
		}
		body = new_body;
		int *new_dir = (int *)realloc(dir, (INIT_LENGTH) * sizeof(int));
		if (new_dir == NULL) {
			// Handle realloc failure
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

    // Reset time, scores, or other variables
    *worldTime = 0;
	*tpCount = 0;

    // Clear screen (if needed)
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));

    // Log or print a message if debugging
}

void mainLoop(TurningPoint *turningPoints, SDL_Event event, int *lastMoveTime, int t2, int *quit, SDL_Rect *body, int* dir, double *worldTime, SDL_Surface *screen, int *tpCount, int *length){
		while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE) *quit = 1;
				if(event.key.keysym.sym == SDLK_n) restartGame(body, dir, worldTime, screen, tpCount, length);
				// check if it's time for the next movement
				// adds a turning point coordinates when head makes the move
				if (t2 - *lastMoveTime > MOVE_DELAY) {
					*lastMoveTime = t2;
					if(event.key.keysym.sym == SDLK_UP && dir[0] != DOWN){
						dir[0] = UP;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, UP};
					}
					if(event.key.keysym.sym == SDLK_DOWN && dir[0] != UP) {
						dir[0] = DOWN;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, DOWN};
					}
					if(event.key.keysym.sym == SDLK_LEFT && dir[0] != RIGHT) {
						dir[0] = LEFT;
						turningPoints[(*tpCount)++] = (TurningPoint){body[0].x, body[0].y, LEFT};
					}
					if(event.key.keysym.sym == SDLK_RIGHT && dir[0] != LEFT) {
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

void move(int *dir, TurningPoint *turningPoints, SDL_Rect *body, int *selfCollision, int *tpCount, int step, int length){
	for (int i = 0; i < length; i++) {
		// move the segment in its current direction
		switch (dir[i]) {
			case DOWN: body[i].y += (int)step; break;
			case UP: body[i].y -= (int)step; break;
			case RIGHT: body[i].x += (int)step; break;
			case LEFT: body[i].x -= (int)step; break;
		}
		// if(i != 0 && abs(body[0].x - body[i].x) < TOLERANCE && abs(body[0].y - body[i].y) < TOLERANCE){
		// 	selfCollision = 1;
		// 	break;
		// }
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
				if (i == length - 1) { // Last segment passed the turning point
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

void collision(SDL_Surface *screen, SDL_Surface *charset, SDL_Renderer *renderer, SDL_Texture *scrtex, int *selfCollision, int *quit, char text[], SDL_Rect *body, int* dir, double *worldTime, int *tpCount, int *length, int czarny, int czerwony, int niebieski){
	if (*selfCollision) {
		*selfCollision = 0;
		DrawRectangle(screen, SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 - 22, 220, 54, czerwony, niebieski); // Adjust rectangle to center
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
						waitForInput = 0; // Exit game
						*quit = 1;
					} else if (event.key.keysym.sym == SDLK_n) {
						waitForInput = 0; // Restart game
						restartGame(body, dir, worldTime, screen, tpCount, length);
					}
				}
			}
		}
		SDL_FillRect(screen, NULL, czarny);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

}

bool isOverlapping(int fruitX, int fruitY, SDL_Rect segment) {
    // Check if two rectangles (fruit and segment) overlap
    return !(fruitX + FRUIT_SIZE <= segment.x ||  // Fruit is left of segment
             fruitX >= segment.x + SEGMENT_SIZE || // Fruit is right of segment
             fruitY + FRUIT_SIZE <= segment.y ||  // Fruit is above segment
             fruitY >= segment.y + SEGMENT_SIZE); // Fruit is below segment
}

void generateFruitCoordinates(SDL_Rect* body, int length, int *fruitX, int *fruitY) {
    bool positionValid = false;

    while (!positionValid) {
        // Generate random fruit coordinates within screen bounds
		*fruitX = rand() % (SCREEN_WIDTH - FRUIT_MARGIN * 2) + FRUIT_MARGIN;
		*fruitY = MENU_HEIGHT + rand() % (SCREEN_HEIGHT - MENU_HEIGHT - FRUIT_MARGIN);

        // Check if the fruit overlaps with any segment of the snake
        positionValid = true;
        for (int i = 0; i < length; ++i) {
            if (isOverlapping(*fruitX, *fruitY, body[i])) {
                positionValid = false;
                break;
            }
        }
    }
}

void fruitTouch(SDL_Rect **body, SDL_Rect *Fruit, int fruitX, int fruitY, int *length, int **dir){
	if((*body)[0].x < Fruit->x+FRUIT_SIZE && (*body)[0].x+SEGMENT_SIZE > Fruit->x &&
		(*body)[0].y < Fruit->y+FRUIT_SIZE && (*body)[0].y+SEGMENT_SIZE > Fruit->y){
		fruitX = rand() % (SCREEN_WIDTH - FRUIT_MARGIN * 2) + FRUIT_MARGIN;
		fruitY = MENU_HEIGHT + rand() % (SCREEN_HEIGHT - MENU_HEIGHT - FRUIT_MARGIN);
		// while(1){
		// 	if((*body)[0].x < Fruit->x+10 && (*body)[0].x+10 > Fruit->x &&
		// 		(*body)[0].y < Fruit->y+10 && (*body)[0].y+10 > Fruit->y){
		// 			fruitX = rand() % (SCREEN_WIDTH - 10);
		// 			fruitY = 54 + rand() % (SCREEN_HEIGHT - 54 - 10 + 1);
		// 		}
		// 	else{
		// 		break;
		// 	}
		// }
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

int main(){
    int t1, t2, quit, frames, rc;
	int fruitX, fruitY;
	int dotX, dotY;
	int step = STEP_SIZE;

    SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *snake;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

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


	// wy³¹czenie widocznoci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int bialy = SDL_MapRGB(screen->format, 255, 255, 255);

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

	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
    char text[128];
    quit = 0;
	fpsTimer = 0;
    worldTime = 0;
	int lastMoveTime = 0;
	t1 = SDL_GetTicks();
    // SDL_Rect headRect {SCREEN_WIDTH/2,SCREEN_HEIGHT/2,10,10};
	int prevx, prevy = 0;
	TurningPoint turningPoints[MAX_TURNING_POINTS];
	int tpCount = 0; 
	SDL_Rect* body = (SDL_Rect*) malloc(INIT_LENGTH*sizeof(SDL_Rect));
	int* dir = (int*)malloc(INIT_LENGTH * sizeof(int));
	int p=0;
	int length = INIT_LENGTH;
	
	generateFruitCoordinates(body, length, &fruitX, &fruitY);
	SDL_Rect Fruit = {fruitX, fruitY, FRUIT_SIZE, FRUIT_SIZE}; 
	SDL_FillRect(screen, &Fruit, niebieski);
	
	for(int i=0; i<INIT_LENGTH; i++){
		body[i] = {SCREEN_WIDTH/2-p,SCREEN_HEIGHT/2,SEGMENT_SIZE,SEGMENT_SIZE};
		dir[i] = RIGHT;
		p+=10;
	}
	int speedup = 1;
	while(!quit) {
        t2 = SDL_GetTicks();
        delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

        SDL_FillRect(screen, NULL, czarny);


        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 54, czerwony, niebieski);
		sprintf(text, "Snake game | Michal Binek 203726 | Time = %.1lf s", worldTime);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

		sprintf(text, "Requirements: 1-4; A");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		//"Esc - exit, \030 - faster, \031 - slower"
		sprintf(text, "Esc - exit | n - new game");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
		
		mainLoop(turningPoints, event, &lastMoveTime, t2, &quit, body, dir, &worldTime, screen, &tpCount, &length);
        // moving the head
		int selfCollision = 0;
		move(dir, turningPoints, body, &selfCollision, &tpCount, step, length);
		// if self collision detected, the option menu is displayed
		collision(screen, charset, renderer, scrtex, &selfCollision, &quit, text, body, dir, &worldTime, &tpCount, &length, czarny, czerwony, niebieski);
		directions(body, dir, &tpCount, turningPoints);
		// zmienic na zewnetrzne pixele
		// snake touching the fruit conditions
		fruitTouch(&body, &Fruit, fruitX, fruitY, &length, &dir);
        // clear the window
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // draw body
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for(int i=0; i<length; i++){
			SDL_FillRect(screen, &body[i], bialy);
		}

		SDL_FillRect(screen, &Fruit, niebieski);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

        SDL_Delay(25);

	
    }
}
