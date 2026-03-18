package com.elut1111.gomokuservice.dto;

import lombok.Data;

/**
 * 五子棋通信消息体
 */
@Data
public class GomokuMessage {
    /**
     * 消息类型：
     * CREATE_ROOM - 创建房间
     * JOIN_ROOM   - 加入房间
     * PLAYER_READY - 玩家准备
     * START_GAME  - 开始游戏
     * CHESS_MOVE  - 落子
     * ROOM_INFO   - 房间信息推送
     * GAME_OVER   - 游戏结束
     * ERROR       - 错误提示
     * 单向信息：
     * JOIN_SUCCESS -加入房间成功
     * PLACE_CHESS_STATUS -落子请求返回的落子状态
     */
    private String type;
    // 房间ID
    private String roomId;
    // 玩家（BLACK/WHITE）
    private String player;
    // 落子坐标X
    private Integer x;
    // 落子坐标Y
    private Integer y;
    // 附加消息（错误/提示/胜负结果）
    private String msg;
    // 附加bool量
    private boolean decision;
    // 玩家会话ID
    private String sessionId;
    // 当前落子玩家颜色（BLACK/WHITE）
    private String currentPlayer;
}