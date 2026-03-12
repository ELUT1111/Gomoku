package com.elut1111.gomokuservice.config;

import com.elut1111.gomokuservice.handler.GomokuWebSocketHandler;
import jakarta.annotation.Resource;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

/**
 * WebSocket核心配置
 */
@Configuration
@EnableWebSocket
public class WebSocketConfig implements WebSocketConfigurer {

    @Resource
    private GomokuWebSocketHandler gobangWebSocketHandler;

    @Override
    public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
        // 注册WebSocket端点，允许跨域（适配Qt客户端）
        registry.addHandler(gobangWebSocketHandler, "/goboku/ws")
                .setAllowedOrigins("*") // 开发环境放开，生产环境指定Qt客户端IP
                .withSockJS(); // 降级兼容
    }
}