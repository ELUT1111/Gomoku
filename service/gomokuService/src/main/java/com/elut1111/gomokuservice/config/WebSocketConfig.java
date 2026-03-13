package com.elut1111.gomokuservice.config;

import com.elut1111.gomokuservice.handler.GomokuWebSocketHandler;
import jakarta.annotation.Resource;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;
import org.springframework.web.socket.server.standard.ServletServerContainerFactoryBean;
import org.springframework.context.annotation.Bean;

/**
 * WebSocket核心配置（少量优化，保留原有核心逻辑）
 */
@Configuration
@EnableWebSocket
public class WebSocketConfig implements WebSocketConfigurer {
    @Resource
    private GomokuWebSocketHandler gomokuWebSocketHandler; // 修正原有变量名（gobang→gomoku，与项目名一致）

    @Override
    public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
        // 注册WebSocket端点，兼容Qt客户端（保留原有）
        registry.addHandler(gomokuWebSocketHandler, "/gomoku/ws")
                .setAllowedOriginPatterns("*"); // 替代setAllowedOrigins，Spring Boot 2.4+推荐，更严谨
    }

    /**
     * WebSocket超时配置，避免客户端长时间无操作断开
     */
    @Bean
    public ServletServerContainerFactoryBean createWebSocketContainer() {
        ServletServerContainerFactoryBean container = new ServletServerContainerFactoryBean();
        // 会话超时时间：30分钟
        container.setMaxSessionIdleTimeout(1800000L);
        // 最大消息大小：1024*1024字节
        container.setMaxTextMessageBufferSize(1024 * 1024);
        return container;
    }
}