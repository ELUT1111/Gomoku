package com.elut1111.gomokuservice.entity;

import lombok.Data;
import org.springframework.web.socket.WebSocketSession;

import java.io.Serializable;

/**
 * 玩家实体
 */
@Data
public class Player implements Serializable {
    /**
     * 玩家颜色：BLACK/WHITE
     */
    private String color;
    /**
     * 玩家WebSocket会话
     */
    private WebSocketSession session;
    /**
     * 是否在线
     */
    private boolean online;

    public Player(String color, WebSocketSession session) {
        this.color = color;
        this.session = session;
        this.online = true;
    }
}