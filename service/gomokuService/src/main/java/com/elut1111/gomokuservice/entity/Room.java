package com.elut1111.gomokuservice.entity;

import lombok.Data;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 房间实体
 */
@Data
public class Room implements Serializable {
    /**
     * 房间ID（8位短ID）
     */
    private String roomId;
    /**
     * 房间状态：WAIT-等待对手，PLAYING-游戏中，END-游戏结束，CLOSE-房间关闭
     */
    private RoomStatus status;
    /**
     * 棋盘实例
     */
    private ChessBoard chessBoard;
    /**
     * 黑棋玩家
     */
    private Player blackPlayer;
    /**
     * 白棋玩家
     */
    private Player whitePlayer;
    /**
     * 当前落子玩家颜色
     */
    private String currentPlayer;

    public List<Player> getPlayers() {
        List<Player> players = new ArrayList<>();
        if(blackPlayer != null) players.add(blackPlayer);
        if(whitePlayer != null) players.add(whitePlayer);
        return players;
    }

    // 房间状态枚举
    public enum RoomStatus {
        WAIT, PLAYING, END, CLOSE
    }

    public Room() {
        // 生成8位短房间ID
        this.roomId = UUID.randomUUID().toString().substring(0, 8).toLowerCase();
        this.status = RoomStatus.WAIT;
        this.chessBoard = new ChessBoard();
        // 黑棋先行
        this.currentPlayer = "BLACK";
    }

    /**
     * 获取房间内的对手玩家
     */
    public Player getOpponent(Player player) {
        if ("BLACK".equals(player.getColor())) {
            return whitePlayer;
        } else {
            return blackPlayer;
        }
    }

    /**
     * 切换当前落子玩家
     */
    public void switchCurrentPlayer() {
        this.currentPlayer = "BLACK".equals(this.currentPlayer) ? "WHITE" : "BLACK";
    }

    /**
     * 判断房间是否已满（2名玩家）
     */
    public boolean isFull() {
        return blackPlayer != null && whitePlayer != null;
    }

    /**
     * 判断玩家是否在房间内
     */
    public boolean isPlayerInRoom(String sessionId) {
        return (blackPlayer != null && blackPlayer.getSession().getId().equals(sessionId))
                || (whitePlayer != null && whitePlayer.getSession().getId().equals(sessionId));
    }

    /**
     * 判断双方是否准备完毕
     */
    public boolean isAllReady() {
        return blackPlayer != null && blackPlayer.isReady() &&
                whitePlayer != null && whitePlayer.isReady();
    }
}