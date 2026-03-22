package com.elut1111.gomokuservice.manager;

import com.elut1111.gomokuservice.dto.GomokuMessage;
import com.elut1111.gomokuservice.entity.Player;
import com.elut1111.gomokuservice.entity.Room;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import tools.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * 随机匹配管理器（优先匹配已有等待房间）
 */
@Component
public class MatchManager {
    // 匹配等待队列
    private final ConcurrentLinkedQueue<WebSocketSession> matchQueue = new ConcurrentLinkedQueue<>();
    // 房间管理器
    private final RoomManager roomManager = RoomManager.getInstance();
    private final ObjectMapper objectMapper = new ObjectMapper();

    // 单例
    private static MatchManager instance;
    public static MatchManager getInstance() {
        if (instance == null) {
            instance = new MatchManager();
        }
        return instance;
    }
    /**
     * 加入匹配
     */
    public void addToMatchQueue(WebSocketSession session) {
        // 先查找等待中、未满员的房间
        List<Room> waitRooms = roomManager.getWaitAndNotFullRooms();
        if (!waitRooms.isEmpty()) {
            // 随机选一个等待房间加入
            Room targetRoom = waitRooms.get((int)(Math.random() * waitRooms.size()));
            joinExistRoom(session, targetRoom);
            return;
        }

        // 无可用房间 → 加入匹配队列
        if (!matchQueue.contains(session)) {
            matchQueue.offer(session);
        }
        // 尝试队列配对
        tryMatchFromQueue();
    }

    /**
     * 加入已有等待房间
     */
    private void joinExistRoom(WebSocketSession session, Room room) {
        String roomId = room.getRoomId();
        roomManager.joinRoom(roomId, session);

        // 通知加入者
        GomokuMessage resp = new GomokuMessage();
        resp.setType("JOIN_SUCCESS");
        resp.setRoomId(roomId);
        resp.setPlayer("WHITE"); // 加入者默认白棋
        resp.setMsg("匹配成功，加入房间：" + roomId);
        sendMsgToSession(session, resp);

        // 通知房间内的黑棋玩家（对手已加入）
        WebSocketSession blackPlayer = roomManager.getOpponent(roomId, session);
        if (blackPlayer != null && blackPlayer.isOpen()) {
            GomokuMessage blackMsg = new GomokuMessage();
            blackMsg.setType("JOIN_SUCCESS");
            blackMsg.setRoomId(roomId);
            blackMsg.setMsg("对手已加入！");
            blackMsg.setDecision(true);
            sendMsgToSession(blackPlayer, blackMsg);
        }
    }

    /**
     * 从队列配对
     */
    private void tryMatchFromQueue() {
        if (matchQueue.size() < 2) {
            return;
        }

        // 取出两个玩家新建房间
        WebSocketSession p1 = matchQueue.poll();
        WebSocketSession p2 = matchQueue.poll();

        // 防止玩家匹配中途断开
        if (p1 == null || !p1.isOpen()) {
            if (p2 != null && p2.isOpen()) matchQueue.offer(p2);
            return;
        }
        if (p2 == null || !p2.isOpen()) {
            matchQueue.offer(p1);
            return;
        }
        // 随机分配黑白方：黑方创建房间，白方加入房间
        boolean p1IsBlack = Math.random() > 0.5;
        WebSocketSession blackSession = p1IsBlack ? p1 : p2;
        WebSocketSession whiteSession = p1IsBlack ? p2 : p1;

        // blackSession 创建新房间
        String roomId = roomManager.createRoom(blackSession);
        // whiteSession 加入该房间
        roomManager.joinRoom(roomId, whiteSession);

        // 通知 房主
        GomokuMessage msg1 = new GomokuMessage();
        msg1.setType("CREATE_ROOM_STATUS");
        msg1.setDecision(true);
        msg1.setRoomId(roomId);
        msg1.setPlayer("BLACK");
        msg1.setMsg("匹配成功，等待准备游戏！");
        sendMsgToSession(blackSession, msg1);

        // 通知 加入者
        GomokuMessage msg2 = new GomokuMessage();
        msg2.setType("JOIN_SUCCESS");
        msg2.setRoomId(roomId);
        msg2.setPlayer("WHITE");
        msg2.setMsg("匹配成功，请准备！");
        sendMsgToSession(whiteSession, msg2);

        // 额外通知房主白方已加入
        GomokuMessage notifyHost = new GomokuMessage();
        notifyHost.setType("JOIN_SUCCESS");
        notifyHost.setRoomId(roomId);
        notifyHost.setMsg("对手匹配成功已就绪！");
        notifyHost.setDecision(true);
        sendMsgToSession(blackSession, notifyHost);
    }

    /**
     * 退出匹配队列
     */
    public void removeFromMatchQueue(WebSocketSession session) {
        matchQueue.remove(session);
    }

    private void sendMsgToSession(WebSocketSession session, GomokuMessage msg) {
        try {
            if (session.isOpen()) {
                session.sendMessage(new TextMessage(objectMapper.writeValueAsString(msg)));
            }
        } catch (IOException ignored) {}
    }

    private void sendErrorMsg(WebSocketSession session, String errorMsg) {
        GomokuMessage msg = new GomokuMessage();
        msg.setType("ERROR");
        msg.setMsg(errorMsg);
        sendMsgToSession(session, msg);
    }

}