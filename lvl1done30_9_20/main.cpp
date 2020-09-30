#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL.h>
#include <string>
/*
	All config ActiveWin32

	C/C++ > Gen > add incl Dir > SDL2 include folder
	Linker > Gen > Ad Lib > SDL2 lib x86 folder
	Linker > input > Ad dependencies > SDL2.lib SDL2main.lib
	.dll file from x86 to project folder w/ .cpp file
	Linker > System > Subsystem > /SUBSYSTEM:CONSOLE

	Repeat for additional SDL libraries i.e SDL_image

	any missing .dll files download and add to project folder (32bit)
	dont forget the .ttf font file goes in project folder too
*/

// window&render size
#define WIDTH 1000 // OG 720
#define HEIGHT 750

#define FONT_SIZE 32
#define BALL_SPEED 16
#define SPEED 9
#define SIZE 16// define size of ball
#define PI 3.14159265358979323846

SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font* font;
SDL_Color color;
bool running; // exits when not true (ie pressing esc or click X)
bool guideUp; // controls exiting guide
bool pickUp;
bool win, lose;
int frameCount, timerFPS, lastFrame, fps; // 

SDL_Rect l_paddle, r_paddle, ball, score_board; // drawing our rectangles

// copper wires
SDL_Rect t_copper, b_copper, hw_copper1, hw_copper2, hw_copper3, hw_copper4, playerRect, resiGoal; // top bottom halfwall
float velX, velY; // ball velocity
std::string score; // string for the score
int l_s, r_s; // individual paddle score
bool turn; // whose turn it is

float volt, amp1, amp2, amp3, res1, res2, res3;

// init images etc thatll be on screen
SDL_Surface *player = nullptr;
SDL_Texture *texplayer = nullptr; 
SDL_Rect xplayer = { 0, 0, 0, 0 };

SDL_Surface *anResistor = nullptr;
SDL_Texture *texanResistor = nullptr;
SDL_Rect xanResistor = { 0, 0, 0, 0 };

SDL_Surface *anResistor10 = nullptr;
SDL_Texture *texanResistor10 = nullptr;
SDL_Rect xanResistor10 = { 0, 0, 0, 0 };

SDL_Surface *anResistor20 = nullptr;
SDL_Texture *texanResistor20 = nullptr;
SDL_Rect xanResistor20 = { 0, 0, 0, 0 };

SDL_Surface *tenV = nullptr;
SDL_Texture *textenV = nullptr;
SDL_Rect xtenV = { 0, 0, 0, 0 };

SDL_Surface *oneAmp= nullptr;
SDL_Texture *texoneAmp = nullptr;
SDL_Rect xoneAmp = { 0, 0, 0, 0 };

SDL_Surface *tenmAmp = nullptr;
SDL_Texture *textenmAmp = nullptr;
SDL_Rect xtenmAmp = { 0, 0, 0, 0 };

SDL_Surface *zeroP5Amp = nullptr;
SDL_Texture *texzeroP5Amp = nullptr;
SDL_Rect xzeroP5Amp = { 0, 0, 0, 0 };

SDL_Surface *ammeter = nullptr;
SDL_Texture *texammeter = nullptr;
SDL_Rect xammeter = { 0, 0, 0, 0 };

SDL_Surface *victory = nullptr;
SDL_Texture *texvictory = nullptr;
SDL_Rect xvictory = { 0, 0, 0, 0 };

SDL_Surface *defeat = nullptr;
SDL_Texture *texdefeat = nullptr;
SDL_Rect xdefeat = { 0, 0, 0, 0 };

void serve() {
	l_paddle.y = r_paddle.y = (HEIGHT / 2) - (l_paddle.h / 2);// sets to middle when serving apparently
	if (turn) {// take turns serving
		ball.x = l_paddle.x + (l_paddle.w * 4);// moves paddle a bit when serving i think
		velX = BALL_SPEED / 2;// serves in + x direction
	}
	else {
		ball.x = r_paddle.x - (r_paddle.w * 4);
		velX = -BALL_SPEED / 2;// serves in negative x direction
	}
	velY = 0;//set velocity back to 0 so it only only changes once it hits paddle
	ball.y = HEIGHT / 2 - (SIZE / 2);// h put ball in middle screen, ??
	turn = !turn;// change turns after each serve
}

void write(std::string text, int x, int y) {// score board
	SDL_Surface *surface;// load in the fonts and push it to texture, whichll be renderers to screen
	SDL_Texture *texture;
	const char* t = text.c_str();// character variable to 

	if (font == NULL)
	{
		std::cout << "Font is fucking NULL fuck!" << std::endl;
	}

	// creating text and font
	surface = TTF_RenderText_Solid(font, t, color);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	score_board.w = surface->w;// scoreboard width
	score_board.h = surface->h;
	score_board.x = x - score_board.w;// scoreboard x coord
	score_board.y = y - score_board.h;
	SDL_FreeSurface(surface);// pass surface
	SDL_RenderCopy(renderer, texture, NULL, &score_board);
	SDL_DestroyTexture(texture);
}

void update() {
	// checks to see if 2 rectangles have an intersection
		// is similar function for a circle apparently
	if (SDL_HasIntersection(&ball, &r_paddle)) {// if ball touches rPaddle
		double rel = (r_paddle.y + (r_paddle.h / 2)) - (ball.y + (SIZE / 2));// (determines current paddle pos) - (ball position)
		double norm = rel / (r_paddle.h / 2);// norm = gets a number relative to where it hit the paddle from center
		double bounce = norm * (5 * PI / 12); // bounce angle (max 75degrees)
		velX = -BALL_SPEED * cos(bounce);// changes ball direction with bounce
		velY = BALL_SPEED * -sin(bounce);// same but in y axis
	}
	// same thing but for other paddle
	if (SDL_HasIntersection(&ball, &l_paddle)) {
		double rel = (l_paddle.y + (l_paddle.h / 2)) - (ball.y + (SIZE / 2));
		double norm = rel / (l_paddle.h / 2);
		double bounce = norm * (5 * PI / 12);
		velX = BALL_SPEED * cos(bounce);
		velY = BALL_SPEED * -sin(bounce);
	}
	if (ball.y > r_paddle.y + (r_paddle.h / 2)) r_paddle.y += SPEED; // opponent AI follow ball down
	if (ball.y < r_paddle.y + (r_paddle.h / 2)) r_paddle.y -= SPEED;// opponent AI follow ball up
	if (ball.x <= 0) { r_s++;serve(); }// increase score when ball hits wall
	if (ball.x + SIZE >= WIDTH) { l_s++;serve(); }// increase score when ball hits other wall
	if (ball.y <= 0 || ball.y + SIZE >= HEIGHT) velY = -velY;// if ball hits top or bottom screen bounce back in
	ball.x += velX;// gives the ball velocity
	ball.y += velY;
	score = std::to_string(l_s) + "   " + std::to_string(r_s);// print score to screen
	if (l_paddle.y < 0)l_paddle.y = 0;// stops paddle leaving the top of the screen
	if (l_paddle.y + l_paddle.h > HEIGHT)l_paddle.y = HEIGHT - l_paddle.h;// stops it leaving bottom of screen
	if (r_paddle.y < 0)r_paddle.y = 0;
	if (r_paddle.y + r_paddle.h > HEIGHT)r_paddle.y = HEIGHT - r_paddle.h;

	// if player and resistor interact, pick up resistor or drop it
	if ((SDL_HasIntersection(&xanResistor, &xplayer)) || (SDL_HasIntersection(&xanResistor10, &xplayer)) || (SDL_HasIntersection(&xanResistor20, &xplayer)) && (pickUp == true))
	{
		if ((SDL_HasIntersection(&xanResistor, &xplayer) && (pickUp == true)))
		{
			xanResistor.x = xplayer.x + (xplayer.w - 20);
			xanResistor.y = xplayer.y + (xplayer.h / 2) - (xanResistor.h / 2);
			SDL_Delay(5);
		}
		else if ((SDL_HasIntersection(&xanResistor10, &xplayer) && (pickUp == true)))
		{
			xanResistor10.x = xplayer.x + (xplayer.w - 20);
			xanResistor10.y = xplayer.y + (xplayer.h / 2) - (xanResistor10.h / 2);
			SDL_Delay(5);
		}
		else if ((SDL_HasIntersection(&xanResistor20, &xplayer) && (pickUp == true)))
		{
			xanResistor20.x = xplayer.x + (xplayer.w - 20);
			xanResistor20.y = xplayer.y + (xplayer.h / 2) - (xanResistor20.h / 2);
			SDL_Delay(5);
		}
	}

	// volt = 10; amp1 = 0.01; amp2 = 0.5; amp3 = 1; res1 = 10; res2 = 20; res3 = 1000;
	// if player drops resistor in resiGoal (correct position)
				// MAYBE an if statement to check which level it is, so it can update accordingly
	// level one 10V / 0.5A = 20R
	// level 1 lose condition
	if (SDL_HasIntersection(&xanResistor, &resiGoal))
	{
		if (pickUp == false)
		{
			xanResistor.x = resiGoal.x + (resiGoal.w / 2) - (xanResistor.w / 2);
			xanResistor.y = resiGoal.y + (resiGoal.h / 2) - (xanResistor.h / 2);
			SDL_Delay(5);

			// if lvl1
			lose = true;

		}
	}
	// Level one lose condition
	if (SDL_HasIntersection(&xanResistor10, &resiGoal))
	{
		if (pickUp == false)
		{
			xanResistor10.x = resiGoal.x + (resiGoal.w / 2) - (xanResistor10.w / 2);
			xanResistor10.y = resiGoal.y + (resiGoal.h / 2) - (xanResistor10.h / 2);
			SDL_Delay(5);
			//if lvl1
			lose = true;
		}
	}
	// LEVEL ONE WIN CONDITION
	if (SDL_HasIntersection(&xanResistor20, &resiGoal))
	{
		if (pickUp == false)
		{
			xanResistor20.x = resiGoal.x + (resiGoal.w / 2) - (xanResistor20.w / 2);
			xanResistor20.y = resiGoal.y + (resiGoal.h / 2) - (xanResistor20.h / 2);
			SDL_Delay(5);
			// if lvl1
			win = true;
		}
	}



}

void componentsLvlOne()
{
	win = false; lose = false;
	volt = 10; amp1 = 0.01; amp2 = 0.5; amp3 = 1; res1 = 10; res2 = 20; res3 = 1000;

	// load the images
	anResistor = IMG_Load("an1kRes.png");
	anResistor10 = IMG_Load("an10Res.png");
	anResistor20 = IMG_Load("an20Res.png");
	tenV = IMG_Load("10V.png");
	oneAmp = IMG_Load("1amp.png");
	tenmAmp = IMG_Load("10mamp.png");
	zeroP5Amp = IMG_Load("0p5amp.png");
	ammeter = IMG_Load("ammeter0p5A.png");
	victory = IMG_Load("victory.png");
	defeat = IMG_Load("defeat.png");

	if (!anResistor)
	{
		std::cout << "image didnt load fuck" << std::endl;
	}

	// create image textures
	texanResistor = SDL_CreateTextureFromSurface(renderer, anResistor);
	SDL_FreeSurface(anResistor);
	texanResistor10 = SDL_CreateTextureFromSurface(renderer, anResistor10);
	SDL_FreeSurface(anResistor10);
	texanResistor20 = SDL_CreateTextureFromSurface(renderer, anResistor20);
	SDL_FreeSurface(anResistor20);
	textenV = SDL_CreateTextureFromSurface(renderer, tenV);
	SDL_FreeSurface(tenV);
	texoneAmp = SDL_CreateTextureFromSurface(renderer,oneAmp);
	SDL_FreeSurface(oneAmp);
	textenmAmp = SDL_CreateTextureFromSurface(renderer, tenmAmp);
	SDL_FreeSurface(tenmAmp);
	texzeroP5Amp = SDL_CreateTextureFromSurface(renderer, zeroP5Amp);
	SDL_FreeSurface(zeroP5Amp);
	texammeter = SDL_CreateTextureFromSurface(renderer, ammeter);
	SDL_FreeSurface(ammeter);
	texvictory = SDL_CreateTextureFromSurface(renderer, victory);
	SDL_FreeSurface(victory);
	texdefeat = SDL_CreateTextureFromSurface(renderer, defeat);
	SDL_FreeSurface(defeat);

	if (!texanResistor) std::cout << "image didnt load fuck" << std::endl;

	// getting the images dimensions
	SDL_QueryTexture(texanResistor, NULL, NULL, &xanResistor.w, &xanResistor.h);
	SDL_QueryTexture(texanResistor10, NULL, NULL, &xanResistor10.w, &xanResistor10.h);
	SDL_QueryTexture(texanResistor20, NULL, NULL, &xanResistor20.w, &xanResistor20.h);
	SDL_QueryTexture(textenV, NULL, NULL, &xtenV.w, &xtenV.h);
	SDL_QueryTexture(texzeroP5Amp, NULL, NULL, &xzeroP5Amp.w, &xzeroP5Amp.h);
	SDL_QueryTexture(texammeter, NULL, NULL, &xammeter.w, &xammeter.h);
	SDL_QueryTexture(texvictory, NULL, NULL, &xvictory.w, &xvictory.h);
	SDL_QueryTexture(texvictory, NULL, NULL, &xdefeat.w, &xdefeat.h);

	// positioning the images
	xanResistor.x = (WIDTH - xanResistor.w) / 2;
	xanResistor.y = (HEIGHT - xanResistor.h) / 1.1;

	xanResistor10.x = (WIDTH - xanResistor10.w) / 2 * 1.5;
	xanResistor10.y = (HEIGHT - xanResistor10.h) / 1.1;

	xanResistor20.x = (WIDTH - xanResistor20.w) / 4;
	xanResistor20.y = (HEIGHT - xanResistor20.h) / 1.1;

	xtenV.x = 90 - (xtenV.w / 2);
	xtenV.y = (HEIGHT / 2) - (xtenV.h / 2);

	xzeroP5Amp.x = 800 - (xzeroP5Amp.w);
	xzeroP5Amp.y = (HEIGHT / 2) - (xzeroP5Amp.h / 2);

	xammeter.x = 900 - (xammeter.w);
	xammeter.y = (HEIGHT / 2) - (xammeter.h / 2);

	xvictory.x = (WIDTH / 2) - (xvictory.w / 2);
	xvictory.y = (HEIGHT / 2) - (xvictory.h / 2);

	xdefeat.x = (WIDTH / 2) - (xdefeat.w / 2);
	xdefeat.y = (HEIGHT / 2) - (xdefeat.h / 2);
}

void thePlayer()
{

	player = IMG_Load("player.png");
	if (!player)
	{
		std::cout << "image didnt load fuck" << std::endl;
	}
	texplayer = SDL_CreateTextureFromSurface(renderer, player);
	SDL_FreeSurface(player);
	if (!texplayer) std::cout << "image didnt load fuck" << std::endl;

	SDL_QueryTexture(texplayer, NULL, NULL, &xplayer.w, &xplayer.h);

	xplayer.x = (WIDTH - xplayer.w) / 2;
	xplayer.y = (HEIGHT - xplayer.h) / 2;

}

void levelOne()
{
		// building the circuit
	// setting top and bottom copper rectangles, then the sides
	t_copper.w = WIDTH * 0.8 + 40;// thickness of paddles
	// x-pos ;		height length
	t_copper.x = (WIDTH / 2) - (t_copper.w / 2); t_copper.h = 20;// paddle x start loc, paddle h 4th of the screen
	t_copper.y = 50 - (t_copper.h / 2); // 1st part adjusts height - wtf does 2nd part do
	// bottom = top but some adjusted settings
	b_copper = t_copper;// make the bottom copper the same
	b_copper.x = (WIDTH / 2) - (b_copper.w / 2);// move it to the side
	b_copper.y = 700 - (b_copper.h / 2); // 1st part adjusts height - wtf does 2nd part do

	// 1	2
	// 4	3
	hw_copper1.w = 20;// thickness of halfwair
	hw_copper1.h = HEIGHT / 3; // paddle h 4th of the screen
	hw_copper4 = hw_copper3 = hw_copper2 = hw_copper1;// make the right paddle the same

	hw_copper1.x = 80;
	hw_copper1.y = 60;

	hw_copper2.x = 900;
	hw_copper2.y = 60;

	hw_copper3.x = 900;
	hw_copper3.y = 690 - (hw_copper3.h);

	hw_copper4.x = 80;
	hw_copper4.y = 690 - (hw_copper4.h);

	// Correct resistor placement position
	resiGoal.w = resiGoal.h = 50;
	resiGoal.x = 900 - (resiGoal.w / 3);
	resiGoal.y = (HEIGHT / 2) - (resiGoal.h / 2); // half of 750 width

}

void input() {
	SDL_Event e;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);

	// check to see if key is pressed and react accordingly
	while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
	if (keystates[SDL_SCANCODE_ESCAPE]) running = false;
	if (keystates[SDL_SCANCODE_UP]) l_paddle.y -= SPEED; //guideUp = false;
	if (keystates[SDL_SCANCODE_DOWN]) l_paddle.y += SPEED; //guideUp = false;

	//	LETS TRY TO ADD LEFT RIGHT MOVEMENT
	if (keystates[SDL_SCANCODE_LEFT]) l_paddle.x -= SPEED;
	if (keystates[SDL_SCANCODE_RIGHT]) l_paddle.x += SPEED;
	if (keystates[SDL_SCANCODE_SPACE])
	{
		pickUp = !pickUp;
		guideUp = false;
	}

	const Uint8 *state = SDL_GetKeyboardState(NULL);// for some reason there is ALWAYS a key pressed...
	if (state != NULL) {
		printf("Any Key is pressed.\n");
	}

	// wsad for real player
	if (keystates[SDL_SCANCODE_W]) xplayer.y -= SPEED; //guideUp = false;
	if (keystates[SDL_SCANCODE_S]) xplayer.y += SPEED; //guideUp = false;

	if (keystates[SDL_SCANCODE_A]) xplayer.x -= SPEED; //guideUp = false;
	if (keystates[SDL_SCANCODE_D]) xplayer.x += SPEED; //guideUp = false;
}

// draw things to screen here
// things render in order, so background first, etc
void renderOne() {
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255); // sets background to black
	SDL_RenderClear(renderer); // clear the renderer

	frameCount++;
	timerFPS = SDL_GetTicks() - lastFrame; //  see the difference
	if (timerFPS < (1000 / 60)) {// 60fps
		SDL_Delay((1000 / 60) - timerFPS);// delays the screen for same time
	}

	// Victory screen
	if (win == true)
	{
		SDL_RenderCopy(renderer, texvictory, NULL, &xvictory);
	}
	else if (lose == true)
	{
		SDL_RenderCopy(renderer, texdefeat, NULL, &xdefeat);
	}
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); // white
	SDL_RenderFillRect(renderer, &l_paddle);// fills in the shapes with colour
	SDL_RenderFillRect(renderer, &r_paddle);
	SDL_RenderFillRect(renderer, &ball);
	//SDL_RenderFillRect(renderer, &xplayer);

	// 'rendering' my own images to screen.
	SDL_SetRenderDrawColor(renderer, 184, 115, 51, 255); // coppery color & transparent
	SDL_RenderFillRect(renderer, &t_copper);// fills in the shapes with colour
	SDL_RenderFillRect(renderer, &b_copper);
	SDL_RenderFillRect(renderer, &hw_copper1);
	SDL_RenderFillRect(renderer, &hw_copper2);
	SDL_RenderFillRect(renderer, &hw_copper3);
	SDL_RenderFillRect(renderer, &hw_copper4);
	SDL_RenderFillRect(renderer, &resiGoal);

	// 'drawing' my own images to screen
	//SDL_RenderCopy(renderer, texzeroP5Amp, NULL, &xzeroP5Amp);
	SDL_RenderCopy(renderer, texammeter, NULL, &xammeter);
	SDL_RenderCopy(renderer, texplayer, NULL, &xplayer); // updates player img constantly
	SDL_RenderCopy(renderer, texanResistor, NULL, &xanResistor);
	SDL_RenderCopy(renderer, texanResistor10, NULL, &xanResistor10);
	SDL_RenderCopy(renderer, texanResistor20, NULL, &xanResistor20);
	SDL_RenderCopy(renderer, textenV, NULL, &xtenV);

	write(score, WIDTH / 2 + FONT_SIZE, FONT_SIZE * 2); // writes score and moves  it L R U P

	SDL_RenderPresent(renderer);
}

void levelGuide()
{

	SDL_Surface *lvlGuide1 = nullptr;
	SDL_Texture *texlvlGuide1 = nullptr;
	lvlGuide1 = IMG_Load("lvl1Guide.png");

	if (!lvlGuide1)
	{
		std::cout << "image didnt load fuck" << std::endl;
	}
	texlvlGuide1 = SDL_CreateTextureFromSurface(renderer, lvlGuide1);
	SDL_FreeSurface(lvlGuide1);
	if (!texlvlGuide1) std::cout << "image didnt load fuck" << std::endl;

	SDL_Rect xlvlGuide1 = { 0, 0, 0, 0 };
	SDL_QueryTexture(texlvlGuide1, NULL, NULL, &xlvlGuide1.w, &xlvlGuide1.h);

	xlvlGuide1.x = (WIDTH - xlvlGuide1.w) / 2;
	xlvlGuide1.y = (HEIGHT - xlvlGuide1.h) / 2;

	//SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texlvlGuide1, NULL, &xlvlGuide1);
	SDL_RenderPresent(renderer);

	// display level guide to user, press 'spacebar' when ready to continue
	while (guideUp)
	{
		input();
	}
}

int main(int argc, char *argv[])
{
	// initialize everything, create window and renderer, initialize font, initialize imgs, call level guide
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;

	// creates window, can do - if (...... (W, H, SDL_WINDOW_FULLSCREEN, ..., ...)
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer())" << std::endl;
	if (TTF_Init() < 0) std::cout << "TTF_Init fucken failed" << std::endl;

	// change font file here (cant use peepo its copyrighted) ##################
	font = TTF_OpenFont("Peepo.ttf", FONT_SIZE);
	if (font == NULL) std::cout << "No fucking font" << std::endl;
	running = 1;
	static int lastTime = 0;
	guideUp = 1;

	// running my stuff first for level 1
	IMG_Init(IMG_INIT_JPG);
	levelGuide();
	thePlayer();
	componentsLvlOne();
	levelOne();

	// were initialize @ start with SDL_Rect
	color.r = color.g = color.b = 255; // all white
	l_s = r_s = 0;// sets scores to 0
	l_paddle.x = 32; l_paddle.h = HEIGHT / 4;// paddle x start loc, paddle h 4th of the screen
	l_paddle.y = (HEIGHT / 2) - (l_paddle.h / 2); //
	l_paddle.w = 12;// thickness of paddles
	r_paddle = l_paddle;// make the right paddle the same
	r_paddle.x = WIDTH - r_paddle.w - 32;// move it to the side
	ball.w = ball.h = SIZE;// set ball size

	serve();// seems to work the same without the serve function???

	// game loop
	while (running) {
		lastFrame = SDL_GetTicks();
		if (lastFrame >= (lastTime + 1000)) {
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
		}

		update();// game is mostly played here, logic, input, check for keyboard input
		input();
		renderOne();

	}
	// closes everything whence you exit
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;

}



