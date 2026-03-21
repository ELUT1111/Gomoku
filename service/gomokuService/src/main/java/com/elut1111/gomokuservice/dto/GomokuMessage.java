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
     *  RANDOM_MATCH -随机匹配
     *  CANCEL_MATCH -取消匹配
     * QUIT_ROOM   - 玩家退出
     * PLAYER_READY - 玩家准备
     * START_GAME  - 开始游戏
     * CHESS_MOVE  - 落子
     * ROOM_INFO   - 房间信息推送
     * GAME_OVER   - 游戏结束
     * ERROR       - 错误提示
     * REFRESH_ROOM_LIST  - 客户端请求刷新房间列表
     * 单向信息：
     * JOIN_SUCCESS - 加入房间成功
     * ROOM_LIST    - 服务端推送房间列表
     * QUIT_ROOM_SUCCESS - 退出房间成功
     * PLACE_CHESS_STATUS - 落子请求返回的落子状态
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