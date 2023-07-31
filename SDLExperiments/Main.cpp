#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>

const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 80;
const int BALL_SIZE = 10;
const int PADDLE_SPEED = 6;
const int BALL_SPEED_X = 3;
const int BALL_SPEED_Y = 2;
const int FRAME_RATE = 60;

int screenWidth = 640;
int screenHeight = 480;

// Function to handle paddle movement
void movePaddle(SDL_Rect& paddle, int direction) {
    paddle.y += direction * PADDLE_SPEED;

    // Prevent paddles from going out of the screen
    if (paddle.y < 0) {
        paddle.y = 0;
    }
    if (paddle.y > screenHeight - PADDLE_HEIGHT) {
        paddle.y = screenHeight - PADDLE_HEIGHT;
    }
}

// Function to render text using SDL_ttf
SDL_Texture* renderText(const std::string& text, SDL_Renderer* renderer, TTF_Font* font, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Pong Clone", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font!" << std::endl;
        return 1;
    }

    SDL_Rect leftPaddle = { 50, screenHeight / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_Rect rightPaddle = { screenWidth - 50 - PADDLE_WIDTH, screenHeight / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_Rect ball = { screenWidth / 2 - BALL_SIZE / 2, screenHeight / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE };

    int ballSpeedX = BALL_SPEED_X;
    int ballSpeedY = BALL_SPEED_Y;

    int leftScore = 0;
    int rightScore = 0;

    bool isRunning = true;
    bool ballMoving = false;
    Uint32 frameStart;

    while (isRunning) {
        frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    if (!ballMoving) {
                        ballMoving = true;
                        // Generate random initial velocity for the ball in both X and Y directions
                        int signX = rand() % 2 == 0 ? 1 : -1;
                        int signY = rand() % 2 == 0 ? 1 : -1;
                        ballSpeedX = signX * (rand() % 3 + 2); // Random speed between 2 and 4
                        ballSpeedY = signY * (rand() % 3 + 2); // Random speed between 2 and 4
                    }
                }
            }
            else if (event.type == SDL_WINDOWEVENT) {
                // Handle window resize
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    screenWidth = event.window.data1;
                    screenHeight = event.window.data2;
                    // Update the paddles and ball positions for the new window size
                    leftPaddle.y = screenHeight / 2 - PADDLE_HEIGHT / 2;
                    rightPaddle.y = screenHeight / 2 - PADDLE_HEIGHT / 2;
                    rightPaddle.x = screenWidth - 50 - PADDLE_WIDTH;
                    ball.x = screenWidth / 2 - BALL_SIZE / 2;
                    ball.y = screenHeight / 2 - BALL_SIZE / 2;
                }
            }
        }

        if (ballMoving) {
            // Handle paddle movement
            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_W]) {
                movePaddle(leftPaddle, -1);
            }
            if (state[SDL_SCANCODE_S]) {
                movePaddle(leftPaddle, 1);
            }
            if (state[SDL_SCANCODE_UP]) {
                movePaddle(rightPaddle, -1);
            }
            if (state[SDL_SCANCODE_DOWN]) {
                movePaddle(rightPaddle, 1);
            }

            // Ball movement
            ball.x += ballSpeedX;
            ball.y += ballSpeedY;

            // Ball collision with walls
            if (ball.y <= 0 || ball.y >= screenHeight - BALL_SIZE) {
                ballSpeedY = -ballSpeedY;
            }

            // Ball collision with paddles
            if (SDL_HasIntersection(&ball, &leftPaddle) || SDL_HasIntersection(&ball, &rightPaddle)) {
                ballSpeedX = -ballSpeedX;
            }

            // Ball out of bounds (score)
            if (ball.x <= 0) {
                ball.x = screenWidth / 2 - BALL_SIZE / 2;
                ball.y = screenHeight / 2 - BALL_SIZE / 2;
                ballSpeedX = BALL_SPEED_X;
                ballSpeedY = BALL_SPEED_Y;
                ballMoving = false;
                rightScore++; // Increase right player score
            }
            else if (ball.x >= screenWidth - BALL_SIZE) {
                ball.x = screenWidth / 2 - BALL_SIZE / 2;
                ball.y = screenHeight / 2 - BALL_SIZE / 2;
                ballSpeedX = -BALL_SPEED_X; // Reverse ball direction after score
                ballSpeedY = BALL_SPEED_Y;
                ballMoving = false;
                leftScore++; // Increase left player score
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw paddles and ball
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &leftPaddle);
        SDL_RenderFillRect(renderer, &rightPaddle);
        SDL_RenderFillRect(renderer, &ball);

        // Render scores as text
        SDL_Color textColor = { 255, 255, 255, 255 };
        std::string scoreText = "Score: " + std::to_string(leftScore) + " - " + std::to_string(rightScore);
        SDL_Texture* scoreTexture = renderText(scoreText, renderer, font, textColor);

        int textWidth, textHeight;
        SDL_QueryTexture(scoreTexture, nullptr, nullptr, &textWidth, &textHeight);

        SDL_Rect scoreRect = { (screenWidth - textWidth) / 2, 10, textWidth, textHeight };
        SDL_RenderCopy(renderer, scoreTexture, nullptr, &scoreRect);

        SDL_DestroyTexture(scoreTexture);

        // Render everything
        SDL_RenderPresent(renderer);

        // Frame rate limiting
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 1000 / FRAME_RATE) {
            SDL_Delay((1000 / FRAME_RATE) - frameTime);
        }
    }

    // Clean up and exit
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
