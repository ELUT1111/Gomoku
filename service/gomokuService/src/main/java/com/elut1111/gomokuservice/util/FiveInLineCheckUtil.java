package com.elut1111.gomokuservice.util;

import com.elut1111.gomokuservice.entity.ChessBoard;

/**
 * 五子连珠校验工具类
 */
public class FiveInLineCheckUtil {
    /**
     * 连珠胜利数（五子棋=5）
     */
    private static final int WIN_COUNT = 5;
    /**
     * 四个检测方向：右、下、右下、右上（与前端DIRS数组前4个方向一致）
     */
    private static final int[][] DIRS = {{0, 1}, {1, 0}, {1, 1}, {-1, 1}};

    /**
     * 校验是否五子连珠
     * @param x 落子横坐标
     * @param y 落子纵坐标
     * @param color 落子颜色
     * @param chessBoard 棋盘实例
     * @return 是否胜利
     */
    public static boolean checkFiveInLine(int x, int y, String color, ChessBoard chessBoard) {
        int chessValue = "BLACK".equals(color) ? 1 : 2;
        int[][] board = chessBoard.getBoard();

        // 遍历四个方向检测
        for (int[] dir : DIRS) {
            int count = 1;
            int dx = dir[0];
            int dy = dir[1];

            // 正向检测
            int nx = x + dx;
            int ny = y + dy;
            while (nx >= 0 && nx < ChessBoard.BOARD_SIZE && ny >= 0 && ny < ChessBoard.BOARD_SIZE
                    && board[nx][ny] == chessValue) {
                count++;
                nx += dx;
                ny += dy;
            }

            // 反向检测
            nx = x - dx;
            ny = y - dy;
            while (nx >= 0 && nx < ChessBoard.BOARD_SIZE && ny >= 0 && ny < ChessBoard.BOARD_SIZE
                    && board[nx][ny] == chessValue) {
                count++;
                nx -= dx;
                ny -= dy;
            }

            // 达到五子连珠
            if (count >= WIN_COUNT) {
                return true;
            }
        }
        return false;
    }
}