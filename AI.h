#ifndef AI
#define AI

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <set>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include "botbaseline.h"
#include "config.h"

#define MIN_BOARD_LIMIT 3
#define MAX_BOARD_LIMIT 50

#define ALPHA_INF -1032768
#define BETA_INF 1032768

#define SCORE_WIN 418192
#define SCORE_LOSE -418192

#define SCORE_OVER -16
#define SCORE_OPP_OVER 16

#define SCORE_DEADEND -128
#define SCORE_OPP_DEADEND 128

#define SCORE_M 2048
#define SCORE_OPP_M -2048

#define SCORE_M_MINUS 256
#define SCORE_OPP_M_MINUS -256

#define SCORE_STRAIGHT_M 4096
#define SCORE_OPP_STRAIGHT_M -4096

Point q;
struct GameState {
	bool game_end;
	int hscore;
	unsigned int n;
	unsigned int tiles_left;
	unsigned int last_row;
	unsigned int last_column;
	std::vector<bool> column_count;
	std::vector<char> board;

	GameState(unsigned int size=0): game_end(false), n(size),
		tiles_left(size*size), last_row(0), last_column(0)
		{column_count.assign(size, false); board.assign(size*size, '.');};

	char at(unsigned int row, unsigned int column) {
		unsigned pos = (row*n)+column;
		return board[pos];
	}

	void set(unsigned int row, unsigned int column, char player) {
		unsigned pos = (row*n)+column;
		board[pos] = player;
		column_count[column] = true;
		last_column = column;
		last_row = row;
		tiles_left--;
	}
};
GameState heuristics_func(GameState node, const int m, const char cur_player) {
	int board_size = node.column_count.size();
	node.hscore = 0;
	//lưu tọa độ những vị trí liền nhau theo hàng ngang//
	//lưu tọa độ những vị trí liền nhau theo hành chéo//
	std::set< std::pair<int, int> > checked_coords_columns;
	std::set< std::pair<int, int> > checked_coords_diag_botR;
	std::set< std::pair<int, int> > checked_coords_diag_topR;

	//check các ô liền giống nhau, với i là cột
	for (int i = 0 ; i < WIDTH; i++) {
		if (node.column_count[i]) {
			//lưu tọa độ những vị trí liền nhau theo hành dọc//
			std::set< std::pair<int, int> > checked_coords;
			//checks từng hàng trong một cột với j là hàng
			//bắt đầu bằng kiểm tra thảnh phần từ trên xuống dưới
			for (int j = 0;  j < HEIGHT; j++) {
				//kiểm tra ô giống nhau theo hàng dọc
				int cur_row = j;
				char cur_piece = node.at(cur_row, i);
				if (cur_piece != '.') {
					std::pair<int, int> cur_pos;
					cur_pos.first = cur_row;
					cur_pos.second = i;
					//kiểm tra xem vị trí hiện tại đã có trong hàm chưa//
					if (checked_coords.find(cur_pos) == checked_coords.end()) {
						//if this piece is the player's
						if (cur_piece == cur_player) {
							unsigned int cur_pattern_size = 1;
							//start_pattern và end_pattern theo dấu(tracks) hàng hiện tại
							int start_pattern = cur_pos.first;
							int end_pattern = cur_pos.first;
							checked_coords.insert(cur_pos);
							if (cur_row+1 < HEIGHT) {
								cur_row++;
								cur_piece = node.at(cur_row, i);
								//lặp tới khi ô tiếp theo vẫn của player, cộng thêm vào biến đếm...
								//độ dài các ô liền nhau
								while (cur_piece == cur_player) {
									cur_pos.first = cur_row;
									cur_pos.second = i;
									end_pattern = cur_pos.first;
									checked_coords.insert(cur_pos);
									cur_pattern_size++;
									if (cur_row+1 < HEIGHT) {
										cur_row++;
										cur_piece = node.at(cur_row, i);
									}
									else
										break;
								}
							}
							//nếu số ô liền nhau đúng bằng số ô chiến thắng...
							//đổi trạng thái chiến thắng -> return
							if (cur_pattern_size == m) {
								node.hscore = SCORE_WIN;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OVER;
							//nếu player cần phải đi 1 nước nữa mới đạt được trạng thái thắng
							//empty_count = 1 -> bị chặn 1 đầu
							//empty_count = 2 -> không bị chặn
							else if (cur_pattern_size == (m-1)) {
								//kiếm tra trên và dưới của dãy liền nhau
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(start_pattern-1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < HEIGHT) {
									cur_piece = node.at(end_pattern+1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								//không bị chặn được đánh giá là trạng thái tốt hơn bị chặn 1 đầu
								if (empty_count == 1)
									node.hscore += SCORE_M;
								else if (empty_count == 2){
									node.hscore += SCORE_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_DEADEND;
							}
							//nếu chiều dài ô liền nhau = (m-2), với m là số ô liền nhau cần để thắng

							else if (cur_pattern_size == (m-2)) {
								//kiểm tra phía trên và bên dưới dãy liền nhau
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(start_pattern-1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < HEIGHT) {
									cur_piece = node.at(end_pattern+1, i);
									if (cur_piece == '.') {
										empty_count++;
									}
								}
								//đánh giá trạng thái nguy hiểm
								//empty_count = 1 -> bị chặn 1 đầu
							    //empty_count = 2 -> không bị chặn
								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									//nếu không bị chặn 2 đầu...
									//kiểm tra có bị chặn cách 1 ô ở cả 2 đầu hay không
									if ((start_pattern-2) >=0) {
										cur_piece = node.at(start_pattern-2, i);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if ((end_pattern+2) < HEIGHT) {
										cur_piece = node.at(end_pattern+2, i);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}
									//nếu có -> trạng thái deadend
									if (M_tiles)
										node.hscore += SCORE_M_MINUS;
									else
										node.hscore += SCORE_DEADEND;
								}
								else
									node.hscore += SCORE_DEADEND;
							}
							//nếu không rơi vào các trường hợp trên -> cộng điểm theo số ô liền nhau
							else
								node.hscore += cur_pattern_size;
						}
						//nếu đây là ô của đối thủ
						else {
							char opp_player = cur_piece;
							unsigned int cur_pattern_size = 1;
							int start_pattern = cur_pos.first;
							int end_pattern = cur_pos.first;
							checked_coords.insert(cur_pos);
							if (cur_row+1 < HEIGHT) {
								cur_row++;
								cur_piece = node.at(cur_row, i);
								//nếu bên dưới ô hiện tại cũng là ô của đối thủ
								//cộng thêm vào biến đếm số ô liền nhau
								while (cur_piece == opp_player) {
									cur_pos.first = cur_row;
									cur_pos.second = i;
									end_pattern = cur_pos.first;
									checked_coords.insert(cur_pos);
									cur_pattern_size++;
									if (cur_row+1 < HEIGHT) {
										cur_row++;
										cur_piece = node.at(cur_row, i);
									}
									else
										break;
								}
							}
							//nếu số ô liền nhau của đối thủ đúng bằng số ô chiến thắng...
							//đổi trạng thái thua cuộc -> return
							if (cur_pattern_size == m) {
								node.hscore = SCORE_LOSE;
								node.game_end = true;
								return node;
							}
							//nếu số ô liền nhau của đối thủ lớn hơn cần thiết -> cộng điểm cho player
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OPP_OVER;
							//nếu đối thủ cần phải đi 1 nước nữa mới đạt được trạng thái thắng
							//empty_count = 1 -> bị chặn 1 đầu
							//empty_count = 2 -> không bị chặn
							else if (cur_pattern_size == (m-1)) {
								//kiểm tra phía trên và bên dưới dãy liền nhau
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(start_pattern-1, i);
									if (cur_piece == '.') {
										empty_count++;
									}
								}
								if ((end_pattern+1) < HEIGHT) {
									cur_piece = node.at(end_pattern+1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								//đối thủ không bị chặn nguy hiểm hơn bị chặn 1 đầu
								if (empty_count == 1) {
									node.hscore += SCORE_OPP_M;
								}
								else if (empty_count == 2){
									node.hscore += SCORE_OPP_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}
							//nếu chiều dài ô liền nhau = (m-2), với m là số ô liền nhau cần để thắng
							//empty_count = 1 -> bị chặn 1 đầu
							//empty_count = 2 -> không bị chặn
							else if (cur_pattern_size == (m-2)) {
								//kiểm tra phía trên và bên dưới dãy liền nhau
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(start_pattern-1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < HEIGHT) {
									cur_piece = node.at(end_pattern+1, i);
									if (cur_piece == '.')
										empty_count++;
								}
								//đánh giá trạng thái nguy hiểm
								//empty_count = 1 -> bị chặn 1 đầu
							    //empty_count = 2 -> không bị chặn
								if (empty_count == 1)
									node.hscore -= cur_pattern_size;
								else if (empty_count == 2){
									bool M_tiles = false;
									//nếu không bị chặn 2 đầu...
									//kiểm tra có bị chặn cách 1 ô ở cả 2 đầu hay không
									if ((start_pattern-2) >=0) {
										cur_piece = node.at(start_pattern-2, i);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if ((end_pattern+2) < HEIGHT) {
										cur_piece = node.at(end_pattern+2, i);
										if (cur_piece == '.')
											M_tiles = true;
									}
									//nếu có -> trạng thái deadend
									if (M_tiles)
										node.hscore += SCORE_OPP_M_MINUS;
									else
										node.hscore += SCORE_OPP_DEADEND;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;

							}
							//mặc định trừ điểm bằng số ô liền nhau của đối thủ
							else
								node.hscore -= cur_pattern_size;
						}
					}
				}
				//kiểm tra ô liền nhau trái sang phải
				cur_row = j;
				int cur_column = i;
				cur_piece = node.at(cur_row, cur_column);
				if (cur_piece != '.') {
					std::pair<int, int> cur_pos;
					cur_pos.first = cur_row;
					cur_pos.second = cur_column;
					if (checked_coords_columns.find(cur_pos) == checked_coords_columns.end()) {
						if (cur_piece == cur_player) {
							unsigned int cur_pattern_size = 1;
							int start_pattern = cur_pos.second;
							int end_pattern = cur_pos.second;
							checked_coords_columns.insert(cur_pos);
							if (cur_column+1 < WIDTH) {
								cur_column++;
								cur_piece = node.at(cur_row, cur_column);
								while (cur_piece == cur_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos.second;
									checked_coords_columns.insert(cur_pos);
									cur_pattern_size++;
									if (cur_column+1 < WIDTH) {
										cur_column++;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_WIN;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(cur_row, start_pattern-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < WIDTH) {
									cur_piece = node.at(cur_row, end_pattern+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore += SCORE_M;
								else if (empty_count == 2){
									node.hscore += SCORE_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(cur_row, start_pattern-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < WIDTH) {
									cur_piece = node.at(cur_row, end_pattern+1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}

								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									if ((start_pattern-2) >=0) {
										cur_piece = node.at(cur_row, start_pattern-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if ((end_pattern+2) < WIDTH) {
										cur_piece = node.at(cur_row, end_pattern+2);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}
									if (M_tiles)
										node.hscore += SCORE_M_MINUS;
									else
										node.hscore += SCORE_DEADEND;
								}
								else
									node.hscore += SCORE_DEADEND;
							}
							else
								node.hscore += cur_pattern_size;
						}
						else {
							char opp_player = cur_piece;
							unsigned int cur_pattern_size = 1;
							int start_pattern = cur_pos.second;
							int end_pattern = cur_pos.second;
							checked_coords_columns.insert(cur_pos);
							if (cur_column+1 < WIDTH) {
								cur_column++;
								cur_piece = node.at(cur_row, cur_column);

								while (cur_piece == opp_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos.second;
									checked_coords_columns.insert(cur_pos);
									cur_pattern_size++;
									if (cur_column+1 < WIDTH) {
										cur_column++;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_LOSE;
								node.game_end = true;
								return node;
							}

							else if (cur_pattern_size >m)
								node.hscore += SCORE_OPP_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(cur_row, start_pattern-1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}
								if ((end_pattern+1) < WIDTH) {
									cur_piece = node.at(cur_row, end_pattern+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1) {
									node.hscore += SCORE_OPP_M;
								}
								else if (empty_count == 2){
									node.hscore += SCORE_OPP_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if ((start_pattern-1) >=0) {
									cur_piece = node.at(cur_row, start_pattern-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if ((end_pattern+1) < WIDTH) {
									cur_piece = node.at(cur_row, end_pattern+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore -= cur_pattern_size;
								else if (empty_count == 2){
									bool M_tiles = false;
									if ((start_pattern-2) >=0) {
										cur_piece = node.at(cur_row, start_pattern-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if ((end_pattern+2) < WIDTH) {
										cur_piece = node.at(cur_row, end_pattern+2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if (M_tiles)
										node.hscore += SCORE_OPP_M_MINUS;
									else
										node.hscore += SCORE_OPP_DEADEND;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;

							}
							else
								node.hscore -= cur_pattern_size;
						}
					}
				}
				//kiểm tra ô liền nhau hướng 1h
				cur_row = j;
				cur_column = i;
				cur_piece = node.at(cur_row, cur_column);
				if (cur_piece != '.') {
					std::pair<int, int> cur_pos;
					cur_pos.first = cur_row;
					cur_pos.second = cur_column;
					if (checked_coords_diag_topR.find(cur_pos) == checked_coords_diag_topR.end()) {
						if (cur_piece == cur_player) {
							unsigned int cur_pattern_size = 1;
							std::pair<int, int> start_pattern = cur_pos;
							std::pair<int, int> end_pattern = cur_pos;
							checked_coords_diag_topR.insert(cur_pos);
							if ((cur_column+1 < WIDTH) && (cur_row-1 >= 0)) {
								cur_column++;
								cur_row--;
								cur_piece = node.at(cur_row, cur_column);
								while (cur_piece == cur_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos;
									checked_coords_diag_topR.insert(cur_pos);
									cur_pattern_size++;
									if ((cur_column+1 < WIDTH) && (cur_row-1 >= 0)) {
										cur_column++;
										cur_row--;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_WIN;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if (((start_pattern.first+1) < HEIGHT) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first+1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first-1) >= 0) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first-1, end_pattern.second+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore += SCORE_M;
								else if (empty_count == 2){
									node.hscore += SCORE_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if (((start_pattern.first+1) < HEIGHT) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first+1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first-1) >= 0) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first-1, end_pattern.second+1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}

								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									if (((start_pattern.first+2) < HEIGHT) && ((start_pattern.second-2) >=0)) {
										cur_piece = node.at(start_pattern.first+2, start_pattern.second-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if (((end_pattern.first-2) >= 0) && ((end_pattern.second+2) < WIDTH)) {
										cur_piece = node.at(end_pattern.first-2, end_pattern.second+2);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}

									if (M_tiles)
										node.hscore += SCORE_M_MINUS;
									else
										node.hscore += SCORE_DEADEND;
								}
								else
									node.hscore += SCORE_DEADEND;
							}
							else
								node.hscore += cur_pattern_size;
						}
						else {
							char opp_player = cur_piece;
							unsigned int cur_pattern_size = 1;
							std::pair<int, int> start_pattern = cur_pos;
							std::pair<int, int> end_pattern = cur_pos;
							checked_coords_diag_topR.insert(cur_pos);
							if ((cur_column+1 < board_size) && (cur_row-1 >= 0)) {
								cur_column++;
								cur_row--;
								cur_piece = node.at(cur_row, cur_column);

								while (cur_piece == opp_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos;
									checked_coords_diag_topR.insert(cur_pos);
									cur_pattern_size++;
									if ((cur_column+1 < WIDTH) && (cur_row-1 >= 0)) {
										cur_column++;
										cur_row--;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_LOSE;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OPP_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if (((start_pattern.first+1) < HEIGHT) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first+1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first-1) >= 0) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first-1, end_pattern.second+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore += SCORE_OPP_M;
								else if (empty_count == 2){
									node.hscore += SCORE_OPP_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if (((start_pattern.first+1) < HEIGHT) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first+1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first-1) >= 0) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first-1, end_pattern.second+1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}

								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									if (((start_pattern.first+2) < HEIGHT) && ((start_pattern.second-2) >=0)) {
										cur_piece = node.at(start_pattern.first+2, start_pattern.second-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if (((end_pattern.first-2) >= 0) && ((end_pattern.second+2) < WIDTH)) {
										cur_piece = node.at(end_pattern.first-2, end_pattern.second+2);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}

									if (M_tiles)
										node.hscore += SCORE_OPP_M_MINUS;
									else
										node.hscore += SCORE_OPP_DEADEND;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}
							else
								node.hscore -= cur_pattern_size;
						}
					}
				}
				//kiểm tra ô liền nhau theo hướng 5h
				cur_row = j;
				cur_column = i;
				cur_piece = node.at(cur_row, cur_column);
				if (cur_piece != '.') {
					std::pair<int, int> cur_pos;
					cur_pos.first = cur_row;
					cur_pos.second = cur_column;
					if (checked_coords_diag_botR.find(cur_pos) == checked_coords_diag_botR.end()) {
						if (cur_piece == cur_player) {
							unsigned int cur_pattern_size = 1;
							std::pair<int, int> start_pattern = cur_pos;
							std::pair<int, int> end_pattern = cur_pos;
							checked_coords_diag_botR.insert(cur_pos);
							if ((cur_column+1 < WIDTH) && (cur_row+1 < HEIGHT)) {
								cur_column++;
								cur_row++;
								cur_piece = node.at(cur_row, cur_column);

								while (cur_piece == cur_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos;
									checked_coords_diag_botR.insert(cur_pos);
									cur_pattern_size++;
									if ((cur_column+1 < WIDTH) && (cur_row+1 < HEIGHT)) {
										cur_column++;
										cur_row++;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_WIN;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if (((start_pattern.first-1) >=0) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first-1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first+1) < HEIGHT) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first+1, end_pattern.second+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore += SCORE_M;
								else if (empty_count == 2){
									node.hscore += SCORE_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if (((start_pattern.first-1) >=0) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first-1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first+1) < HEIGHT) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first+1, end_pattern.second+1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}

								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									if (((start_pattern.first-2) >=0) && ((start_pattern.second-2) >=0)) {
										cur_piece = node.at(start_pattern.first-2, start_pattern.second-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if (((end_pattern.first+2) < HEIGHT) && ((end_pattern.second+2) < WIDTH)) {
										cur_piece = node.at(end_pattern.first+2, end_pattern.second+2);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}

									if (M_tiles)
										node.hscore += SCORE_M_MINUS;
									else
										node.hscore += SCORE_DEADEND;
								}
								else
									node.hscore += SCORE_DEADEND;
							}
							else
								node.hscore += cur_pattern_size;
						}
						else {
							char opp_player = cur_piece;
							unsigned int cur_pattern_size = 1;
							std::pair<int, int> start_pattern = cur_pos;
							std::pair<int, int> end_pattern = cur_pos;
							checked_coords_diag_botR.insert(cur_pos);
							if ((cur_column+1 < WIDTH) && (cur_row+1 < HEIGHT)) {
								cur_column++;
								cur_row++;
								cur_piece = node.at(cur_row, cur_column);

								while (cur_piece == opp_player) {
									cur_pos.first = cur_row;
									cur_pos.second = cur_column;
									end_pattern = cur_pos;
									checked_coords_diag_botR.insert(cur_pos);
									cur_pattern_size++;
									if ((cur_column+1 < WIDTH) && (cur_row+1 < HEIGHT)) {
										cur_column++;
										cur_row++;
										cur_piece = node.at(cur_row, cur_column);
									}
									else
										break;
								}
							}

							if (cur_pattern_size == m) {
								node.hscore = SCORE_LOSE;
								node.game_end = true;
								return node;
							}
							else if (cur_pattern_size >m)
								node.hscore += SCORE_OPP_OVER;

							else if (cur_pattern_size == (m-1)) {
								int empty_count = 0;
								if (((start_pattern.first-1) >=0) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first-1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first+1) < HEIGHT) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first+1, end_pattern.second+1);
									if (cur_piece == '.')
										empty_count++;
								}

								if (empty_count == 1)
									node.hscore += SCORE_OPP_M;
								else if (empty_count == 2){
									node.hscore += SCORE_OPP_STRAIGHT_M;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}

							else if (cur_pattern_size == (m-2)) {
								int empty_count = 0;
								if (((start_pattern.first-1) >=0) && ((start_pattern.second-1) >=0)) {
									cur_piece = node.at(start_pattern.first-1, start_pattern.second-1);
									if (cur_piece == '.')
										empty_count++;
								}
								if (((end_pattern.first+1) < HEIGHT) && ((end_pattern.second+1) < WIDTH)) {
									cur_piece = node.at(end_pattern.first+1, end_pattern.second+1);
									if (cur_piece == '.') {
										empty_count++;
									}
								}

								if (empty_count == 1) {
									node.hscore += cur_pattern_size;
								}
								else if (empty_count == 2){
									bool M_tiles = false;
									if (((start_pattern.first-2) >=0) && ((start_pattern.second-2) >=0)) {
										cur_piece = node.at(start_pattern.first-2, start_pattern.second-2);
										if (cur_piece == '.')
											M_tiles = true;
									}
									if (((end_pattern.first+2) < HEIGHT) && ((end_pattern.second+2) < WIDTH)) {
										cur_piece = node.at(end_pattern.first+2, end_pattern.second+2);
										if (cur_piece == '.') {
											M_tiles = true;
										}
									}

									if (M_tiles)
										node.hscore += SCORE_OPP_M_MINUS;
									else
										node.hscore += SCORE_OPP_DEADEND;
								}
								else
									node.hscore += SCORE_OPP_DEADEND;
							}
							else
								node.hscore -= cur_pattern_size;
						}
					}
				}
			}
		}
	}

	if (!node.game_end && (node.tiles_left == 0)) {
		node.game_end = true;
	}
	return node;
}

// Kiểm tra 8 ô xung quanh nước đi
std::deque<GameState> gen_all_moves(GameState cur_board, const unsigned int m, const char score_player, const char player) {
	std::deque<GameState> move_list;
	std::set< std::pair<int, int> > gen_coords_list;
	bool empty_board = true;
	int board_column_size = cur_board.column_count.size();

	for (int i = 0; i < board_column_size; i++) {
		//check if there are any pieces in the column
		if (cur_board.column_count[i]) {
			empty_board = false;
			for (int j = 0; j < board_column_size; j++) {
				if (cur_board.at(j, i) != '.') {
					//add top tile to gen list
					if (j-1 >= 0) {
						if (cur_board.at(j-1, i) == '.') {
							std::pair<int, int> temp(j-1, i);
							gen_coords_list.insert(temp);
						}
					}
					//add bottom tile to gen list
					if (j+1 < board_column_size) {
						if (cur_board.at(j+1, i) == '.') {
							std::pair<int, int> temp(j+1, i);
							gen_coords_list.insert(temp);
						}
					}
					//add right tile to gen list
					if (i+1 < board_column_size) {
						if (cur_board.at(j, i+1) == '.') {
							std::pair<int, int> temp(j, i+1);
							gen_coords_list.insert(temp);
						}
					}
					//add left tile to gen list
					if (i-1 >= 0) {
						if (cur_board.at(j, i-1) == '.') {
							std::pair<int, int> temp(j, i-1);
							gen_coords_list.insert(temp);
						}
					}
					//add top left tile
					if ((j-1 >= 0) && (i-1 >= 0)) {
						if (cur_board.at(j-1, i-1) == '.') {
							std::pair<int, int> temp(j-1, i-1);
							gen_coords_list.insert(temp);
						}
					}
					//add top right tile
					if ((j-1 >= 0) && (i+1 < board_column_size)) {
						if (cur_board.at(j-1, i+1) == '.') {
							std::pair<int, int> temp(j-1, i+1);
							gen_coords_list.insert(temp);
						}
					}
					//add bottom left tile
					if ((j+1 < board_column_size) && (i-1 >= 0)) {
						if (cur_board.at(j+1, i-1) == '.') {
							std::pair<int, int> temp(j+1, i-1);
							gen_coords_list.insert(temp);
						}
					}
					//add bottom right tile
					if ((j+1 < board_column_size) && (i+1 < board_column_size)) {
						if (cur_board.at(j+1, i+1) == '.') {
							std::pair<int, int> temp(j+1, i+1);
							gen_coords_list.insert(temp);
						}
					}
				}
			}
		}
	}
	// Nếu bảng trống thì đánh vào chính giữa
	if (empty_board) {
		int middle_board = board_column_size/2;
		std::pair<int, int> temp(14, 24);
		gen_coords_list.insert(temp);
	}
	for (std::set< std::pair<int, int> >::iterator it = gen_coords_list.begin(); it != gen_coords_list.end(); it++) {
		GameState temp_board = cur_board;
		temp_board.set(it->first, it->second, player);
		temp_board = heuristics_func(temp_board, m, score_player);
		move_list.push_back(temp_board);
	}
	return move_list;
}

//Đi đến vị trí được chỉ định
GameState player_gen_move(GameState cur_board, char player, unsigned int row, unsigned int column) {
	unsigned int pos = (row*cur_board.n)+column;
	unsigned int n = cur_board.n;
	// Kiểm tra bàn cờ hết nước đi chưa
	if ( cur_board.game_end || cur_board.tiles_left == 0) {
		std::cout << "Game has ended or board is filled" << std::endl;
		return cur_board;
	}
	// Kiểm tra nước đi đó đã bị ai đi chưa
	if (((row < 0 || row >= n) || (column < 0 || column >= n)) || cur_board.board[pos] != '.') {
		std::cout << "Illegal move by player " << player << " at " << row << " " << column << std::endl;
	}
	else {
		cur_board.board[pos] = player;
		cur_board.column_count[column] = true;
		cur_board.tiles_left--;
		cur_board.last_column = column;
		cur_board.last_row = row;
	}
	return cur_board;
}

// Giải thuật đưa ra để tìm một nước đi tốt nhất trong các nước đi của heuristic
std::pair<int, std::pair<int, int> > alphabeta(GameState root,
	unsigned int depth, std::pair<int, std::pair<int, int> > alpha,
	std::pair<int, std::pair<int, int> > beta, char player, bool maxPlayer,
	timespec &start_time, const unsigned int &time_limit, bool &cutoff,
	unsigned int m) {

	timespec time_now;
	clock_gettime(CLOCK_REALTIME, &time_now);
	double time_taken = (time_now.tv_sec - start_time.tv_sec)+(time_now.tv_nsec - start_time.tv_nsec)/1000000000.0;

	if (time_taken > time_limit) {
		cutoff = true;
		return alpha;
	}

	//Nếu cutoff, terminal node, hoặc depth bằng 0
	if ((time_taken >= time_limit) || depth == 0 || root.game_end || root.tiles_left == 0) {
		std::pair<int, std::pair<int, int> > hscore;
		hscore.first = root.hscore;

		// Khi gần thắng thì tăng độ sâu lên, khi gần thua thì giảm độ sâu đi
		if (hscore.first == SCORE_WIN)
			hscore.first += depth;
		if (hscore.first == SCORE_LOSE)
			hscore.first -= depth;

		hscore.second.first = root.last_row;
		hscore.second.second = root.last_column;

		return hscore;
	}

	char current_player;
	if (player == 'X') {
		if (maxPlayer)
			current_player = 'X';
		else
			current_player = 'O';
	}
	else if (player == 'O') {
		if (maxPlayer)
			current_player = 'O';
		else
			current_player = 'X';
	}
	else {
		std::cout << "alphabeta error: unrecognized player" << std::endl;
	}

	std::deque<GameState> moves = gen_all_moves(root, m, player, current_player);

	if (maxPlayer) {
		for (std::deque<GameState>::iterator itr = moves.begin(); itr != moves.end(); itr++) {
			std::pair<int, std::pair<int, int> > temp_score;
			temp_score = alphabeta(*itr, depth-1, alpha, beta, player, false, start_time, time_limit, cutoff, m);
			if (cutoff) {
				return alpha;
			}
			if (temp_score.first > alpha.first) {
				alpha.first = temp_score.first;
				alpha.second.first = itr->last_row;
				alpha.second.second = itr->last_column;
			}
			if (alpha.first >= beta.first)
				break;
		}

		return alpha;
	}
	else {
		for (std::deque<GameState>::iterator itr = moves.begin(); itr != moves.end(); itr++) {
			std::pair<int, std::pair<int, int> > temp_score;
			temp_score = alphabeta(*itr, depth-1, alpha, beta, player, true, start_time, time_limit, cutoff, m);
			if (cutoff) {
				return beta;
			}
			if (temp_score.first < beta.first) {
				beta.first = temp_score.first;
				beta.second.first = itr->last_row;
				beta.second.second = itr->last_column;
				if (depth == 3)
					std::cout <<"  changing beta: last_row: " << alpha.second.first << "last_column: " << alpha.second.second << " hscore: "<< alpha.first <<std::endl;

			}
			if (alpha.first >= beta.first)
				break;
		}
		return beta;
	}
}
std::pair<int, int> itr_deep_minimax(GameState root, char player, const unsigned int time_limit, unsigned int m) {
	timespec start;
	std::pair<int, std::pair<int, int> > alpha, beta;
	std::pair<int, int> r_move;
	alpha.first = ALPHA_INF;
	beta.first = BETA_INF;
	unsigned int depth = 1;
	bool cutoff = false;
	std::pair<int, std::pair<int, int> > best_move;
	best_move.first = 0;
	best_move.second.first = -1;
	best_move.second.second = -1;

	// Thiết lập thời gian khi hàm chạy
	clock_gettime(CLOCK_REALTIME, &start);
	while (!cutoff) {
		best_move = alphabeta(root, depth, alpha, beta, player, true, start, time_limit, cutoff, m);
		if (!cutoff) {
			r_move = best_move.second;
			depth+=2;
		}
	}
	return r_move;
}
// Hàm điều khiển quy luật của trò chơi
void mode_two(int board_game[50][50],unsigned int size, const char random_player, const unsigned int time_limit, const unsigned int m,int k){
    srand(time(NULL));
	GameState game_board(size);
	Point p;
	int dem;
	dem = 0;
	if (!game_board.game_end) {
		if (k == -1){
            for(int i = 0; i < 50; i++){
                for(int j = 0; j< 50; j++){
                    if(board_game[i][j] ==  1 ){ game_board = player_gen_move(game_board,'O', i, j);}
                    if(board_game[i][j] == -1 ){ game_board = player_gen_move(game_board,'X', i, j);}

                }
            }
            game_board = heuristics_func(game_board, m,'O');
            std::pair<int, int> results = itr_deep_minimax(game_board,'X', time_limit, m);
			game_board = player_gen_move(game_board,'X', results.first, results.second);
			q.x = results.first;
			q.y = results.second;
		}
		if (k == 1){
            for(int i = 0; i < 50; i++){
                for(int j = 0; j< 50; j++){
                    if(board_game[i][j] ==  1 ){ game_board = player_gen_move(game_board,'O', i, j); dem ++;}
                    if(board_game[i][j] == -1 ){ game_board = player_gen_move(game_board,'X', i, j); dem ++;}

                }
            }
            if(dem == 0){
                std::pair<int, int> results = itr_deep_minimax(game_board,'O', time_limit, m);
                game_board = player_gen_move(game_board,'O', results.first, results.second);
                q.x = results.first;
                q.y = results.second;
            }
            if(dem != 0){
                game_board = heuristics_func(game_board, m,'X');
                std::pair<int, int> results = itr_deep_minimax(game_board,'O', time_limit, m);
                game_board = player_gen_move(game_board,'O', results.first, results.second);
                q.x = results.first;
                q.y = results.second;
            }
		}
	}
}
void padding(int board_game[][50])
{
    for(int i = 0; i < 50 ; i ++){
        for(int j = 0 ; j < 50 ; j ++){
           if(i >= 30){ board_game[i][j] = 0;}
        }
    }
}
// Trả về giá trị cho chương trình
Point check_n_tile_TTV(int board_game[30][50],int k){
    Point p;
    if (k == 1){
        padding(board_game);
        mode_two(board_game,50,'X',5,5,k);
        p.x = q.x;
        p.y = q.y;
    }
    if (k == -1){
        padding(board_game);
        mode_two(board_game,50,'O',5,5,k);
        p.x = q.x;
        p.y = q.y;
    }
    return p;
}
#endif
