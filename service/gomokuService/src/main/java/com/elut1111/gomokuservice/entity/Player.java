package com.elut1111.gomokuservice.entity;

import com.elut1111.gomokuservice.handler.GomokuWebSocketHandler;
import com.fasterxml.jackson.annotation.JsonIgnore;
import lombok.Data;
import lombok.Getter;
import lombok.Setter;
import org.springframework.web.socket.WebSocketSession;

import java.io.Serial;
import java.io.Serializable;

/**
 * 玩家实体
 */
@Data
public class Player implements Serializable {

    // 序列化UID
    @Serial
    private static final long serialVersionUID = 1L;

    /**
     * 玩家颜色：BLACK/WHITE
     */
    @Setter
    @Getter
    private String color;
    /**
     * 玩家WebSocket会话
     */
//    @Getter
//    @JsonIgnore
//    private transient WebSocketSession session;
//    public void setSession(WebSocketSession session) {
//        this.session = session;
//        if (session != null) this.sessionId = session.getId();
//    }
    @Getter
    @Setter
    private String sessionId;

    @JsonIgnore
    public WebSocketSession getSession() {
        return GomokuWebSocketHandler.getSessionBySessionId(sessionId);
    }

    /**
     * 是否在线
     */
    private boolean online;

    /**
     * 玩家准备状态
     */
    @Setter
    @Getter
    private boolean ready;

    public Player() {
    }

//    public Player(String color, WebSocketSession session) {
//        this.color = color;
//        this.online = true;
//        this.ready = false;
//        this.sessionId = session.getId();
//    }
    public Player(String color, String sessionId) {
        this.color = color;
        this.online = true;
        this.ready = false;
        this.sessionId = sessionId;
    }
}