#include <iostream>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <unistd.h> // sleep function

#include "config.h"
#include "botbaseline.h"

using namespace std;

int board_game[HEIGHT][WIDTH];
Point win_path[5];

void init_board_game();
void go_to_xy(Point p);
void set_text_color(int color);
void draw_tile(Point p, int color);
int who_win();
void draw_win_path(int winner);
void play_game();
Point player1_run();
Point player2_run();

void draw_background();

int main(){
    srand (time(NULL));

    set_text_color(WHITE_COLOR);
    char c;
    do{
        cout<<"Are you ready?(y/n)"<<endl;
        cin>>c;
    }while(c != 'y');

    play_game();

    //_getch();
    return 0;
}

void init_board_game(){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            board_game[i][j] = 0;
            draw_tile(Point(BLOCK_RATIO*j, i), BLACK_COLOR);
        }
    }
}

void set_text_color(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void go_to_xy(Point p){
    COORD coord;
    coord.X = p.x;
    coord.Y = p.y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void draw_win_path(int winner){

    bool turn = true;
    int color;
    int blink_count = 10;
    while(blink_count >= 0){
        if(winner == 1){
            if(turn) color = WHITE_COLOR;
            else color = BLACK_COLOR;
        } else {
            if(turn) color = RED_COLOR;
            else color = BLACK_COLOR;
        }
        for(int i = 0; i < 5; i++){
            draw_tile(win_path[i], color);
        }
        Sleep(500);
        turn = !turn;
        blink_count--;
    }

}

void draw_tile(Point p, int color){
    go_to_xy(p);
    set_text_color(color);
    for(int i = 0; i < BLOCK_RATIO; i++) {
        if(color == WHITE_COLOR)
            cout<<"O";
        else cout<<"X";
    }
}

int who_win(){
    int win = 0; // 1. thang . 0 la chua thang
    for(int i=0; i < HEIGHT; i++){
        for(int j=0; j < WIDTH; j++){
            if(board_game[i][j] == 0) continue;
            // check huong 6h, code goi y
            if((board_game[i][j]==board_game[i+1][j])&&(board_game[i][j]==board_game[i+2][j])
               &&(board_game[i][j]==board_game[i+3][j])&&(board_game[i+4][j]==board_game[i][j])){
                if(board_game[i-1][j] == 0 || board_game[i+5][j] == 0)
                    win = 1;
                else win = 0;

                if(win == 1){
                    for(int k=0; k <= 4 ;k++){
                        win_path[k] = Point(BLOCK_RATIO*j, i+k);
                    }
                    return board_game[i][j];
                }
            }
            // check huong 3h
            // your code here

            // check huong 5h
           	// your code here

            // check huong 1h
            // your code here
        }
    }

    return 0;
}


Point player1_run(){
//    return player_rand(board_game, 1);
    return player_baseline(board_game, 1);
}

Point player2_run(){
//    return player_rand(board_game, -1);
    return player_baseline(board_game, -1);
}

void play_game(){
    bool turn_player1 = true;
    int turn_limit = 3000;
    int row, col, winner, repeat_pos;
    Point position;
    char c;
    do{
        init_board_game();
        turn_player1 = true;
        turn_limit = 3000;

        while(turn_limit > 0){
            repeat_pos = 5;
            if(turn_player1){
                do{
                    position = player1_run();
                    if(repeat_pos == 0){
                        cout<<"player 1 is so stupid"<<endl;
                        goto reset_game;
                    }
                    repeat_pos--;
                }while(board_game[position.x][position.y] != 0);

                board_game[position.x][position.y] = 1;
                draw_tile(Point(BLOCK_RATIO*position.y, position.x), WHITE_COLOR);
            } else {
                do{
                    position = player2_run();
                    if(repeat_pos == 0){
                        cout<<"player 2 is so stupid"<<endl;
                        goto reset_game;
                    }
                    repeat_pos--;
                }while(board_game[position.x][position.y] != 0);
                board_game[position.x][position.y] = -1;
                draw_tile(Point(BLOCK_RATIO*position.y, position.x), RED_COLOR);
            }

            winner = who_win();
            if(winner != 0){
                go_to_xy(Point(WIDTH/2, HEIGHT+5));
                cout<<winner<<" win"<<endl;
                draw_win_path(winner);
                break;
            }

            turn_player1 = !turn_player1;

            turn_limit--;
            Sleep(PAUSE_TIME);
        }
        reset_game:
        cin>>c;
    }while(c == 'y');
}
