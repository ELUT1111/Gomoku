package com.elut1111.gomokuservice.entity;

import lombok.Data;

import java.util.LinkedList;
import java.util.List;

/**
 * 棋盘实体（15*15）
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
    /**
     * 落子历史
     */
    private List<ChessMove> moveHistory;

    @Data
    public static class ChessMove {
        private int x;
        private int y;
        private String color;
        public ChessMove(int x, int y, String color) {
            this.x = x;
            this.y = y;
            this.color = color;
        }
    }

    public ChessBoard() {
        // 初始化空棋盘
        this.board = new int[BOARD_SIZE][BOARD_SIZE];
        this.chessCount = 0;
        this.moveHistory = new LinkedList<>();
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
        moveHistory.clear();
    }

    /**
     * 落子校验：位置是否合法
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
        moveHistory.add(new ChessMove(x, y, color));
        return true;
    }

    /**
     * 获取棋子颜色
     * @return "BLACK"|"WHITE"
     */
    public String getChessColor(int x, int y) {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            return null;
        }
        return board[x][y] == 1 ? "BLACK" : (board[x][y] == 2 ? "WHITE" : null);
    }

    /**
     * 悔棋
     * @return 悔棋状态
     */
    public boolean undo() {
        if (moveHistory.isEmpty()) {
            return false;
        }
        ChessMove lastMove = moveHistory.remove(moveHistory.size() - 1);
        board[lastMove.getX()][lastMove.getY()] = 0;
        chessCount--;
        return true;
    }
}