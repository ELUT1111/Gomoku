package com.elut1111.gomokuservice.entity;

import lombok.Data;
import org.springframework.web.socket.WebSocketSession;

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

    /**
     * 房主sessionId
     */
    private String ownerSessionId;

    /**
     * 获取玩家列表
     * @return 当前玩家列表
     */
    public List<Player> getPlayers() {
        List<Player> players = new ArrayList<>();
        if(blackPlayer != null) players.add(blackPlayer);
        if(whitePlayer != null) players.add(whitePlayer);
        return players;
    }

    /**
     * 房间状态枚举:
     *  WHIT, PLAYING, END, CLOSE
     */
    public enum RoomStatus {
        WAIT, PLAYING, END, CLOSE
    }

    public Room() {
        // 生成8位短房间ID
        this.roomId = UUID.randomUUID().toString().substring(0, 8).toLowerCase();
        this.status = RoomStatus.WAIT;
        this.chessBoard = new ChessBoard();
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
     * 判断房间是否已满
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

    /**
     * 判断指定对话是否是房主
     * @param session:会话
     */
    public boolean isOwner(WebSocketSession session) {
        return this.ownerSessionId.equals(session.getId());
    }

    /**
     * 转移房主给对手
     */
    public void transferOwnerToOpponent(WebSocketSession oldOwnerSession) {
        Player opponent = getOpponent(getPlayerBySession(oldOwnerSession));
        if (opponent != null) {
            this.ownerSessionId = opponent.getSession().getId();
            // 新房主默认执黑
            if (!opponent.getColor().equals("BLACK")) {
                swapPlayerColor();
            }
        }
    }

    /**
     * 交换黑白棋双方
     */
    private void swapPlayerColor() {
        Player temp = blackPlayer;
        blackPlayer = whitePlayer;
        whitePlayer = temp;
        if (blackPlayer != null) blackPlayer.setColor("BLACK");
        if (whitePlayer != null) whitePlayer.setColor("WHITE");
    }

    /**
     * 根据会话获取玩家
     */
    public Player getPlayerBySession(WebSocketSession session) {
        if (blackPlayer != null && blackPlayer.getSession().getId().equals(session.getId())) {
            return blackPlayer;
        } else if (whitePlayer != null && whitePlayer.getSession().getId().equals(session.getId())) {
            return whitePlayer;
        }
        return null;
    }
}