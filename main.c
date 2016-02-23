#include "mylib.h"
#include <stdio.h>
#include "manRed.h"
#include "manBlue.h"
#include <time.h>
#include <stdlib.h>

#define REG_DISPCNT *(unsigned short*) 0x4000000
#define MODE_3 3
#define BG2_EN (1 << 10)
#define RGB(r, g, b) (((b) << 10) | ((g) << 5)) | (r)

void drawGameBackground() {
	drawRect(0, 0, 240, 76, NICE_BLUE);
	drawRect(0, 76, 240, 4, NICE_BLACK);
	drawRect(0, 80, 240, 76, NICE_RED);
	drawRect(0, 156, 240, 4, NICE_BLACK);
}

struct man {
	char color;
	int x;
	int y;
	int jump;
};

struct randomRect {
	int height;
	int width;
	char color;
	int startCounter;
	int x;
	int y;
};

char scoreText[40];
char openingMessage[320];

void drawMan(struct man man) {
	drawImage3(man.x, man.y, MAN_WIDTH, MAN_HEIGHT, man.color == 'b' ? manBlue : manRed);
	if (man.jump == 1) {
		drawRect(man.x, man.y + MAN_HEIGHT, MAN_WIDTH, 1, man.color == 'r' ? NICE_RED : NICE_BLUE);
	}
	if (man.jump == -1) {
		drawRect(man.x, man.y - 1, MAN_WIDTH, 1, man.color == 'r' ? NICE_RED : NICE_BLUE);
	}
}

void drawFreeMan(struct man man, int buttonPressed) {
	drawImage3(man.x, man.y, MAN_WIDTH, MAN_HEIGHT, man.color == 'b' ? manBlue : manRed);
	if (buttonPressed == 2) {
		drawRect(man.x, man.y + MAN_HEIGHT, MAN_WIDTH, 1, NICE_RED);
	}
	if (buttonPressed == 4) {
		drawRect(man.x, man.y - 1, MAN_WIDTH, 1, NICE_RED);
	}
	if (buttonPressed == 1) {
		drawRect(man.x + MAN_WIDTH, man.y, 1, MAN_HEIGHT, NICE_RED);
	}
	if (buttonPressed == 3) {
		drawRect(man.x -1, man.y, 1, MAN_HEIGHT, NICE_RED);
	}
}

struct man updateJump(struct man man) {
    if (man.jump != 0 && man.y == (man.color == 'r' ? RED_Y : BLUE_Y) - MAN_HEIGHT) {
        man.jump = 0;
    }
    if (KEY_DOWN_NOW(BUTTON_B) && man.jump == 0 && man.color == 'r') {
        man.jump = 1;
    }
    if (KEY_DOWN_NOW(BUTTON_A) && man.jump == 0 && man.color == 'b') {
    	man.jump = 1;
    }
    if (man.jump == 1) {
        if (man.y > (man.color == 'r' ? RED_Y : BLUE_Y) - MAN_HEIGHT - JUMP_LIMIT) {
            man.y--;
        } else {
            man.jump = -1;
        }
    }
    if (man.jump == -1) {
        man.y++;
    }
    return man;
}

int throwObstacle(int index, int lim, int counter, struct randomRect obstacles[]) {
	if (index < lim && counter >= obstacles[index].startCounter) {
		if (obstacles[index].x >= 0) {
		    drawRect(obstacles[index].x, obstacles[index].y, 1, obstacles[index].height, NICE_BLACK);
		}
		if (obstacles[index].x <= 239 - obstacles[index].width) {
		    drawRect(obstacles[index].x + obstacles[index].width, obstacles[index].y, 1, obstacles[index].height, obstacles[index].color == 'r' ? NICE_RED : NICE_BLUE);
		}
		obstacles[index].x--;
		if (obstacles[index].x + obstacles[index].width < 0) {
			return 1;
		}
		return 0;
	}
	return 0;
}


struct randomRect createObstacle(int color, struct randomRect arr[], int i) {
	struct randomRect x = {rand() % 6 + 5, rand() % 11 + 5, color, i == 0 ? 0 : (AVG_DISTANCE + (rand() % 11) - 5) + arr[i-1].startCounter, 239, 0};
	return x;
}

int checkCollision(struct man man, int hittingIndex, struct randomRect arr[]) {
	if ((man.x + MAN_WIDTH >= arr[hittingIndex].x && man.x + MAN_WIDTH <= arr[hittingIndex].x + arr[hittingIndex].width)
        || (man.x >= arr[hittingIndex].x && man.x <= arr[hittingIndex].x + arr[hittingIndex].width) ) {

        if (man.y + MAN_HEIGHT >= arr[hittingIndex].y) {
            return 1;
        }
    }
    return 0;
}

void updateScore(int score) {
	sprintf(scoreText, "Score: %d", score);
	drawRect(210, 10, 30, 10, NICE_BLUE);
	drawString(10, 180, scoreText, NICE_BLACK);
}

struct randomRect drawStartScreen(struct man startMan, struct randomRect startObstacle) {
	sprintf(openingMessage, "MAKE THE JUMP!");
	drawRect(0, 0, 240, 156, NICE_RED);
	drawRect(0, 156, 240, 4, NICE_BLACK);
	drawRect(startObstacle.x, startObstacle.y, startObstacle.width, startObstacle.height, NICE_BLACK);
	drawString(10, 90, openingMessage, NICE_BLACK);
	while (startObstacle.x > 121) {
		startObstacle.x--;
		drawRect(startObstacle.x, startObstacle.y, 1, startObstacle.height, NICE_BLACK);
		drawRect(startObstacle.x + startObstacle.width, startObstacle.y, 1, startObstacle.height, NICE_RED);
		waitForVblank();	
		waitForVblank();	
	}
	drawMan(startMan);
	sprintf(openingMessage, "Use arrow keys to jump over obstacle");
	drawRect(90, 10, 90, 10, NICE_RED);
	drawString(10, 10, openingMessage, NICE_BLACK);
	sprintf(openingMessage, "and land!");
	drawString(30, 90, openingMessage, NICE_BLACK);
	return startObstacle;
}

void pause() {
	for (int i = 0; i < 10000; i++);
}

void playStartScreen(struct man startMan, struct randomRect startObstacle) {
	int crossedObstacle = 0;
	int buttonPressed = 0;
	struct randomRect arr[1];
	arr[0] = startObstacle;
	while (crossedObstacle == 0) {
		if (KEY_DOWN_NOW(BUTTON_UP)) {
			if (startMan.y > 50) {
				startMan.y--;
				buttonPressed = 2;
			}
		} else if (KEY_DOWN_NOW(BUTTON_DOWN)) {
			if (startMan.y < RED_Y - MAN_HEIGHT) {
				startMan.y++;
				buttonPressed = 4;
			}
		} else if (KEY_DOWN_NOW(BUTTON_LEFT)) {
			if (startMan.x > 0) {
				startMan.x--;
				buttonPressed = 1;
			}
		} else if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
			if (startMan.x + MAN_WIDTH < 240) {
				startMan.x++;
				buttonPressed = 3;
			}
		} 

		drawFreeMan(startMan, buttonPressed);

		if (checkCollision(startMan, 0, arr)) {
			drawRect(startMan.x, startMan.y, MAN_WIDTH, MAN_HEIGHT, NICE_RED);
			startMan.x = 60;
			startMan.y = RED_Y - MAN_HEIGHT;
		}

		if (startMan.x > startObstacle.x + startObstacle.width && startMan.y == RED_Y - MAN_HEIGHT) {
			crossedObstacle = 1;
		}
		waitForVblank();
	}
	drawRect(0, 0, 240, 50, NICE_RED);
	sprintf(openingMessage, "WELL DONE!");
	drawString(10, 90, openingMessage, NICE_BLACK);
	sprintf(openingMessage, "Now Press start to play game");
	drawString(30, 30, openingMessage, NICE_BLACK);
}

void showEndScreen() {
	drawRect(0, 0, 240, 160, NICE_YELLOW);
	drawHollowRect(92, 70, 65, 15, NICE_BLACK);
	drawString(75, 95, scoreText, NICE_BLACK);
	sprintf(openingMessage, "GAME OVER!");
	drawString(50, 95, openingMessage, NICE_BLACK);
	sprintf(openingMessage, "Press SELECT to go back");
	drawString(90, 65, openingMessage, NICE_BLACK);
}

int main() {
	REG_DISPCNT = MODE_3 | BG2_EN;

	//start screen callings
	struct man startMan = {'r', 60, RED_Y - MAN_HEIGHT, 0};
	struct randomRect startObstacle = {15, 10, 'n', -1, 230, RED_Y - 15};
	startObstacle = drawStartScreen(startMan, startObstacle);
	playStartScreen(startMan, startObstacle);

	//show Start message

	//main game initializations
	struct randomRect red[40], blue[40];
	int hasBegun = 0;
    int redCollided = 0;
    int blueCollided = 0;
    int flag = 1;

	//main game loop
	while(1) {
		if (KEY_DOWN_NOW(BUTTON_START) && flag == 1) {
			drawGameBackground();
			hasBegun = 1;
			updateScore(0);
		}
		if (hasBegun == 1) {
			struct man redMan = {'r', 15, RED_Y - MAN_HEIGHT, 0};
			struct man blueMan = {'b', 15, BLUE_Y - MAN_HEIGHT, 0};
			for (int i = 1; i < 10; i++) {
				struct randomRect x = createObstacle('b', blue, i);
				struct randomRect y = createObstacle('r', red, i);
				blue[i] = x;
				red[i] = y;
				blue[i].y = BLUE_Y - blue[i].height;
				red[i].y = RED_Y - red[i].height;
			}
			drawMan(redMan);
			drawMan(blueMan);


            int counter = 0;
            int lim = 10;
            int one = 0;
            int two = 1;
            int three = 2;
            int a = 0;
            int b = 1;
            int c = 2;
            int redHittingIndex = a;
            int blueHittingIndex = one;
            int score = 0;
            while ((a < lim || b < lim || c < lim || one < lim || two < lim || three < lim) && hasBegun == 1) {
                //Updating jump and redMan.y
                redMan = updateJump(redMan);
                blueMan = updateJump(blueMan);

                //throwing obstacles
                int blueObstacleDone = throwObstacle(one, lim, counter, blue);
                if (blueObstacleDone == 1) {
                	one += 3;
                	blueHittingIndex = two;
                	updateScore((score++) - 1);
                }
                int redObstacleDone = throwObstacle(a, lim, counter, red);
                if (redObstacleDone == 1) {                
                    a += 3;
                    redHittingIndex = b;
                    updateScore((score++) - 1);
                }


                blueObstacleDone = throwObstacle(two, lim, counter, blue);
                if (blueObstacleDone == 1) {
                	two += 3;
                	blueHittingIndex = three;
                	updateScore((score++) - 1);
                }
                redObstacleDone = throwObstacle(b, lim, counter, red);
                if (redObstacleDone == 1) {                
                    b += 3;
                    redHittingIndex = c;
                    updateScore((score++) - 1);
                }


                blueObstacleDone = throwObstacle(three, lim, counter, blue);
                if (blueObstacleDone == 1) {
                	three += 3;
                	blueHittingIndex = one;
                	updateScore((score++) - 1);
                }
                redObstacleDone = throwObstacle(c, lim, counter, red);
                if (redObstacleDone == 1) {                
                    c += 3;
                    redHittingIndex = a;
                    updateScore((score++) - 1);
                }

                //drawing Man if he's jumping
                if (blueMan.jump != 0) {
                	drawMan(blueMan);
                }
                if (redMan.jump != 0) {
                    drawMan(redMan);
                }


                waitForVblank();
                counter++;

                //check for collision
                redCollided = checkCollision(redMan, redHittingIndex, red);
                blueCollided = checkCollision(blueMan, blueHittingIndex, blue);
                if (redCollided == 1 || blueCollided == 1) {
                	hasBegun = 0;
                }

            	//check for exit
            	if (KEY_DOWN_NOW(BUTTON_SELECT)) {
            		hasBegun = 0;
					drawStartScreen(startMan, startObstacle);
					playStartScreen(startMan, startObstacle);
            	}
            }
            hasBegun = 0;
            redCollided = 1;
		}
		if (redCollided == 1 || blueCollided == 1) {
			pause();
			showEndScreen();
			redCollided = 0;
			blueCollided = 0;
			flag = 0;
		}

        if (KEY_DOWN_NOW(BUTTON_SELECT)) {
    		hasBegun = 0;
    		flag = 1;
			drawStartScreen(startMan, startObstacle);
			playStartScreen(startMan, startObstacle);
    	}
	}
	return 0;
}


