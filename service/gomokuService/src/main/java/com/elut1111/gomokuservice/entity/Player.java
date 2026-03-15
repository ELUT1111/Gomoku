package com.elut1111.gomokuservice.entity;

import lombok.Data;
import lombok.Getter;
import lombok.Setter;
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
    @Setter
    private String color;
    /**
     * 玩家WebSocket会话
     */
    @Setter
    private WebSocketSession session;
    /**
     * 是否在线
     */
    private boolean online;

    @Setter
    @Getter
    private boolean ready;

    public Player(String color, WebSocketSession session) {
        this.color = color;
        this.session = session;
        this.online = true;
        this.ready = false;
    }

}