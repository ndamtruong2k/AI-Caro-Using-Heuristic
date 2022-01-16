#ifndef BOTBASELINE
#define BOTBASELINE

#include <iostream>
#include <stdlib.h>
#include "config.h"

Point check_win(int board_game[][WIDTH], int player_id);
Point defend(int board_game[][WIDTH], int player_id);
Point attack(int board_game[][WIDTH], int player_id);
Point check_n_tile(int board_game[][WIDTH], int player_id, int n);

// player_id = 1 || -1
Point player_rand(int board_game[][WIDTH], int player_id){
    int row, col;
    row = rand() % HEIGHT;
    col = rand() % WIDTH;
    return Point(row, col);
}

Point player_baseline(int board_game[][WIDTH], int player_id){
    Point p = check_win(board_game, player_id);
    if(p.x != -1 && p.y != -1){
        return p;
    } else {
        p = defend(board_game, player_id);
        if(p.x != -1 && p.y != -1){
            return p;
        } else {
            return attack(board_game, player_id);
        }
    }
}

Point check_win(int board_game[][WIDTH], int player_id){
    return check_n_tile(board_game, player_id, 4);
}

Point defend(int board_game[][WIDTH], int player_id){
//    Pointp p = check_n_tile(board_game, -player_id, 4);
//    if(p.x != -1 || p.y != -1) return p;
//    else {
//        p = check_n_tile(board_game, -player_id, 3);
//        if(p.x != -1 || p.y != -1) return p;
//    }
//    return Point(-1, -1);
    return check_n_tile(board_game, -player_id, 4);
}

Point attack(int board_game[][WIDTH], int player_id){
    Point p = check_n_tile(board_game, player_id, 3);
    if(p.x != -1 && p.y != -1) return p;

    p = check_n_tile(board_game, -player_id, 3);
    if(p.x != -1 && p.y != -1) return p;

    p = check_n_tile(board_game, player_id, 2);
    if(p.x != -1 && p.y != -1) return p;

    p = check_n_tile(board_game, player_id, 1);
    if(p.x != -1 && p.y != -1) return p;

    for(int i=0; i < HEIGHT; i++){
        for(int j=0; j < WIDTH; j++){
            if(board_game[i][j] == -player_id){
                return Point(i+1, j);
            }
        }
    }

    return Point(HEIGHT/2, WIDTH/2);
}

Point check_n_tile(int board_game[][WIDTH], int player_id, int n){
    int check_6h = 1, check_3h = 1, check_5h = 1, check_1h = 1;
    Point temp(-1, -1);
    Point posible_moves[8];
    int p_moves = 0;
    for(int i=0; i < HEIGHT; i++){
        for(int j=0; j < WIDTH; j++){
            if(board_game[i][j] != player_id) continue;

            check_6h = 1, check_3h = 1, check_5h = 1, check_1h = 1;
            for(int k = 1; k < n; k++){
                if(board_game[i][j] == board_game[i+k][j]) check_6h++;
                if(board_game[i][j] == board_game[i][j+k]) check_3h++;
                if(board_game[i][j] == board_game[i+k][j+k]) check_5h++;
                if(board_game[i][j] == board_game[i-k][j+k]) check_1h++;
            }

            if(check_6h == n){
                if(n == 3){
                    if(board_game[i-1][j] == 0 && board_game[i+n][j] == 0) return Point(i-1, j);
                }
                if(board_game[i-1][j] == 0) {
                    posible_moves[p_moves] = Point(i-1, j);
                    p_moves++;
                }
                if(board_game[i+n][j] == 0) {
                    posible_moves[p_moves] = Point(i+n, j);
                    p_moves++;
                }

            }
            if(check_3h == n){
                if(n == 3){
                    if(board_game[i][j-1] == 0 && board_game[i][j+n] == 0) return Point(i, j-1);
                }

                if(board_game[i][j-1] == 0) {
                    posible_moves[p_moves] = Point(i, j-1);
                    p_moves++;
                }
                if(board_game[i][j+n] == 0) {
                    posible_moves[p_moves] = Point(i, j+n);
                    p_moves++;
                }

            }
            if(check_5h == n){
                if(n == 3){
                    if(board_game[i-1][j-1] == 0 && board_game[i+n][j+n] == 0) return Point(i-1, j-1);
                }

                if(board_game[i-1][j-1] == 0) {
                    posible_moves[p_moves] = Point(i-1, j-1);
                    p_moves++;
                }
                if(board_game[i+n][j+n] == 0) {
                    posible_moves[p_moves] = Point(i+n, j+n);
                    p_moves++;
                }

            }
            if(check_1h == n){
                if(n == 3){
                    if(board_game[i+1][j-1] == 0 && board_game[i-n][j+n] == 0) return Point(i+1, j-1);
                }
                if(board_game[i+1][j-1] == 0) {
                    posible_moves[p_moves] = Point(i+1, j-1);
                    p_moves++;
                }
                if(board_game[i-n][j+n] == 0) {
                    posible_moves[p_moves] = Point(i-n, j+n);
                    p_moves++;
                }

            }

            if(p_moves > 0){
                return posible_moves[rand()%p_moves];
            }
        }
    }
    return Point(-1, -1);
}

#endif // BOTBASELINE
