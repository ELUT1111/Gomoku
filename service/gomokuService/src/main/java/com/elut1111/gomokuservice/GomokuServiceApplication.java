package com.elut1111.gomokuservice;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class GomokuServiceApplication {

    public static void main(String[] args) {
        SpringApplication.run(GomokuServiceApplication.class, args);
        System.out.println("五子棋服务端启动成功（Spring Boot 4.0.2）");
        System.out.println("WebSocket端点：ws://localhost:8080/gomoku/ws");
    }

}
