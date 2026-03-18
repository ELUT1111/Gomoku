package com.elut1111.gomokuservice.handler;

import com.elut1111.gomokuservice.dto.GomokuMessage;
import com.elut1111.gomokuservice.entity.ChessBoard;
import com.elut1111.gomokuservice.entity.Player;
import com.elut1111.gomokuservice.entity.Room;
import com.elut1111.gomokuservice.manager.RoomManager;
import com.elut1111.gomokuservice.util.FiveInLineCheckUtil;
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
        log.info("客户端连接成功，会话ID：{}，当前在线数：{}", session.getId(), ONLINE_SESSIONS.size());
    }

    /**
     * 处理客户端发送的消息
     */
    @Override
    protected void handleTextMessage(WebSocketSession session, TextMessage message) throws Exception {
        try {
            // 解析客户端JSON消息
            String payload = message.getPayload();
            GomokuMessage gomokuMsg = objectMapper.readValue(payload, GomokuMessage.class);
            gomokuMsg.setSessionId(session.getId()); // 绑定会话ID
            log.info("接收客户端消息：{}，会话ID：{}", payload, session.getId());

            // 根据消息类型分发处理
            switch (gomokuMsg.getType()) {
                case "CREATE_ROOM" -> handleCreateRoom(session, gomokuMsg);
                case "JOIN_ROOM" -> handleJoinRoom(session, gomokuMsg);
                case "CHESS_MOVE" -> handleChessMove(session, gomokuMsg);
                case "PLAYER_READY" -> handlePlayerReady(session, gomokuMsg);
                case "START_GAME" -> handleStartGame(session, gomokuMsg);
                default -> {
                    log.warn("未知消息类型：{}，会话ID：{}", gomokuMsg.getType(), session.getId());
                    sendErrorMsg(session, "未知消息类型：" + gomokuMsg.getType());
                }
            }
        } catch (Exception e) {
            log.error("处理客户端消息异常，会话ID：{}，异常信息：{}", session.getId(), e.getMessage(), e);
            sendErrorMsg(session, "消息解析失败，请检查格式");
        }
    }

    /**
     * 客户端断开连接
     */
    @Override
    public void afterConnectionClosed(WebSocketSession session, org.springframework.web.socket.CloseStatus status) throws Exception {
        String sessionId = session.getId();
        ONLINE_SESSIONS.remove(sessionId);
        roomManager.removeSession(session); // 清理房间关联
        log.info("客户端断开连接，会话ID：{}，当前在线数：{}", sessionId, ONLINE_SESSIONS.size());

        // 通知对手：玩家掉线
        String roomId = roomManager.getRoomIdBySessionId(sessionId);
        if (roomId != null) {
            WebSocketSession opponent = roomManager.getOpponent(roomId, session);
            if (opponent != null && opponent.isOpen()) {
                GomokuMessage msg = new GomokuMessage();
                msg.setType("GAME_OVER");
                msg.setMsg("对手已掉线，游戏结束！");
                sendMsgToSession(opponent, msg);
            }
        }
    }

    /**
     * 处理创建房间
     */
    private void handleCreateRoom(WebSocketSession session, GomokuMessage msg) {
        try {
            // 避免重复创建房间
            if (roomManager.getRoomIdBySessionId(session.getId()) != null) {
                sendErrorMsg(session, "你已创建/加入房间，不可重复创建");
                return;
            }
            String roomId = roomManager.createRoom(session);
            // 构建响应消息
            GomokuMessage resp = new GomokuMessage();
            resp.setType("ROOM_INFO");
            resp.setRoomId(roomId);
            resp.setPlayer("BLACK"); // 创建者默认执黑
            resp.setMsg("房间创建成功，ID：" + roomId + "，等待对手加入");
            // 发送给创建者
            sendMsgToSession(session, resp);
            log.info("[服务端] 房间创建成功，ID：{}", roomId);
        } catch (Exception e) {
            log.error("创建房间异常，会话ID：{}", session.getId(), e);
            sendErrorMsg(session, "房间创建失败，请重试");
        }
    }

    /**
     * 处理加入房间
     */
    private void handleJoinRoom(WebSocketSession session, GomokuMessage msg) {
        try {
            String roomId = msg.getRoomId();
            // 校验房间ID非空
            if (roomId == null || roomId.trim().isEmpty()) {
                sendErrorMsg(session, "房间ID不能为空");
                return;
            }
            // 避免重复加入房间
            if (roomManager.getRoomIdBySessionId(session.getId()) != null) {
                sendErrorMsg(session, "你已创建/加入房间，不可重复加入");
                return;
            }
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
            // 加入房间
            roomManager.joinRoom(roomId, session);
            GomokuMessage resp = new GomokuMessage();
            resp.setType("JOIN_SUCCESS");
            resp.setRoomId(roomId);
            resp.setPlayer("WHITE"); // 加入者执白
            resp.setMsg("成功加入房间：" + roomId);
            // 发送给加入者
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
        } catch (Exception e) {
            log.error("加入房间异常，会话ID：{}，房间ID：{}", session.getId(), msg.getRoomId(), e);
            sendErrorMsg(session, "加入房间失败，请重试");
        }
    }

    /**
     * 处理落子，同步给对手（实现五子连珠判断、落子校验）
     */
    private void handleChessMove(WebSocketSession session, GomokuMessage msg) {
        try {
            String roomId = msg.getRoomId();
            Integer x = msg.getX();
            Integer y = msg.getY();
            String playerColor = msg.getPlayer();

            // 1. 基础参数校验
            if (roomId == null || x == null || y == null || playerColor == null) {
                sendErrorMsg(session, "落子参数不完整");
                return;
            }
            // 2. 校验是否在房间内
            if (!roomManager.isInRoom(roomId, session)) {
                sendErrorMsg(session, "你未加入房间：" + roomId);
                return;
            }
            // 3. 获取房间实体，校验房间状态
            Room room = roomManager.getRoomByRoomId(roomId);
            if (room == null || room.getStatus() != Room.RoomStatus.PLAYING) {
                sendErrorMsg(session, "房间未开始游戏或已结束");
                return;
            }
            // 4. 校验是否为当前落子玩家
            if (!playerColor.equals(room.getCurrentPlayer())) {
                sendErrorMsg(session, "请等待对方落子");
                return;
            }
            // 5. 校验棋盘位置是否合法
            ChessBoard chessBoard = room.getChessBoard();
            if (!chessBoard.checkChessPos(x, y)) {
                sendErrorMsg(session, "落子位置非法");
                return;
            }


            // 6. 执行落子，更新棋盘状态
            boolean placeSuccess = chessBoard.placeChess(x, y, playerColor);
            if (!placeSuccess) {
                sendErrorMsg(session, "落子失败，请重试");
                return;
            }

            // 7. 同步落子消息给对手
            WebSocketSession opponent = roomManager.getOpponent(roomId, session);
            if (opponent != null && opponent.isOpen()) {
                sendMsgToSession(opponent, msg);
            }

            // 发送落子成功信息
            GomokuMessage resp = new GomokuMessage();
            resp.setType("PLACE_CHESS_STATUS");
            resp.setX(x);
            resp.setY(y);
            resp.setRoomId(roomId);
            resp.setPlayer(playerColor);
            resp.setDecision(true);
            sendMsgToSession(session, resp);
            if (opponent != null) {
                sendMsgToSession(opponent,resp);
            }

            // 8. 五子连珠胜负判断
            boolean isWin = FiveInLineCheckUtil.checkFiveInLine(x, y, playerColor, chessBoard);
            if (isWin) {
                GomokuMessage winMsg = new GomokuMessage();
                winMsg.setType("GAME_OVER");
                winMsg.setRoomId(roomId);
                winMsg.setMsg(playerColor + "方获胜！五子连珠！");
                // 通知双方游戏结束
                sendMsgToSession(session, winMsg);
                if (opponent != null && opponent.isOpen()) {
                    sendMsgToSession(opponent, winMsg);
                }
                // 销毁房间
                roomManager.destroyRoom(roomId);
                return;
            }

            // 9. 校验是否平局（棋盘下满）
            if (chessBoard.getChessCount() >= ChessBoard.BOARD_SIZE * ChessBoard.BOARD_SIZE) {
                GomokuMessage drawMsg = new GomokuMessage();
                drawMsg.setType("GAME_OVER");
                drawMsg.setRoomId(roomId);
                drawMsg.setMsg("棋盘下满，双方平局！");
                sendMsgToSession(session, drawMsg);
                if (opponent != null && opponent.isOpen()) {
                    sendMsgToSession(opponent, drawMsg);
                }
                roomManager.destroyRoom(roomId);
                return;
            }

            // 10. 切换当前落子玩家
            room.switchCurrentPlayer();

        } catch (Exception e) {
            log.error("落子处理异常，会话ID：{}，房间ID：{}", session.getId(), msg.getRoomId(), e);
            sendErrorMsg(session, "落子失败，服务端异常");
        }
    }

    /**
     * 发送消息给指定会话（保留方法名，增加IO异常捕获）
     */
    private void sendMsgToSession(WebSocketSession session, GomokuMessage msg) {
        try {
            if (session.isOpen()) {
                String json = objectMapper.writeValueAsString(msg);
                session.sendMessage(new TextMessage(json));
                log.info("发送消息给客户端：{}，会话ID：{}", json, session.getId());
            }
        } catch (IOException e) {
            log.error("发送消息异常，会话ID：{}", session.getId(), e);
        }
    }

    /**
     * 发送错误消息
     */
    private void sendErrorMsg(WebSocketSession session, String errorMsg) {
        GomokuMessage msg = new GomokuMessage();
        msg.setType("ERROR");
        msg.setMsg(errorMsg);
        sendMsgToSession(session, msg);
    }

    /**
     * 处理玩家点击准备
     */
    private void handlePlayerReady(WebSocketSession session, GomokuMessage msg) {
        String roomId = roomManager.getRoomIdBySessionId(session.getId());
        if (roomId == null) return;
        Room room = roomManager.getRoomByRoomId(roomId);
        Player player = roomManager.getPlayerBySession(room, session);
        if (player != null) {
            boolean decision = msg.isDecision();
            player.setReady(decision);

            // 通知双方准备状态
            WebSocketSession opponent = roomManager.getOpponent(roomId, session);
            if (opponent != null && opponent.isOpen()) {
                GomokuMessage notify = new GomokuMessage();
                notify.setType("PLAYER_READY");
                notify.setRoomId(roomId);
                notify.setPlayer("OPPONENT_READY");
                notify.setDecision(decision);
                notify.setMsg(decision? "对手已准备，可以开始游戏！" : "等待对手准备游戏");
                sendMsgToSession(opponent, notify);

                GomokuMessage notify2 = new GomokuMessage();
                notify2.setType("PLAYER_READY");
                notify2.setRoomId(roomId);
                notify2.setPlayer("OPPONENT_READY");
                notify2.setMsg(decision? "已准备游戏":"尚未准备游戏");
                notify2.setDecision(decision);
                sendMsgToSession(session, notify2);
            }

            // 通知自己
//            GomokuMessage selfMsg = new GomokuMessage();
//            selfMsg.setType("ROOM_INFO");
//            selfMsg.setRoomId(roomId);
//            selfMsg.setPlayer("SELF_READY");
//            selfMsg.setMsg("已准备，等待房主开始游戏");
//            sendMsgToSession(session, selfMsg);
        }
    }

    /**
     * 处理房主开始游戏
     */
    private void handleStartGame(WebSocketSession session, GomokuMessage msg) {
        String roomId = roomManager.getRoomIdBySessionId(session.getId());
        if (roomId == null) return;
        Room room = roomManager.getRoomByRoomId(roomId);
        Player player = roomManager.getPlayerBySession(room, session);

        if (player == null || !"BLACK".equals(player.getColor())) {
            sendErrorMsg(session, "只有房主可以开始游戏");
            return;
        }
        if (!room.isAllReady()) {
            sendErrorMsg(session, "有玩家未准备，无法开始");
            return;
        }

        room.setStatus(Room.RoomStatus.PLAYING);
        GomokuMessage startMsg = new GomokuMessage();
        startMsg.setType("GAME_START");
        startMsg.setRoomId(roomId);
        startMsg.setMsg("游戏开始！黑棋先行");

        sendMsgToSession(session, startMsg);
        WebSocketSession opponent = roomManager.getOpponent(roomId, session);
        if (opponent != null && opponent.isOpen()) {
            sendMsgToSession(opponent, startMsg);
        }
    }
}