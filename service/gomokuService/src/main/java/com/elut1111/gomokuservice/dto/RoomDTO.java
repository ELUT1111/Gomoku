package com.elut1111.gomokuservice.dto;

import lombok.Data;

@Data
public class RoomDTO {
    private String roomId;      // 房间ID
    private String status;     // 状态：WAIT/PLAYING/CLOSE
    private Integer playerCount; // 当前人数
    private String creator;    // 房主标识
}
