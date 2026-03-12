package com.elut1111.gomokuservice.manager;

import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 房间管理器（内存存储，开发/测试用）
 */
@Component
public class RoomManager {
    // 房间映射：key=roomId，value=[创建者会话, 加入者会话]
    private static final Map<String, WebSocketSession[]> ROOM_MAP = new ConcurrentHashMap<>();

    /**
     * 创建房间，返回8位短房间ID
     */
    public String createRoom(WebSocketSession creator) {
        String roomId = UUID.randomUUID().toString().substring(0, 8);
        ROOM_MAP.put(roomId, new WebSocketSession[]{creator, null});
        return roomId;
    }

    /**
     * 加入房间
     */
    public void joinRoom(String roomId, WebSocketSession joiner) {
        WebSocketSession[] sessions = ROOM_MAP.get(roomId);
        if (sessions != null) {
            sessions[1] = joiner;
            ROOM_MAP.put(roomId, sessions);
        }
    }

    /**
     * 判断房间是否存在
     */
    public boolean existsRoom(String roomId) {
        return ROOM_MAP.containsKey(roomId);
    }

    /**
     * 判断房间是否已满（2人）
     */
    public boolean isRoomFull(String roomId) {
        WebSocketSession[] sessions = ROOM_MAP.get(roomId);
        return sessions != null && sessions[1] != null;
    }

    /**
     * 获取房间内的对手会话
     */
    public WebSocketSession getOpponent(String roomId, WebSocketSession currentSession) {
        WebSocketSession[] sessions = ROOM_MAP.get(roomId);
        if (sessions == null) return null;
        return sessions[0] == currentSession ? sessions[1] : sessions[0];
    }

    /**
     * 判断会话是否在指定房间内
     */
    public boolean isInRoom(String roomId, WebSocketSession session) {
        WebSocketSession[] sessions = ROOM_MAP.get(roomId);
        return sessions != null && (sessions[0] == session || sessions[1] == session);
    }

    /**
     * 移除断开连接的会话，清理空房间
     */
    public void removeSession(WebSocketSession session) {
        ROOM_MAP.entrySet().removeIf(entry -> {
            WebSocketSession[] sessions = entry.getValue();
            boolean contains = sessions[0] == session || sessions[1] == session;
            if (contains) {
                // 房间内有玩家断开，直接销毁房间
                return true;
            }
            return false;
        });
    }

    /**
     * 销毁房间
     */
    public void destroyRoom(String roomId) {
        ROOM_MAP.remove(roomId);
    }
}