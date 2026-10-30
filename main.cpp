#include "raylib.h"
#include <cstdint>
#include <ctime>

//libraries

//macros and flags

//returns when my function is completed successfuly and does not return any other value
#define FUNC_SUCCESS    0

//Window Config
#define WIDTH           720
#define HEIGHT          480
#define TITLE           "Pong"

#define CENTER_X        (WIDTH/2)
#define CENTER_Y        (HEIGHT/2)

//Window in focus
#define WIN_MENU        1
#define WIN_GAME        0
#define WIN_CONTROLS    2
#define WIN_SETTINGS    3

//Config of Arena elements
#define MID_WIDTH       4
#define MID_LENGHT      16
#define MID_OFFSET      ((int)(MID_LENGHT/2))

//Player Config
#define PLAYER_WIDTH    10
#define PLAYER_LENGHT   84
#define PLAYER_COLOR    WHITE

//Other Configs
#define TARGET_FPS      60
#define PX_FROM_EDGE    20
#define MOVE_SPEED      ((int)400/TARGET_FPS)        // 400 FPS -> 1 MOVE_SPEED
#define COLOR_ARENA     GRAY
#define COLOR_SCORE     GREEN
#define SCORE_SIZE      60                          //score Font size
#define MAX_SCORE       10

//Ball Config
#define BALL_SPEED      ((double)(MOVE_SPEED-2) )
#define BALL_COLOR      ORANGE
#define BALL_SIZE       20              //can be used as side of a square or diameter in ball

//State of Playing Flags
#define STATE_RUN       0
#define STATE_1WON      1
#define STATE_2WON      2
#define STATE_PAUSE     3

//Button Config
#define BUTTON_FRAME_COLOR      {128,128,128,255} 
#define BUTTON_COLOR            {82,82,82,255}
#define BUTTON_WIDTH            (int)(WIDTH/3)
#define BUTTON_HEIGHT           (int)(HEIGHT/8)
#define BUTTON_FRAME            4
#define BUTTON_FONT_SIZE        40
#define BUTTON_TEXT_COLOR       WHITE
#define BUTTONS_START           (-20)
#define BUTTONS_OFFSET          70

#define CAPTIONS_COLOR          GREEN

//Player structure
struct Player{

    int pos_y;
    int pos_x;
    int num;        //Player 1 or 2
    int score;

};

//Ball Structure
struct Ball{

    int pos_x;
    int pos_y;
    double vel_x;   //velocity of ball on x axis
    double vel_y;   //velocity of ball on y axis

};

struct Button{

    char text[64];
    int pos_x;
    int pos_y;
    
};

struct TimerTask{

    int64_t milis;
    bool staged;
    bool busy;
};

TimerTask global_timer = {0, false, false};
TimerTask ball_timer = {0, false, false};

//UNIVERSAL FUNCTIONS:

//THROW() when expresion is false
int ASSERT(bool expr){

    if(!expr){ throw(1); }

    return 0;
}

//real delay used by global timer not recomened to used elsewhere
int delayMilis(int milis){

    int64_t milis_start = std::clock() / (CLOCKS_PER_SEC/1000);
    int64_t milis_now = std::clock() / (CLOCKS_PER_SEC/1000);

    while(milis_now - milis_start < milis){

        milis_now = std::clock() / (CLOCKS_PER_SEC/1000);
        __asm__("nop");
    }

    return FUNC_SUCCESS;
}

//Draw line down from start_x, start_y lenght of lenght color of color
int DrawLineDown(int start_x, int start_y, int lenght, Color color){

    for(int i = 0; i < lenght; i++){

        DrawPixel(start_x, start_y+i, color);

    }

    return FUNC_SUCCESS;
}

//Draws Filled Rectangle technicaly
int DrawLineWidth(int start_x, int start_y, int lenght, int width, Color color){

    for(int i = 0; i < width; i++){
        //DrawLine(start_x+i, start_y, start_x+i, start_y+lenght, color);
        DrawLineDown(start_x+i, start_y, lenght, color);
    }
    return FUNC_SUCCESS;
}

//GAME FUNCTIONS:

//Draws Arena for the Player
int DrawArena(){

    // line in the middle
    
    for(int i = 0; i < HEIGHT/MID_LENGHT; i+=2){

        DrawLineWidth((WIDTH-MID_WIDTH)/2, (MID_LENGHT*i)+MID_OFFSET, MID_LENGHT, MID_WIDTH, COLOR_ARENA);
    }
    
    //lines where players can move -> 5px from edges of screen and 5 px from line of player move space -> 10 px + width
    //left

    for(int i = 0; i < HEIGHT/(MID_LENGHT*2); i+=2){

        DrawLineWidth((PX_FROM_EDGE*2)+PLAYER_WIDTH, ((MID_LENGHT*2)*i)+MID_OFFSET*2, MID_LENGHT*2, MID_WIDTH, COLOR_ARENA);
    }

    //right

    for(int i = 0; i < HEIGHT/(MID_LENGHT*2); i+=2){

        DrawLineWidth(WIDTH-((PX_FROM_EDGE*2)+PLAYER_WIDTH), ((MID_LENGHT*2)*i)+MID_OFFSET*2, MID_LENGHT*2, MID_WIDTH, COLOR_ARENA);
    }

    return FUNC_SUCCESS;
}

//Draws player on screen
int DrawPlayer(Player* player){

    //MAX_Y = HEIGHT - PLAYER_LENGHT

    if(player->pos_y > HEIGHT - PLAYER_LENGHT){ 
        player->pos_y = HEIGHT - PLAYER_LENGHT;
    }
    if(player->pos_y < 0){ player->pos_y = 0; }

    DrawLineWidth(player->pos_x, player->pos_y, PLAYER_LENGHT, PLAYER_WIDTH, PLAYER_COLOR);
    

    return FUNC_SUCCESS;
}

//converts score from int to string
int convert_score(int score, char* ret){

    ret[0] = score + 48;
    ret[1] = '\0';
  
    return FUNC_SUCCESS;

}

//Draws score on screen
int DrawScores(Player* player1, Player* player2){

    char textScore[2] = {0,'\0'};
    convert_score(player1->score, textScore);

    DrawText(textScore, (WIDTH-MID_WIDTH-SCORE_SIZE/2)/2 - SCORE_SIZE , 10, SCORE_SIZE, COLOR_SCORE);

    convert_score(player2->score, textScore);

    DrawText(textScore, (WIDTH-MID_WIDTH-SCORE_SIZE/2)/2 + SCORE_SIZE, 10, SCORE_SIZE, COLOR_SCORE);

    return FUNC_SUCCESS;
}

//Handles input 
int HandleInput(Player* player1, Player* player2){

    if(IsKeyDown(KEY_W) ){
        
        player1->pos_y = player1->pos_y - MOVE_SPEED;

    }
    
    if(IsKeyDown(KEY_S)){

        player1->pos_y = player1->pos_y + MOVE_SPEED;

    }
    

    if(IsKeyDown(KEY_UP)){

        player2->pos_y = player2->pos_y - MOVE_SPEED;

    }
    if(IsKeyDown(KEY_DOWN)){

        player2->pos_y = player2->pos_y + MOVE_SPEED;

    }

    if(player2->pos_y < 0){ player2->pos_y = 0; }
    if(player1->pos_y < 0){ player1->pos_y = 0; }

    if(player2->pos_y > HEIGHT-PLAYER_LENGHT){ player2->pos_y = HEIGHT-PLAYER_LENGHT; }
    if(player1->pos_y > HEIGHT-PLAYER_LENGHT){ player1->pos_y = HEIGHT-PLAYER_LENGHT; }

    return FUNC_SUCCESS;
}

//check if ball hit player
bool HitPlayer(Ball* ball, Player* player1, Player* player2){

    //check if ball hitbox is in player hitbox
    //check player1

    if(ball->pos_x <= (PX_FROM_EDGE+PLAYER_WIDTH)){

        //BALL Y from ball->pos_y to ball->pos_y + BALL_SIZE
        //bro just check when it doesnt hit and inverse it bruh

        if( ball->pos_y <= player1->pos_y+PLAYER_LENGHT && ball->pos_y+BALL_SIZE >= player1->pos_y){
            return true;
        }
    }

    //check player2

    if(ball->pos_x+BALL_SIZE >= WIDTH-(PX_FROM_EDGE+PLAYER_WIDTH)){
        
        if( ball->pos_y <= player2->pos_y+PLAYER_LENGHT && ball->pos_y+BALL_SIZE >= player2->pos_y){
            return true;
        }

    }

    return false;
}

int ResetPlayers(Player* player1, Player* player2){

    player1->pos_y = (HEIGHT-PLAYER_LENGHT)/2;
    player1->pos_x = PX_FROM_EDGE;
    player1->score = 0;

    player2->score = 0;
    player2->pos_y = player1->pos_y;
    player2->pos_x = WIDTH-PX_FROM_EDGE-PLAYER_WIDTH;

    return FUNC_SUCCESS;
}

int ResetBall(Ball* ball){

    float random_x = 0.0f;
    float random_y = 0.0f;

    (std::clock() % 16) > 8? random_x = 1.0f : random_x = -1.0f;
    (std::clock() % 8 ) > 4? random_y = 0.5f : random_y = -0.5f;


    ball->pos_x = (WIDTH-BALL_SIZE)/2;
    ball->pos_y = (HEIGHT-BALL_SIZE)/2;
    ball->vel_x = random_x;
    ball->vel_y = random_y;

    return FUNC_SUCCESS;
}

int Handle_end(uint8_t state, uint8_t* window){

    DrawRectangle(0, 0, WIDTH, HEIGHT, {128,128,128,64});
    *window = WIN_MENU;


    switch(state){

        case STATE_1WON:
            DrawText("Player 1 won!", (WIDTH/2)-SCORE_SIZE*3-20, (HEIGHT/2)-SCORE_SIZE-50, SCORE_SIZE, CAPTIONS_COLOR);
            break;
        case STATE_2WON:
            DrawText("Player 2 won!", (WIDTH/2)-SCORE_SIZE*3-20, (HEIGHT/2)-SCORE_SIZE-50, SCORE_SIZE, CAPTIONS_COLOR);
            break;

        default:
            break;
    }

    global_timer.staged = true;
    global_timer.milis  = 2000;

    return FUNC_SUCCESS;
}


int Handle_win(uint8_t state, Player* player1, Player* player2, uint8_t* window){

    switch(state){

        case STATE_1WON:
            if(player1->score == MAX_SCORE-1){

                Handle_end(STATE_1WON, window);
                ResetPlayers(player1, player2);

            }else{

                player1->score++;

            }
            break;
        case STATE_2WON:
            if(player2->score == MAX_SCORE-1){

                Handle_end(STATE_2WON, window);
                ResetPlayers(player1, player2);

            }else{

                player2->score++;

            }
            break;

        default:
            break;
    }


    return FUNC_SUCCESS;
}


//Move and render ball 
int HandleBall(Ball* ball, Player* player1, Player* player2, uint8_t* window){

    //BALL_SPEED = BALL_SPEED * MOVE_SPEED
    //sike

    //calculate next position
    ball->pos_x = ball->pos_x + (ball->vel_x * BALL_SPEED);
    ball->pos_y = ball->pos_y + (ball->vel_y * BALL_SPEED); 

    //check boundaries X bound -> half of the player x , Y -> edges of screen -> flip y
    // Hit a player? -> flip x

    if(ball->pos_x <= (PX_FROM_EDGE+(PLAYER_WIDTH/2)) ){ //did not hit player behind him


        Handle_win(STATE_2WON, player1, player2, window);
        ResetBall(ball);


        ball_timer.staged = true;
        ball_timer.milis  = 300;

    }
    if(ball->pos_x+BALL_SIZE >= WIDTH-((PX_FROM_EDGE+PLAYER_WIDTH)/2) ){ //did not hit player behind him

        Handle_win(STATE_1WON, player1, player2, window);
        ResetBall(ball);


        ball_timer.staged = true;
        ball_timer.milis  = 300;
    }


    if(ball->pos_y <= 0 || ball->pos_y >= (HEIGHT - BALL_SIZE)){

        ball->vel_y = ball->vel_y * (-1.0f);

    }

    
    if(HitPlayer(ball, player1, player2)){

        ball->vel_x = ball->vel_x * (-1.0f);

    }
    
    //draw on screen
    DrawRectangle(ball->pos_x, ball->pos_y, BALL_SIZE, BALL_SIZE, BALL_COLOR);

    return FUNC_SUCCESS;
}

//MENU FUNCTIONS:

int DrawButton(Button* button, int offset_x = 40, int offset_y = 10){

    
    DrawRectangle(button->pos_x, button->pos_y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_FRAME_COLOR);
    DrawRectangle(button->pos_x+BUTTON_FRAME, button->pos_y+BUTTON_FRAME, BUTTON_WIDTH-(BUTTON_FRAME*2), BUTTON_HEIGHT-(BUTTON_FRAME*2), BUTTON_COLOR);

    DrawText(button->text, button->pos_x + offset_x, button->pos_y + offset_y, BUTTON_FONT_SIZE, BUTTON_TEXT_COLOR);

    return FUNC_SUCCESS;
}

int DrawCursor(uint8_t cursor, Button* play, Button* controls, Button* settings, Button* exit){

    Button button;

    switch(cursor){

        case 1:
            button.pos_x = play->pos_x;
            button.pos_y = play->pos_y;
            break;
        case 2:
            button.pos_x = controls->pos_x;
            button.pos_y = controls->pos_y;
            break;
        case 3:
            button.pos_x = settings->pos_x;
            button.pos_y = settings->pos_y;
            break;
        case 4:
            button.pos_x = exit->pos_x;
            button.pos_y = exit->pos_y;
            break;

        default: 
            break;
    }

    DrawText(">", button.pos_x - (PX_FROM_EDGE*2), button.pos_y-(BUTTON_HEIGHT/12) , 75, BUTTON_TEXT_COLOR);
    DrawText("<", button.pos_x + (PX_FROM_EDGE) + BUTTON_WIDTH, button.pos_y-(BUTTON_HEIGHT/12) , 75, BUTTON_TEXT_COLOR);
      

    return FUNC_SUCCESS;
}

//FIXED
//returns 0 if theres nothing to return or KEY_ENTER, buttons used for x and y bounds for mouse
int HandleInputMenu(uint8_t* cursor, Button* play, Button* controls, Button* settings, Button* exit){

    //scrool handle inmplement too

    float mouse_scrool = GetMouseWheelMove();

    if(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || mouse_scrool >= 1.0f){

        if(*cursor <= 1){

            *cursor = 1;

        }else{

            *cursor = *cursor - 1;
        }
    }
    if(IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN) || mouse_scrool <= -1.0f){

        if(*cursor >= 4){

            *cursor = 4;

        }else{
            *cursor = *cursor + 1;
        }
    }

    //keyboard handle up now mouse handle down

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

        Vector2 mouse_pos = GetMousePosition();

        //check bounds
        if(mouse_pos.x >= play->pos_x && mouse_pos.x <= play->pos_x+BUTTON_WIDTH){

            

            //check y   
            if(mouse_pos.y >= play->pos_y && mouse_pos.y <= play->pos_y+BUTTON_HEIGHT){

                *cursor = 1;
                return KEY_ENTER;


            }else if(mouse_pos.y >= controls->pos_y && mouse_pos.y <= controls->pos_y+BUTTON_HEIGHT){

                *cursor = 2;
                return KEY_ENTER;

            }else if(mouse_pos.y >= settings->pos_y && mouse_pos.y <= settings->pos_y+BUTTON_HEIGHT){

                *cursor = 3;
                return KEY_ENTER;

            }else if(mouse_pos.y >= exit->pos_y && mouse_pos.y <= exit->pos_y+BUTTON_HEIGHT){

                *cursor = 4;
                return KEY_ENTER;

            }else{}

        } // X POS IF

    }


    if(IsKeyPressed(KEY_ENTER)){

        return KEY_ENTER;

    }
    
    return FUNC_SUCCESS;

}


int HandleMenuWindow(uint8_t cursor, uint8_t* window, uint8_t* running){

    switch(cursor){

        case 1: //play
            //give control to the GAME_WIN
            *window = WIN_GAME;
            return FUNC_SUCCESS;
            break;

        case 2:
            *window = WIN_CONTROLS;
            return FUNC_SUCCESS;
            break;
        case 3:
            *window = WIN_SETTINGS;
            return FUNC_SUCCESS;
            break;
        case 4:
            *running = false;
            return FUNC_SUCCESS;
            break;

        default:
            break;

    }


    return FUNC_SUCCESS;
}

int HandleControls(uint8_t* window){

    Button exit_controls = {"Exit Menu \0", CENTER_X-(BUTTON_WIDTH/2), CENTER_Y - (BUTTON_HEIGHT) + BUTTONS_START+(BUTTONS_OFFSET*3) };

    DrawText("Controls:", WIDTH/3, HEIGHT/8, 50, CAPTIONS_COLOR);
    DrawText("Move Up:           W, Arrow Up\nMove Down:        S, Arrow Down\nEnd Application:  ESC",
                50, 150, BUTTON_FONT_SIZE, BUTTON_TEXT_COLOR
    );

    DrawButton(&exit_controls, 20, 10);

    //FROM DrawCursor()
    DrawText(">", exit_controls.pos_x - (PX_FROM_EDGE*2), exit_controls.pos_y-(BUTTON_HEIGHT/12) , 75, BUTTON_TEXT_COLOR);
    DrawText("<", exit_controls.pos_x + (PX_FROM_EDGE) + BUTTON_WIDTH, exit_controls.pos_y-(BUTTON_HEIGHT/12) , 75, BUTTON_TEXT_COLOR);

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

        Vector2 mouse_pos = GetMousePosition();

        if(mouse_pos.x >= exit_controls.pos_x && mouse_pos.x <= exit_controls.pos_x + BUTTON_WIDTH){
            if(mouse_pos.y >= exit_controls.pos_y && mouse_pos.y <= exit_controls.pos_y+BUTTON_HEIGHT){
                        
                    *window = WIN_MENU;
                    return FUNC_SUCCESS;
            }
        }

    }

    if(IsKeyPressed(KEY_ENTER)){
        *window = WIN_MENU;
        return FUNC_SUCCESS;

    }
    //END MENU


    return FUNC_SUCCESS;
}

//Yea no settings now
int HandleSettings(uint8_t* window){

    *window = WIN_MENU;

    return FUNC_SUCCESS;
}



int main(){

    //const int monitor = GetCurrentMonitor();
    ///const int mon_width = GetMonitorWidth(monitor);
    //const int mon_height = GetMonitorHeight(monitor);
    //const int mon_refrate = GetMonitorRefreshRate(monitor);
    //Init of variables

    Player player1 = {(HEIGHT-PLAYER_LENGHT)/2,PX_FROM_EDGE, 1,0};
    Player player2 = {player1.pos_y,WIDTH-PX_FROM_EDGE-PLAYER_WIDTH, 2,0};

    Ball ball = {(WIDTH-BALL_SIZE)/2, (HEIGHT-BALL_SIZE)/2, -1.0f, 0.0f};


    Button play_button      = {"  Play  \0", CENTER_X-(BUTTON_WIDTH/2), CENTER_Y - (BUTTON_HEIGHT) + BUTTONS_START };
    Button controls_button  = {"Controls\0", CENTER_X-(BUTTON_WIDTH/2), CENTER_Y - (BUTTON_HEIGHT) + BUTTONS_START+(BUTTONS_OFFSET*1) };
    Button settings_button  = {"Settings\0", CENTER_X-(BUTTON_WIDTH/2), CENTER_Y - (BUTTON_HEIGHT) + BUTTONS_START+(BUTTONS_OFFSET*2) };
    Button exit_button      = {"  Exit  \0", CENTER_X-(BUTTON_WIDTH/2), CENTER_Y - (BUTTON_HEIGHT) + BUTTONS_START+(BUTTONS_OFFSET*3) };


    uint8_t running = true;
    uint8_t window = WIN_MENU; // IN MENU ---------------------------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!!§§§

    //int64_t clock_start = 0;

    uint8_t cursor = 1;

    SetTargetFPS(TARGET_FPS);
    InitWindow(WIDTH, HEIGHT, TITLE);

    //clock_start = std::clock() / (CLOCKS_PER_SEC/1000);

    ResetBall(&ball); // init random behaviour :DD :3


    while(running){

        if(WindowShouldClose()){ running = false; break; }

        BeginDrawing();

        //i forgot this fr
        ClearBackground(BLACK);
        

        if(window == WIN_GAME){

            DrawArena();
                
            DrawPlayer(&player1);
            DrawPlayer(&player2);


            HandleBall(&ball, &player1, &player2, &window);


            DrawScores(&player1, &player2);

            //get input and hand it over to function 
            HandleInput(&player1, &player2);

        }
        else if(window == WIN_MENU){


            //Draw menu
            DrawText("Pong Game", WIDTH/3, HEIGHT/8, 50, CAPTIONS_COLOR);

            DrawButton(&play_button,       45, 10);
            DrawButton(&controls_button,   35, 10);
            DrawButton(&settings_button,   35, 10);
            DrawButton(&exit_button,       45, 10);

            DrawCursor(cursor, &play_button, &controls_button, &settings_button, &exit_button);        

            if(HandleInputMenu(&cursor, &play_button, &controls_button, &settings_button, &exit_button) == KEY_ENTER){

                HandleMenuWindow(cursor, &window, &running);

            }


        }
        else if(window == WIN_CONTROLS){

            HandleControls(&window);
            
        }
        else if(window == WIN_SETTINGS){

            HandleSettings(&window);

        }
        else{}
            
        


        //Handle End 
        EndDrawing();

        //Timer task

        if(global_timer.staged == true){

            global_timer.busy = true;
            delayMilis(global_timer.milis);

            global_timer.busy   = false;
            global_timer.staged = false;
            global_timer.milis  = 0;

        }

        if(ball_timer.staged == true){

            ball_timer.busy = true;
            delayMilis(ball_timer.milis);

            ball_timer.busy   = false;
            ball_timer.staged = false;
            ball_timer.milis  = 0;

        }

    }

    CloseWindow();

    return 0;
}