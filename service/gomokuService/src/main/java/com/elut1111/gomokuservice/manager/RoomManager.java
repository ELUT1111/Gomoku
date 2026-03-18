package com.elut1111.gomokuservice.manager;

import com.elut1111.gomokuservice.entity.Player;
import com.elut1111.gomokuservice.entity.Room;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 房间管理器（内存存储，后续可扩展Redis持久化）
 */
@Component
public class RoomManager {
    // 房间映射：key=roomId，value=Room实体
    private static final Map<String, Room> ROOM_MAP = new ConcurrentHashMap<>();
    // 会话-房间映射：key=sessionId，value=roomId
    private static final Map<String, String> SESSION_ROOM_MAP = new ConcurrentHashMap<>();

    /**
     * 创建房间（返回8位短ID）
     */
    public String createRoom(WebSocketSession creator) {
        Room room = new Room();
        // 创建者为黑棋玩家
        Player blackPlayer = new Player("BLACK", creator);
        blackPlayer.setReady(true);
        room.setBlackPlayer(blackPlayer);
        // 存入房间映射
        ROOM_MAP.put(room.getRoomId(), room);
        // 绑定会话-房间
        SESSION_ROOM_MAP.put(creator.getId(), room.getRoomId());
        return room.getRoomId();
    }

    /**
     * 加入房间
     */
    public void joinRoom(String roomId, WebSocketSession joiner) {
        Room room = ROOM_MAP.get(roomId);
        if (room == null || room.getStatus() != Room.RoomStatus.WAIT) {
            return;
        }
        // 加入者为白棋玩家
        Player whitePlayer = new Player("WHITE", joiner);
        room.setWhitePlayer(whitePlayer);
        // 绑定会话-房间
        SESSION_ROOM_MAP.put(joiner.getId(), roomId);
    }

    /**
     * 判断房间是否存在
     */
    public boolean existsRoom(String roomId) {
        Room room = ROOM_MAP.get(roomId);
        return room != null && room.getStatus() != Room.RoomStatus.CLOSE;
    }

    /**
     * 判断房间是否已满
     */
    public boolean isRoomFull(String roomId) {
        Room room = ROOM_MAP.get(roomId);
        return room != null && room.isFull();
    }

    /**
     * 获取房间内的对手会话
     */
    public WebSocketSession getOpponent(String roomId, WebSocketSession currentSession) {
        Room room = ROOM_MAP.get(roomId);
        if (room == null || !room.isFull()) {
            return null;
        }
        Player currentPlayer = getPlayerBySession(room, currentSession);
        Player opponent = room.getOpponent(currentPlayer);
        return opponent != null && opponent.isOnline() ? opponent.getSession() : null;
    }

    /**
     * 判断会话是否在指定房间内
     */
    public boolean isInRoom(String roomId, WebSocketSession session) {
        Room room = ROOM_MAP.get(roomId);
        return room != null && room.isPlayerInRoom(session.getId());
    }

    /**
     * 移除断开连接的会话，清理房间
     */
    public void removeSession(WebSocketSession session) {
        String sessionId = session.getId();
        String roomId = SESSION_ROOM_MAP.get(sessionId);
        if (roomId == null) {
            return;
        }
        Room room = ROOM_MAP.get(roomId);
        if (room == null) {
            SESSION_ROOM_MAP.remove(sessionId);
            return;
        }

        // 标记玩家为离线
        Player player = getPlayerBySession(room, session);
        if (player != null) {
            player.setOnline(false);
        }

        // 房间状态改为关闭，清理映射
        room.setStatus(Room.RoomStatus.CLOSE);
        ROOM_MAP.remove(roomId);
        // 清理该房间所有会话绑定
        SESSION_ROOM_MAP.entrySet().removeIf(entry -> entry.getValue().equals(roomId));
    }

    /**
     * 销毁房间
     */
    public void destroyRoom(String roomId) {
        Room room = ROOM_MAP.get(roomId);
        if (room == null) {
            return;
        }
        room.setStatus(Room.RoomStatus.CLOSE);
        ROOM_MAP.remove(roomId);
        // 清理会话-房间绑定
        SESSION_ROOM_MAP.entrySet().removeIf(entry -> entry.getValue().equals(roomId));
    }

    /**
     * 根据房间ID获取房间实体
     */
    public Room getRoomByRoomId(String roomId) {
        return ROOM_MAP.get(roomId);
    }

    /**
     * 根据会话获取玩家实体
     */
    public Player getPlayerBySession(Room room, WebSocketSession session) {
        if (room.getBlackPlayer() != null && room.getBlackPlayer().getSession().getId().equals(session.getId())) {
            return room.getBlackPlayer();
        } else if (room.getWhitePlayer() != null && room.getWhitePlayer().getSession().getId().equals(session.getId())) {
            return room.getWhitePlayer();
        }
        return null;
    }

    /**
     * 根据会话ID获取所在房间ID
     */
    public String getRoomIdBySessionId(String sessionId) {
        return SESSION_ROOM_MAP.get(sessionId);
    }
}