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
int frameCount, timerFPS, lastFrame, fps; // 

SDL_Rect l_paddle, r_paddle, ball, score_board; // drawing our rectangles

// copper wires
SDL_Rect t_copper, b_copper, hw_copper1, hw_copper2, hw_copper3, hw_copper4, playerRect, resiGoal; // top bottom halfwall
float velX, velY; // ball velocity
std::string score; // string for the score
int l_s, r_s; // individual paddle score
bool turn; // whose turn it is

// init images etc thatll be on screen
SDL_Surface *player = nullptr; // maybe wrong
SDL_Texture *texplayer = nullptr; // also this
SDL_Rect xplayer = { 0, 0, 0, 0 };

SDL_Surface *anResistor = nullptr;
SDL_Texture *texanResistor = nullptr;
SDL_Rect xanResistor = { 0, 0, 0, 0 };

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

	// if player and resistor interact
	if ((SDL_HasIntersection(&xanResistor, &xplayer) && (pickUp == true)))
	{
		//xanResistor.x = (WIDTH - xanResistor.w) / 2;
		//xanResistor.y = (HEIGHT - xanResistor.h) / 1.1;

		xanResistor.x = xplayer.x + (xplayer.w - 20);
		xanResistor.y = xplayer.y + (xplayer.h / 2) - (xanResistor.h / 2);
	}

	// if player drops resistor in resiGoal (correct position)
	if (SDL_HasIntersection(&xanResistor, &resiGoal))
	{
		if (pickUp == true)
		{
			xanResistor.x = resiGoal.x;
			xanResistor.y = resiGoal.y;
		}

		//resiGoal.x = 900 - (resiGoal.w / 2);
		//resiGoal.y = WIDTH / 2; // half of 750 width
	}



}

void anResista()
{

	//SDL_Surface *anResistor = nullptr;
	//SDL_Texture *texanResistor = nullptr;
	anResistor = IMG_Load("an1kRes.png");
	//player = IMG_Load("player.png");
	if (!anResistor)
	{
		std::cout << "image didnt load fuck" << std::endl;
	}
	texanResistor = SDL_CreateTextureFromSurface(renderer, anResistor);
	SDL_FreeSurface(anResistor);
	if (!texanResistor) std::cout << "image didnt load fuck" << std::endl;

	//SDL_Rect xanResistor = { 0, 0, 0, 0 };
	SDL_QueryTexture(texanResistor, NULL, NULL, &xanResistor.w, &xanResistor.h);

	xanResistor.x = (WIDTH - xanResistor.w) / 2;
	xanResistor.y = (HEIGHT - xanResistor.h) / 1.1;

	//SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texanResistor, NULL, &xanResistor);
	SDL_RenderPresent(renderer);
	//while (guideUp)
	//{
		//input();
	//}

	//SDL_Delay(1000); // 1/60th of a sec
	//SDL_DestroyTexture(texplayer);

}

void thePlayer()
{

	//SDL_Surface *player = nullptr;
	//SDL_Texture *texplayer = nullptr;
	//lvlGuide1 = IMG_Load("lvl1Guide.png");
	player = IMG_Load("player.png");
	if (!player)
	{
		std::cout << "image didnt load fuck" << std::endl;
	}
	texplayer = SDL_CreateTextureFromSurface(renderer, player);
	SDL_FreeSurface(player);
	if (!texplayer) std::cout << "image didnt load fuck" << std::endl;

	//SDL_Rect xplayer = { 0, 0, 0, 0 };
	SDL_QueryTexture(texplayer, NULL, NULL, &xplayer.w, &xplayer.h);

	xplayer.x = (WIDTH - xplayer.w) / 2;
	xplayer.y = (HEIGHT - xplayer.h) / 2;

	//playerRect.w = player->w;
	//playerRect.h = player->h;
	//playerRect.x = player->pixels;
	//playerRect.y = player.y;

	//SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texplayer, NULL, &xplayer);
	SDL_RenderPresent(renderer);
	//while (guideUp)
	//{
		//input();
	//}

	//SDL_Delay(1000); // 1/60th of a sec
	//SDL_DestroyTexture(texplayer);

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
	resiGoal.x = 900 - (resiGoal.w / 2);
	resiGoal.y = WIDTH / 2; // half of 750 width

}

void input() {
	SDL_Event e;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);

	// check to see if key is pressed and react accordingly
	while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
	if (keystates[SDL_SCANCODE_ESCAPE]) running = false;
	if (keystates[SDL_SCANCODE_UP]) l_paddle.y -= SPEED;
	if (keystates[SDL_SCANCODE_DOWN]) l_paddle.y += SPEED;

	//	LETS TRY TO ADD LEFT RIGHT MOVEMENT
	if (keystates[SDL_SCANCODE_LEFT]) l_paddle.x -= SPEED;
	if (keystates[SDL_SCANCODE_RIGHT]) l_paddle.x += SPEED;
	if (keystates[SDL_SCANCODE_SPACE])
	{
		guideUp = false; 
		if (pickUp == false)
		{
			pickUp = true;
		} 
		else if (pickUp == true) 
		{
			pickUp = false;
		}
		
	}

	// wsad for real player
	if (keystates[SDL_SCANCODE_W]) xplayer.y -= SPEED;
	if (keystates[SDL_SCANCODE_S]) xplayer.y += SPEED;

	if (keystates[SDL_SCANCODE_A]) xplayer.x -= SPEED;
	if (keystates[SDL_SCANCODE_D]) xplayer.x += SPEED;
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

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); // white
	SDL_RenderFillRect(renderer, &l_paddle);// fills in the shapes with colour
	SDL_RenderFillRect(renderer, &r_paddle);
	SDL_RenderFillRect(renderer, &ball);
	//SDL_RenderFillRect(renderer, &xplayer);

	// 'drawing' my own images to screen
	SDL_RenderCopy(renderer, texplayer, NULL, &xplayer); // updates player img constantly
	SDL_RenderCopy(renderer, texanResistor, NULL, &xanResistor);


	// 'rendering' my own images to screen.
	SDL_SetRenderDrawColor(renderer, 184, 115, 51, 255); // coppery color & transparent
	SDL_RenderFillRect(renderer, &t_copper);// fills in the shapes with colour
	SDL_RenderFillRect(renderer, &b_copper);
	SDL_RenderFillRect(renderer, &hw_copper1);
	SDL_RenderFillRect(renderer, &hw_copper2);
	SDL_RenderFillRect(renderer, &hw_copper3);
	SDL_RenderFillRect(renderer, &hw_copper4);

	write(score, WIDTH / 2 + FONT_SIZE, FONT_SIZE * 2); // writes score and moves  it L R U P

	SDL_RenderPresent(renderer);
}

void levelGuide()
{

	SDL_Surface *lvlGuide1 = nullptr;
	SDL_Texture *texlvlGuide1 = nullptr;
	lvlGuide1 = IMG_Load("lvl1Guide.png");
	//lvlGuide1 = IMG_Load("player.png");
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

	//SDL_Delay(1000); // 1/60th of a sec
	//SDL_DestroyTexture(texlvlGuide1);

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
	anResista();
	levelOne();

	/*
	IMG_Init(IMG_INIT_JPG);
	SDL_Surface * image = IMG_Load("lvl1Guide.JPG");
	for (int i = 0; i < 20060; i++)
	{

	}
	IMG_Quit();
	*/

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



