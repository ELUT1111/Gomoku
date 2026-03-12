package com.elut1111.gomokuservice.handler;


import com.elut1111.gomokuservice.dto.GomokuMessage;
import com.elut1111.gomokuservice.manager.RoomManager;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.annotation.Resource;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;

import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 五子棋WebSocket业务处理器
 */
@Slf4j
@Component
public class GomokuWebSocketHandler extends TextWebSocketHandler {

    // 在线会话缓存
    private static final Map<String, WebSocketSession> ONLINE_SESSIONS = new ConcurrentHashMap<>();

    // JSON解析器
    private final ObjectMapper objectMapper = new ObjectMapper();

    // 注入房间管理器
    @Resource
    private RoomManager roomManager;

    /**
     * 客户端连接成功
     */
    @Override
    public void afterConnectionEstablished(WebSocketSession session) throws Exception {
        ONLINE_SESSIONS.put(session.getId(), session);
        log.info("客户端连接成功，会话ID：{}", session.getId());
    }

    /**
     * 处理客户端发送的消息
     */
    @Override
    protected void handleTextMessage(WebSocketSession session, TextMessage message) throws Exception {
        // 解析客户端JSON消息
        String payload = message.getPayload();
        GomokuMessage gobangMsg = objectMapper.readValue(payload, GomokuMessage.class);
        gobangMsg.setSessionId(session.getId()); // 绑定会话ID

        // 根据消息类型分发处理
        switch (gobangMsg.getType()) {
            case "CREATE_ROOM" -> handleCreateRoom(session, gobangMsg);
            case "JOIN_ROOM" -> handleJoinRoom(session, gobangMsg);
            case "CHESS_MOVE" -> handleChessMove(session, gobangMsg);
            default -> sendErrorMsg(session, "未知消息类型：" + gobangMsg.getType());
        }
    }

    /**
     * 客户端断开连接
     */
    @Override
    public void afterConnectionClosed(WebSocketSession session, org.springframework.web.socket.CloseStatus status) throws Exception {
        ONLINE_SESSIONS.remove(session.getId());
        roomManager.removeSession(session); // 清理房间关联
        log.info("客户端断开连接，会话ID：{}", session.getId());
    }

    // ------------------------ 业务逻辑 ------------------------
    /**
     * 处理创建房间
     */
    private void handleCreateRoom(WebSocketSession session, GomokuMessage msg) throws IOException {
        String roomId = roomManager.createRoom(session);
        // 构建响应消息
        GomokuMessage resp = new GomokuMessage();
        resp.setType("ROOM_INFO");
        resp.setRoomId(roomId);
        resp.setPlayer("BLACK"); // 创建者默认执黑
        resp.setMsg("房间创建成功，ID：" + roomId + "，等待对手加入");
        // 发送给创建者
        sendMsgToSession(session, resp);
    }

    /**
     * 处理加入房间
     */
    private void handleJoinRoom(WebSocketSession session, GomokuMessage msg) throws IOException {
        String roomId = msg.getRoomId();
        // 校验房间是否存在
        if (!roomManager.existsRoom(roomId)) {
            sendErrorMsg(session, "房间不存在：" + roomId);
            return;
        }
        // 校验房间是否已满
        if (roomManager.isRoomFull(roomId)) {
            sendErrorMsg(session, "房间" + roomId + "已满，无法加入");
            return;
        }

        // 加入房间，绑定白棋
        roomManager.joinRoom(roomId, session);
        GomokuMessage resp = new GomokuMessage();
        resp.setType("ROOM_INFO");
        resp.setRoomId(roomId);
        resp.setPlayer("WHITE"); // 加入者执白
        resp.setMsg("成功加入房间：" + roomId + "，游戏开始！");
        // 发送给加入者
        sendMsgToSession(session, resp);

        // 通知房间内的黑棋玩家（对手已加入）
        WebSocketSession blackPlayer = roomManager.getOpponent(roomId, session);
        if (blackPlayer != null) {
            GomokuMessage blackMsg = new GomokuMessage();
            blackMsg.setType("ROOM_INFO");
            blackMsg.setRoomId(roomId);
            blackMsg.setMsg("对手已加入，你执黑棋先行！");
            sendMsgToSession(blackPlayer, blackMsg);
        }
    }

    /**
     * 处理落子，同步给对手
     */
    private void handleChessMove(WebSocketSession session, GomokuMessage msg) throws IOException {
        String roomId = msg.getRoomId();
        // 校验是否在房间内
        if (!roomManager.isInRoom(roomId, session)) {
            sendErrorMsg(session, "你未加入房间：" + roomId);
            return;
        }

        // 同步落子消息给对手
        WebSocketSession opponent = roomManager.getOpponent(roomId, session);
        if (opponent != null && opponent.isOpen()) {
            sendMsgToSession(opponent, msg);
        }

        // 胜负判断（简化版，可自行扩展五子连珠检测逻辑）
        boolean isWin = checkFiveInLine(msg.getX(), msg.getY(), msg.getPlayer());
        if (isWin) {
            GomokuMessage winMsg = new GomokuMessage();
            winMsg.setType("GAME_OVER");
            winMsg.setRoomId(roomId);
            winMsg.setMsg(msg.getPlayer() + "方获胜！");
            // 通知双方游戏结束
            sendMsgToSession(session, winMsg);
            sendMsgToSession(opponent, winMsg);
            // 销毁房间
            roomManager.destroyRoom(roomId);
        }
    }

    // ------------------------ 工具方法 ------------------------
    /**
     * 发送消息给指定会话
     */
    private void sendMsgToSession(WebSocketSession session, GomokuMessage msg) throws IOException {
        if (session.isOpen()) {
            String json = objectMapper.writeValueAsString(msg);
            session.sendMessage(new TextMessage(json));
        }
    }

    /**
     * 发送错误消息
     */
    private void sendErrorMsg(WebSocketSession session, String errorMsg) throws IOException {
        GomokuMessage msg = new GomokuMessage();
        msg.setType("ERROR");
        msg.setMsg(errorMsg);
        sendMsgToSession(session, msg);
    }

    /**
     * 五子连珠判断
     */
    private boolean checkFiveInLine(Integer x, Integer y, String player) {
        // 此处仅为占位，实际需实现：
        // 1. 维护每个房间的棋盘数组
        // 2. 检测横、竖、左斜、右斜四个方向是否有连续5个同色棋子
        return false;
    }
}
