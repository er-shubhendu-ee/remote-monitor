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
// scripts/main.js : Logic for index.html
document.addEventListener('DOMContentLoaded', () => {
    const barCaption = document.getElementById('bar-caption');
    const logEl = document.getElementById('log-entries');
    const networkStatus = document.getElementById('network-status');
    const pumpStatus = document.getElementById('pump-status');
    const waterBar = document.getElementById('water-bar');
    const powerStatus = document.getElementById('value-field');
    const startBtn = document.getElementById('btn-start');
    const stopBtn = document.getElementById('btn-stop');
    const modeSel = document.getElementById('mode');
    const controlsFeedback = document.getElementById('controls-feedback');

    let ws;

    // ===== App state keys =====
    const AppKeys = {
        NETWORK_STATE: 'NETWORK_STATE',
        WS_CONNECTED: 'WS_CONNECTED',
        PUMP_STATE: 'PUMP_STATE',
        LEVEL_PERCENTAGE: 'LEVEL_PERCENTAGE',
        POWER_VALUE: 'POWER_VALUE',
        MODE: 'MODE',
        LAST_PING: 'LAST_PING'
    };

    // ===== Event keys & states =====
    const EventKeys = {
        START: 'START',
        STOP: 'STOP'
    };

    const EventStates = {
        CLICKED: 'CLICKED'
    };

    // ===== Central appState =====
    const appState = {
        [AppKeys.NETWORK_STATE]: false,
        [AppKeys.WS_CONNECTED]: false,
        [AppKeys.PUMP_STATE]: false,
        [AppKeys.LEVEL_PERCENTAGE]: 0.0,
        [AppKeys.POWER_VALUE]: 0.0,
        [AppKeys.MODE]: 'AUTO',
        [AppKeys.LAST_PING]: null
    };

    // ===== Logging helper =====
    function addLog(text) {
        const t = new Date().toLocaleTimeString();
        const line = `[${t}] ${text}`;
        if (logEl.textContent.trim() === 'No logs yet...') logEl.textContent = '';
        let logs = logEl.textContent.split('\n').filter(Boolean);
        logs.unshift(line);
        if (logs.length > 40) logs = logs.slice(0, 40);
        logEl.textContent = logs.join('\n');
        logEl.scrollTop = 0;
        console.log(line);
    }

    // ===== UI rendering =====
    function render() {
        const pct = Math.max(0, Math.min(100, Math.round(appState[AppKeys.LEVEL_PERCENTAGE])));
        waterBar.style.height = appState[AppKeys.NETWORK_STATE] ? pct + '%' : '0%';
        waterBar.textContent = appState[AppKeys.NETWORK_STATE] ? pct + '%' : '--';
        barCaption.textContent = appState[AppKeys.NETWORK_STATE] ? pct + '%' : '--';
        waterBar.style.background = appState[AppKeys.NETWORK_STATE]
            ? pct < 20 ? 'red' : pct < 40 ? 'orange' : pct < 60 ? 'yellow' : 'green'
            : '#ccc';

        pumpStatus.textContent = appState[AppKeys.PUMP_STATE] ? 'On' : 'Off';
        pumpStatus.classList.toggle('pump-on', appState[AppKeys.PUMP_STATE]);
        pumpStatus.classList.toggle('pump-off', !appState[AppKeys.PUMP_STATE]);

        networkStatus.textContent = appState[AppKeys.NETWORK_STATE] ? 'Connected' : 'Disconnected';
        networkStatus.classList.toggle('network-connected', appState[AppKeys.NETWORK_STATE]);
        networkStatus.classList.toggle('network-disconnected', !appState[AppKeys.NETWORK_STATE]);

        powerStatus.textContent = (!isNaN(appState[AppKeys.POWER_VALUE])) ? `${appState[AppKeys.POWER_VALUE].toFixed(1)} W` : '--';
    }

    function updateStatusTitle(text) {
        const statusTitle = document.querySelector('#status .card-title');
        if (statusTitle) statusTitle.textContent = text;
    }

    // ===== Update appState & send to ESP =====
    function updateAppState(key, value) {
        if (key) {
            appState[key] = value;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ TYPE: 'APP_STATE', DATA: { [key]: value } }));
                addLog(`APP_STATE sent: ${key}=${value}`);
            }
        } else {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ TYPE: 'APP_STATE', DATA: appState }));
                addLog('APP_STATE sent: full state');
            }
        }
        render();
    }

    // ===== Reset UI =====
    function resetUI() {
        updateAppState(AppKeys.PUMP_STATE, false);
        updateAppState(AppKeys.LEVEL_PERCENTAGE, 0.0);
        updateAppState(AppKeys.POWER_VALUE, 0.0);
    }

    // ===== Send generic event =====
    function sendEvent(eventId, state) {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({
                TYPE: 'EVENT',
                DATA: { [eventId]: state }
            }));
            addLog(`EVENT sent: ${eventId}=${state}`);
        }
    }

    // ===== WebSocket connection =====
    function connectWS() {
        ws = new WebSocket(`ws://${location.hostname}:${location.port || 80}/ws`);

        ws.addEventListener('open', () => {
            updateAppState(AppKeys.WS_CONNECTED, true);
            addLog('WebSocket connected');
            updateStatusTitle('Status');
            updateAppState(null); // send full state on connect/reconnect
        });

        ws.addEventListener('close', () => {
            updateAppState(AppKeys.WS_CONNECTED, false);
            updateAppState(AppKeys.NETWORK_STATE, false);
            addLog('WebSocket disconnected');
            resetUI();
            updateStatusTitle('❌ Connection lost!');
            attemptReconnect();
        });

        ws.addEventListener('error', (err) => {
            addLog(`WebSocket error: ${err.message || 'unknown error'}`);
            updateStatusTitle('⚠️ Network error');
        });

        ws.addEventListener('message', (ev) => {
            addLog(`WS RX: ${ev.data}`);
            try {
                const msg = JSON.parse(ev.data);

                if (msg.EVENT === AppKeys.NETWORK_STATE) {
                    updateAppState(AppKeys.NETWORK_STATE, !!msg.VALUE);
                    addLog(`Server network state: ${appState[AppKeys.NETWORK_STATE] ? 'Connected' : 'Disconnected'}`);
                } else if (msg.REQUEST === 'GET_APP_STATE') {
                    addLog('ESP requested full app state');
                    updateAppState(null); // send full state
                } else if (msg.TYPE === 'PONG') {
                    updateAppState(AppKeys.LAST_PING, Date.now());
                    addLog('Pong received from ESP (connection alive)');
                }

            } catch (err) {
                console.warn('Invalid JSON:', ev.data);
            }
        });
    }

    let reconnectTimer = null;
    function attemptReconnect() {
        if (reconnectTimer) return;
        reconnectTimer = setTimeout(() => {
            reconnectTimer = null;
            if (navigator.onLine && !appState[AppKeys.WS_CONNECTED]) {
                addLog('Attempting to reconnect...');
                connectWS();
            } else if (!navigator.onLine) {
                addLog('Waiting for internet connection...');
                attemptReconnect();
            }
        }, 5000);
    }

    // ===== Button events =====
    [startBtn, stopBtn].forEach((btn) => {
        btn.addEventListener('click', () => {
            if (modeSel.value.toUpperCase() === 'MANUAL') {
                const eventId = btn.id.toUpperCase() === 'BTN-START' ? EventKeys.START : EventKeys.STOP;
                sendEvent(eventId, EventStates.CLICKED);
            } else {
                controlsFeedback.textContent = '❌ Select manual from operation mode first.';
                controlsFeedback.className = 'feedback error';
            }
            updateAppState(AppKeys.MODE, modeSel.value.toUpperCase());
        });
    });

    // ===== Ping interval =====
    setInterval(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ TYPE: 'PING' }));
            updateAppState(AppKeys.LAST_PING, Date.now());
            addLog('Ping sent to keep connection alive');
        }
    }, 60000);

    modeSel.addEventListener('change', () => {
        controlsFeedback.textContent = '';
        controlsFeedback.className = 'feedback';
        addLog(`Mode changed to ${modeSel.value}`);
        updateAppState(AppKeys.MODE, modeSel.value.toUpperCase());
    });

    window.addEventListener('offline', () => {
        updateAppState(AppKeys.NETWORK_STATE, false);
        updateAppState(AppKeys.WS_CONNECTED, false);
        updateStatusTitle('❌ Network offline!');
        addLog('Browser offline — check Wi-Fi or connection.');
    });

    window.addEventListener('online', () => {
        updateStatusTitle('⏳ Reconnecting...');
        addLog('Browser online — attempting to reconnect...');
        attemptReconnect();
    });

    connectWS();
    render();
    addLog('UI loaded');
});
