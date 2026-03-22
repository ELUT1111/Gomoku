package com.elut1111.gomokuservice.manager;

import com.elut1111.gomokuservice.dto.RoomDTO;
import com.elut1111.gomokuservice.entity.Player;
import com.elut1111.gomokuservice.entity.Room;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 房间管理器
 */
@Slf4j
@Component
public class RoomManager {
    /**
     * id-房间映射：key=roomId，value=Room实体
     */
    private static final Map<String, Room> ROOM_MAP = new ConcurrentHashMap<>();
    /**
     * 会话-房间映射：key=sessionId，value=roomId
     */
    private static final Map<String, String> SESSION_ROOM_MAP = new ConcurrentHashMap<>();

    private static RoomManager instance;
    public static RoomManager getInstance() {
        if (instance == null) {
            instance = new RoomManager();
        }
        return instance;
    }
    /**
     * 创建房间
     * @return 8位roomId
     */
    public String createRoom(WebSocketSession creator) {
        Room room = new Room();
        // 创建者为黑棋玩家
        Player blackPlayer = new Player("BLACK", creator);
        room.setOwnerSessionId(creator.getId()); // 绑定房主
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
     * @param roomId:roomId
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
     * 玩家退出房间
     */
    public void quitRoom(WebSocketSession session, String roomId) {
        String sessionId = session.getId();
        Room room = ROOM_MAP.get(roomId);
        if (room == null) return;

        // 移除会话-房间绑定
        SESSION_ROOM_MAP.remove(sessionId);

        // 清空房间指定玩家
        Player player = getPlayerBySession(room, session);
        if (player == null) return;

        boolean isOwnerQuit = room.isOwner(session);
        if (isOwnerQuit && room.isFull()) {
            room.transferOwnerToOpponent(session); // 转移房主
            log.info("[RoomManager] 房主退出，已转移房主给对手，房间ID：{}", roomId);
        }

        if ("BLACK".equals(player.getColor())) {
            room.setBlackPlayer(null);
        } else {
            room.setWhitePlayer(null);
        }

        // 房间无玩家则直接销毁
        if (room.getBlackPlayer() == null && room.getWhitePlayer() == null) {
            room.setStatus(Room.RoomStatus.CLOSE);
            ROOM_MAP.remove(roomId);
            log.info("[RoomManager] 房间已销毁：{}", roomId);
        } else {
            // 剩余玩家则重置为等待状态
            room.setStatus(Room.RoomStatus.WAIT);
            room.setCurrentPlayer("BLACK");

            // 重置准备状态
            Player remainingPlayer = room.getBlackPlayer() != null ? room.getBlackPlayer() : room.getWhitePlayer();
            if (remainingPlayer != null) remainingPlayer.setReady(false);
            log.info("[RoomManager] 玩家退出，房间重置等待状态：{}", roomId);
        }

        log.info("[RoomManager] 玩家退出清理完成：session={}, roomId={}", sessionId, roomId);
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

    /**
     * 获取可加入等待房间
     * @return 可加入房间列表
     */
    public List<Room> getWaitAndNotFullRooms() {
        List<Room> result = new ArrayList<>();
        for (Room room : ROOM_MAP.values()) {
            if (room.getStatus() == Room.RoomStatus.WAIT && !room.isFull()) {
                result.add(room);
            }
        }
        return result;
    }

    /**
     * 获取所有未关闭有效房间
     * @return Room数据传输体列表
     */
    public List<RoomDTO> getValidRoomList() {
        List<RoomDTO> list = new ArrayList<>();
        for (Map.Entry<String, Room> entry : ROOM_MAP.entrySet()) {
            Room room = entry.getValue();
            if (room.getStatus() == Room.RoomStatus.CLOSE) continue;
            RoomDTO dto = new RoomDTO();
            dto.setRoomId(room.getRoomId());
            dto.setStatus(room.getStatus().name());
            // 计算人数
            int count = 0;
            if (room.getBlackPlayer() != null) count++;
            if (room.getWhitePlayer() != null) count++;
            dto.setPlayerCount(count);
            dto.setCreator("房主");
            list.add(dto);
        }
        return list;
    }
}