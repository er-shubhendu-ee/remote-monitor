/**
 * @file      main.js
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief     
 * @details   Distributed globally for free under the MIT License terms.
 * 
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
// config/scripts/main.js : Logic for config/config.html
document.addEventListener("DOMContentLoaded", () => {
    const networkForm = document.getElementById("network-form");
    const networkFeedback = document.getElementById("network-feedback");

    let ws;                    // WebSocket object
    let wsConnected = false;   // WebSocket connection status
    let reconnectTimer = null; // reconnect timer handle

    // ===== App state JSON =====
    const appState = {
        network: {
            ssid: "",
            password: "",
            connected: false,
        },
        page: "config",
        wsConnected: false,
        lastPing: null,
        lastPong: null,
        lastAction: null,
    };

    // ===== Logging helper =====
    function logLocal(msg) {
        console.log(`[CONFIG] ${msg}`);
    }

    // ===== WebSocket connection =====
    function connectWS() {
        ws = new WebSocket(`ws://${location.host}/ws`);

        ws.addEventListener("open", () => {
            wsConnected = true;
            appState.wsConnected = true;
            logLocal("WebSocket connected (config.html)");
            sendWSMessage({ event: "page_load", page: "config" });
            networkFeedback.textContent = "✅ Connected to device.";
            networkFeedback.className = "feedback success";
        });

        ws.addEventListener("close", () => {
            wsConnected = false;
            appState.wsConnected = false;
            logLocal("WebSocket disconnected (config.html)");
            networkFeedback.textContent = "❌ Connection lost! Trying to reconnect...";
            networkFeedback.className = "feedback error";
            attemptReconnect();
        });

        ws.addEventListener("error", (err) => {
            logLocal(`WebSocket error: ${err.message || err}`);
            networkFeedback.textContent = "⚠️ Network error.";
            networkFeedback.className = "feedback error";
        });

        ws.addEventListener("message", (ev) => {
            try {
                const data = JSON.parse(ev.data);
                if (data?.event === "ack") {
                    console.log("Server ACK:", data.message || "");
                } else if (data.type === "pong") {
                    logLocal("Pong received (connection alive)");
                    appState.lastPong = Date.now();
                }
            } catch {
                // Ignore non-JSON messages
            }
        });
    }

    // ===== Reconnect logic =====
    function attemptReconnect() {
        if (reconnectTimer) return;
        reconnectTimer = setTimeout(() => {
            reconnectTimer = null;
            if (navigator.onLine && !wsConnected) {
                logLocal("Attempting to reconnect...");
                connectWS();
            } else if (!navigator.onLine) {
                logLocal("Waiting for browser to go online...");
                attemptReconnect();
            }
        }, 5000);
    }

    // ===== Keep-alive ping =====
    setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: "ping" }));
            appState.lastPing = Date.now();
            logLocal("Ping sent (keep-alive)");
        }
    }, 60000); // every 60 seconds

    // ===== Offline / Online events =====
    window.addEventListener("offline", () => {
        wsConnected = false;
        appState.wsConnected = false;
        networkFeedback.textContent = "❌ Browser offline!";
        networkFeedback.className = "feedback error";
        logLocal("Browser offline — check Wi-Fi or cable.");
    });

    window.addEventListener("online", () => {
        networkFeedback.textContent = "⏳ Reconnecting...";
        networkFeedback.className = "feedback";
        logLocal("Browser online — attempting reconnect...");
        attemptReconnect();
    });

    // ===== Send helper =====
    function sendWSMessage(obj) {
        if (wsConnected && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify(obj));
        } else {
            console.warn("WS not connected, skipping send:", obj);
        }
    }

    // ===== Button click tracking =====
    document.addEventListener("click", (e) => {
        const target = e.target;
        if (target.tagName === "BUTTON" && target.id) {
            sendWSMessage({ event: "ui_action", action: "button", id: target.id });
            appState.lastAction = { type: "button", id: target.id, timestamp: Date.now() };
            logLocal(`Button clicked: ${target.id}`);
        }
    });

    // ===== Network form submission =====
    networkForm.addEventListener("submit", (e) => {
        e.preventDefault();
        const ssid = document.getElementById("ssid").value.trim();
        const password = document.getElementById("password").value;

        if (ssid === "" || password.length < 8) {
            networkFeedback.textContent = "❌ SSID required & password must be at least 8 chars.";
            networkFeedback.className = "feedback error";
            return;
        }

        networkFeedback.textContent = "⚡ Sending network settings...";
        networkFeedback.className = "feedback";

        appState.network.ssid = ssid;
        appState.network.password = password;
        sendWSMessage({ event: "network_config", ssid, password });
        logLocal(`Network config submitted: SSID=${ssid}, password length=${password.length}`);
    });

    // ===== Initialize =====
    connectWS();
    logLocal("Config UI initialized");

    // ===== Expose appState for debugging / ESP fetch =====
    window.appState = appState;
});
