package com.elut1111.gomokuservice.entity;

import lombok.Data;

/**
 * 棋盘实体（15*15，与Qt客户端一致）
 */
@Data
public class ChessBoard {
    /**
     * 棋盘大小
     */
    public static final int BOARD_SIZE = 15;
    /**
     * 棋盘状态：0-空，1-黑棋，2-白棋
     */
    private int[][] board;
    /**
     * 落子总数
     */
    private int chessCount;

    public ChessBoard() {
        // 初始化空棋盘
        this.board = new int[BOARD_SIZE][BOARD_SIZE];
        this.chessCount = 0;
        clearBoard();
    }

    /**
     * 清空棋盘
     */
    public void clearBoard() {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = 0;
            }
        }
        chessCount = 0;
    }

    /**
     * 落子校验：位置是否合法（越界/已有棋子）
     */
    public boolean checkChessPos(int x, int y) {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            return false;
        }
        return board[x][y] == 0;
    }

    /**
     * 执行落子
     * @param x 横坐标
     * @param y 纵坐标
     * @param color 棋子颜色：BLACK(1)/WHITE(2)
     * @return 落子是否成功
     */
    public boolean placeChess(int x, int y, String color) {
        if (!checkChessPos(x, y)) {
            return false;
        }
        board[x][y] = "BLACK".equals(color) ? 1 : 2;
        chessCount++;
        return true;
    }

    /**
     * 获取棋子颜色（反向转换）
     */
    public String getChessColor(int x, int y) {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            return null;
        }
        return board[x][y] == 1 ? "BLACK" : (board[x][y] == 2 ? "WHITE" : null);
    }
}