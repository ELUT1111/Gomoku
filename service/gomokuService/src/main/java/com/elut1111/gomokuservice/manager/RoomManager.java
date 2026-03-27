package com.elut1111.gomokuservice.manager;

import com.elut1111.gomokuservice.dto.RoomDTO;
import com.elut1111.gomokuservice.entity.Player;
import com.elut1111.gomokuservice.entity.Room;
import com.elut1111.gomokuservice.handler.GomokuWebSocketHandler;
import jakarta.annotation.Resource;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import jakarta.annotation.PostConstruct;


/**
 * 房间管理器
 */
@Slf4j
@Component
public class RoomManager {
//    /**
//     * id-房间映射：key=roomId，value=Room实体
//     */
//    private static final Map<String, Room> ROOM_MAP = new ConcurrentHashMap<>();
//    /**
//     * 会话-房间映射：key=sessionId，value=roomId
//     */
//    private static final Map<String, String> SESSION_ROOM_MAP = new ConcurrentHashMap<>();

    private static final String ROOM_KEY_PREFIX = "gomoku:room:";
    private static final String SESSION_ROOM_KEY_PREFIX = "gomoku:session:room:";
    private static final String VALID_ROOM_SET_KEY = "gomoku:room:valid:set";
    private static final long ROOM_EXPIRE = 60L;
    @Resource
    private RedisTemplate<String, Object> redisTemplate;

    private static RoomManager instance;
    public RoomManager(RedisTemplate<String, Object> redisTemplate) {
        this.redisTemplate = redisTemplate;
    }
    @PostConstruct
    public void init() {
        instance = this;
    }
    public static RoomManager getInstance() {
        if (instance == null) throw new RuntimeException("RoomManager未初始化");
        return instance;
    }
    /**
     * 创建房间
     * @return 8位roomId
     */
    public String createRoom(WebSocketSession creator) {
        Room room = new Room();
        // 创建者为黑棋玩家
        Player blackPlayer = new Player("BLACK", creator.getId());
        room.setOwnerSessionId(creator.getId()); // 绑定房主
        blackPlayer.setReady(true);
        room.setBlackPlayer(blackPlayer);
//        // 存入房间映射
//        ROOM_MAP.put(room.getRoomId(), room);
//        // 绑定会话-房间
//        SESSION_ROOM_MAP.put(creator.getId(), room.getRoomId());
        redisTemplate.opsForValue().set(ROOM_KEY_PREFIX+room.getRoomId(), room, ROOM_EXPIRE, TimeUnit.MINUTES);
        redisTemplate.opsForValue().set(SESSION_ROOM_KEY_PREFIX+creator.getId(), room.getRoomId(), ROOM_EXPIRE, TimeUnit.MINUTES);
        redisTemplate.opsForSet().add(VALID_ROOM_SET_KEY, room.getRoomId());
        return room.getRoomId();
    }

    /**
     * 加入房间
     */
    public void joinRoom(String roomId, WebSocketSession joiner) {
        Room room = getRoomByRoomId(roomId);
        if (room == null || room.getStatus() != Room.RoomStatus.WAIT) {
            return;
        }
        // 加入者为白棋玩家
        Player whitePlayer = new Player("WHITE", joiner.getId());
        room.setWhitePlayer(whitePlayer);
//        // 绑定会话-房间
//        SESSION_ROOM_MAP.put(joiner.getId(), roomId);
        redisTemplate.opsForValue().set(ROOM_KEY_PREFIX+roomId, room, ROOM_EXPIRE, TimeUnit.MINUTES);
        redisTemplate.opsForValue().set(SESSION_ROOM_KEY_PREFIX+joiner.getId(), roomId, ROOM_EXPIRE, TimeUnit.MINUTES);
    }

    /**
     * 判断房间是否存在
     * @param roomId:roomId
     */
    public boolean existsRoom(String roomId) {
        Room room = getRoomByRoomId(roomId);
        return room != null && room.getStatus() != Room.RoomStatus.CLOSE;
    }

    /**
     * 判断房间是否已满
     */
    public boolean isRoomFull(String roomId) {
        Room room = getRoomByRoomId(roomId);
        return room != null && room.isFull();
    }

    /**
     * 获取房间内的对手会话
     */
    public WebSocketSession getOpponent(String roomId, WebSocketSession currentSession) {
        Room room = getRoomByRoomId(roomId);
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
        Room room = getRoomByRoomId(roomId);
        return room != null && room.isPlayerInRoom(session.getId());
    }

    /**
     * 移除断开连接的会话，清理房间
     */
    public void removeSession(WebSocketSession session) {
        String sessionId = session.getId();
        String roomId = getRoomIdBySessionId(sessionId);
        if (roomId == null) {
            return;
        }
        Room room = getRoomByRoomId(roomId);
        if (room == null) {
            redisTemplate.delete(SESSION_ROOM_KEY_PREFIX + sessionId);
            return;
        }

        // 标记玩家为离线
        Player player = getPlayerBySession(room, session);
        if (player != null) {
            player.setOnline(false);
        }

        destroyRoom(roomId);
//        // 房间状态改为关闭，清理映射
//        room.setStatus(Room.RoomStatus.CLOSE);
//        ROOM_MAP.remove(roomId);
//        // 清理该房间所有会话绑定
//        SESSION_ROOM_MAP.entrySet().removeIf(entry -> entry.getValue().equals(roomId));
    }

    /**
     * 销毁房间
     */
    public void destroyRoom(String roomId) {
//        Room room = getRoomByRoomId(roomId);
//        if (room == null) {
//            return;
//        }
//        room.setStatus(Room.RoomStatus.CLOSE);
//        ROOM_MAP.remove(roomId);
//        // 清理会话-房间绑定
//        SESSION_ROOM_MAP.entrySet().removeIf(entry -> entry.getValue().equals(roomId));
        redisTemplate.delete(ROOM_KEY_PREFIX+roomId);
        redisTemplate.opsForSet().remove(VALID_ROOM_SET_KEY, roomId);
        // 清理会话绑定
        Set<String> sessionKeys = redisTemplate.keys(SESSION_ROOM_KEY_PREFIX + "*");
        if (sessionKeys != null && !sessionKeys.isEmpty()) {
            for (String key : sessionKeys) {
                String rid = (String) redisTemplate.opsForValue().get(key);
                if (roomId.equals(rid)) {
                    redisTemplate.delete(key);
                }
            }
        }
        log.info("[Redis] 房间已销毁：{}", roomId);
    }

    /**
     * 玩家退出房间
     */
    public void quitRoom(WebSocketSession session, String roomId) {
        String sessionId = session.getId();
        Room room = getRoomByRoomId(roomId);
        if (room == null) return;

        // 移除会话-房间绑定
//        SESSION_ROOM_MAP.remove(sessionId);
        redisTemplate.delete(SESSION_ROOM_KEY_PREFIX + sessionId);
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
//            room.setStatus(Room.RoomStatus.CLOSE);
//            ROOM_MAP.remove(roomId);
//            log.info("[RoomManager] 房间已销毁：{}", roomId);
            destroyRoom(roomId);
        } else {
            // 剩余玩家则重置为等待状态
            room.setStatus(Room.RoomStatus.WAIT);
            room.setCurrentPlayer("BLACK");

            // 重置准备状态
            Player remainingPlayer = room.getBlackPlayer() != null ?
                    room.getBlackPlayer() : room.getWhitePlayer();
            if (remainingPlayer != null) remainingPlayer.setReady(true);
            redisTemplate.opsForValue().set(ROOM_KEY_PREFIX + roomId, room, ROOM_EXPIRE, TimeUnit.MINUTES);
            log.info("[RoomManager] 玩家退出，房间重置等待状态：{}", roomId);
        }
        log.info("[RoomManager] 玩家退出清理完成：session={}, roomId={}", sessionId, roomId);
    }

    /**
     * 根据房间ID获取房间实体
     */
    public Room getRoomByRoomId(String roomId) {
        if (roomId == null) return null;
        Room room = (Room) redisTemplate.opsForValue().get(ROOM_KEY_PREFIX + roomId);
//        // 补全玩家会话
//        if (room != null) {
//            fillPlayerSession(room.getBlackPlayer());
//            fillPlayerSession(room.getWhitePlayer());
//        }
        return room;
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
        return (String) redisTemplate.opsForValue().get(SESSION_ROOM_KEY_PREFIX+sessionId);
    }

    /**
     * 获取可加入等待房间
     * @return 可加入房间列表
     */
    public List<Room> getWaitAndNotFullRooms() {
//        List<Room> result = new ArrayList<>();
//        for (Room room : ROOM_MAP.values()) {
//            if (room.getStatus() == Room.RoomStatus.WAIT && !room.isFull()) {
//                result.add(room);
//            }
//        }
//        return result;
        Set<Object> roomIds = redisTemplate.opsForSet().members(VALID_ROOM_SET_KEY);
        if (roomIds == null || roomIds.isEmpty()) return new ArrayList<>();

        List<Room> result = new ArrayList<>();
        for (Object roomId : roomIds) {
            Room room = getRoomByRoomId((String) roomId);
            if (room != null && room.getStatus() == Room.RoomStatus.WAIT && !room.isFull()) {
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
//        List<RoomDTO> list = new ArrayList<>();
//        for (Map.Entry<String, Room> entry : ROOM_MAP.entrySet()) {
//            Room room = entry.getValue();
//            if (room.getStatus() == Room.RoomStatus.CLOSE) continue;
//            RoomDTO dto = new RoomDTO();
//            dto.setRoomId(room.getRoomId());
//            dto.setStatus(room.getStatus().name());
//            // 计算人数
//            int count = 0;
//            if (room.getBlackPlayer() != null) count++;
//            if (room.getWhitePlayer() != null) count++;
//            dto.setPlayerCount(count);
//            dto.setCreator("房主");
//            list.add(dto);
//        }
//        return list;
        Set<Object> roomIds = redisTemplate.opsForSet().members(VALID_ROOM_SET_KEY);
        if (roomIds == null || roomIds.isEmpty()) return new ArrayList<>();

        List<RoomDTO> list = new ArrayList<>();
        for (Object roomId : roomIds) {
            Room room = getRoomByRoomId((String) roomId);
            if (room == null || room.getStatus() == Room.RoomStatus.CLOSE) continue;

            RoomDTO dto = new RoomDTO();
            dto.setRoomId(room.getRoomId());
            dto.setStatus(room.getStatus().name());
            int count = 0;
            if (room.getBlackPlayer() != null) count++;
            if (room.getWhitePlayer() != null) count++;
            dto.setPlayerCount(count);
            dto.setCreator("房主");
            list.add(dto);
        }
        return list;
    }
    private void fillPlayerSession(Player player) {
        if (player == null) return;
        String sessionId = player.getSessionId();
        WebSocketSession session = GomokuWebSocketHandler.ONLINE_SESSIONS.get(sessionId);
        player.setSessionId(session.getId());
    }

}